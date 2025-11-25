#!/usr/bin/env python3.15t

from pytempest import *
from lifxlan import LifxLAN
import time
from functools import partial

def rgb_to_hsbk(r: int, g: int, b: int) -> tuple[float, float, float, int]:
    """
    Converts an RGB color tuple (0-255 range) to an HSBK color tuple.
    (HSB is often referred to as HSV or Hue, Saturation, Value).
    
    Note: Kelvin (K) is a color temperature value and cannot be perfectly
    inferred from a single RGB triplet, especially for saturated colors.
    It is set to a neutral default (4000K) for practical use.

    Args:
        r: Red component (0-255).
        g: Green component (0-255).
        b: Blue component (0-255).

    Returns:
        A tuple (h, s, b, k) where:
        h: Hue in degrees (0.0 to 360.0).
        s: Saturation as a percentage (0.0 to 100.0).
        b: Brightness (Value) as a percentage (0.0 to 100.0).
        k: Kelvin (Color Temperature) in an integer range (e.g., 2500 to 9000).
    """

    # 1. Normalize R, G, B components to the range [0, 1]
    r_norm = r / 255.0
    g_norm = g / 255.0
    b_norm = b / 255.0

    # Find the maximum and minimum values
    max_val = max(r_norm, g_norm, b_norm)
    min_val = min(r_norm, g_norm, b_norm)
    delta = max_val - min_val

    h = 0.0
    s = 0.0
    # Brightness (B) is equal to the maximum normalized RGB value
    b_val = max_val 
    
    # Kelvin default: Set to a neutral white point (4000K)
    kelvin = 4000

    # If max_val and min_val are the same, the color is grayscale (no hue, saturation is 0)
    if delta == 0:
        h = 0.0
        s = 0.0
    else:
        # 2. Calculate Saturation (S)
        # s = delta / max_val
        s = delta / max_val

        # 3. Calculate Hue (H)
        if max_val == r_norm:
            # Hue is between yellow and magenta
            h = (g_norm - b_norm) / delta
        elif max_val == g_norm:
            # Hue is between cyan and yellow, add 2
            h = (b_norm - r_norm) / delta + 2.0
        else: # max_val == b_norm
            # Hue is between magenta and cyan, add 4
            h = (r_norm - g_norm) / delta + 4.0

        # Convert hue to degrees (0-360) and make sure it's positive
        h = h * 60.0
        if h < 0:
            h += 360.0

    # 4. Convert Saturation and Brightness to percentage (0-100)
    s_percent = int(round(s * 65535))
    b_percent = .1 * 65535 # 10 percent brightness 
    h_percent = int(round(0x10000 * h) / 360) % 0x10000
    
    # 5. Kelvin is already set

    return (h_percent, s_percent, b_percent, kelvin)

def map_value(x, in_range, out_range):
    in_min, in_max = in_range
    out_min, out_max = out_range
    scaled_value = (x - in_min) / (in_max - in_min)
    return out_min + (scaled_value * (out_max - out_min))

def clamp100(val: float) -> float:
    """Helper to keep values between 0 and 100."""
    return max(0.0, min(100.0, val))

def get_observation_severity(obs) -> float:
    """
    Calculates a severity score from 0-100 based on weighted sub-components.
    The score is driven by the most dangerous element, plus a 'chaos factor'
    if multiple hazards are occurring simultaneously.
    """
    wind_score = get_wind_score(obs.wind)
    rain_score = get_rain_score(obs)
    lightning_score = get_lightning_score(obs)
    env_score = get_environmental_score(obs)

    # List all component scores
    score_report = {
        'wind': wind_score,
        'rain': rain_score,
        'lightning': lightning_score,
        'environment': env_score
    }

    scores = sorted(score_report.values(), reverse=True)
    primary_threat = scores[0]
    
    # Calculate "Chaos Factor":
    # If the primary threat is high AND secondary threats exist, boost the score.
    # We take 25% of the average of the remaining (non-primary) hazards.
    if primary_threat > 0:
        remaining_sum = sum(scores[1:])
        remaining_count = len(scores) - 1
        if remaining_count > 0:
            secondary_hazards_avg = remaining_sum / remaining_count
            primary_threat += (secondary_hazards_avg * 0.25)

    return clamp100(primary_threat)

def get_wind_score(wind) -> float:
    """
    Wind score based on Gust.
    Threshold: 75 mph gust = 100 score (Cat 1 Hurricane floor)
    """
    if wind.gust_mph <= 0:
        return 0.0
    
    # Linear mapping: 0 mph -> 0, 75 mph -> 100
    # 40 mph (Gale) returns ~53
    return clamp100((wind.gust_mph / 75.0) * 100.0)

def get_rain_score(obs) -> float:
    """
    Rain score based on Rate (Inches per Hour).
    Threshold: 1.0 in/hr = 100 score (Violent Rain)
    """
    if obs.report_interval == 0:
        return 0.0

    rain_accum_per_min = obs.rain_accum_in / obs.report_interval

    score = clamp100((rain_accum_per_min / 1.0) * 25.0)

    # Immediate boost for Hail
    if obs.precipitation_type == PrecipitationType.HAIL:
        # If hail is detected, severity is at least 75 (Dangerous)
        score = max(score, 75.0)

    return score

def get_lightning_score(obs: Observation) -> float:
    """
    Lightning score based on Frequency and Proximity.
    Threshold: 5 strikes/min = 100 score
    """
    if obs.lightning_strike_count == 0:
        return 0.0
    
    strikes_per_min = obs.lightning_strike_count / obs.report_interval
    
    # Base score on frequency
    frequency_score = clamp100((strikes_per_min / 5.0) * 100.0)

    # Multiplier for proximity (Danger Close)
    proximity_multiplier = 1.0
    
    # Distance logic:
    # < 3 miles: Very dangerous (1.5x)
    # < 6 miles: Dangerous (1.2x)
    if obs.lightning_strike_dist_mi < 3.0:
        proximity_multiplier = 1.5
    elif obs.lightning_strike_dist_mi < 6.0:
        proximity_multiplier = 1.2
        
    return clamp100(frequency_score * proximity_multiplier)

def get_environmental_score(obs: Observation) -> float:
    """
    Score based on extreme Heat/Cold or UV.
    """
    # UV Index: 11+ is extreme
    uv_score = (obs.uv_index / 11.0) * 100.0

    # Temperature extremes (Simplified)
    temp_score = 0.0
    if obs.air_temp_f > 100.0:
        # Heat: 100F -> 50 score, 110F -> 100 score
        temp_score = (obs.air_temp_f - 100.0) * 5.0 + 50.0
    elif obs.air_temp_f < 0.0:
        # Cold: 0F -> 40 score, -20F -> 80 score
        temp_score = abs(obs.air_temp_f) * 2.0 + 40.0

    # Return whichever environmental factor is worse
    return max(clamp100(uv_score), clamp100(temp_score))

def get_rgb(score):
    offset = map_value(score, (0, 100), (0, 255))
    return (int(offset), int(255 - offset), 0)

def process_observation(bulb, obs):
    score = get_observation_severity(obs)
    print(f"Pressure: {obs.pressure_inhg}")
    r, g, b = get_rgb(score)
    print(f"\033[38;2;{r};{g};{b}mScore: {score}\033[0m")
    hspk = rgb_to_hsbk(r, g, b)
    print(f"Setting bulb HSPK... {hspk}")
    bulb.set_color(hspk)


if __name__ == "__main__":

    lifx = LifxLAN(num_lights=1)

    devices = lifx.get_lights()
    bulb = devices[0]
    print(f"Found bulb {bulb.get_label()}")

    t = Tempest(True)
    t.add_handler(partial(process_observation, bulb))
    t.run()

    while True:
        try:
            time.sleep(1)
        except KeyboardInterrupt:
            break

