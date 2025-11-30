#include "tempest.hpp"
#include "ws_receiver.hpp"
#include <optional>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <thread>
#include <websocketpp/frame.hpp>

namespace tempest
{
Tempest::Tempest(const std::string &token, const std::string &device_id, bool verbose)
  : queue_(std::make_shared<UDPReceiver::QueueType>())
  , udp_receiver_(50222, queue_)
{
    spdlog::set_level(verbose ? spdlog::level::debug : spdlog::level::info);
    if (token.size() > 0 && device_id.size() > 0)
    {
        ws_receiver_ = std::make_unique<WebsocketReceiver>(token, device_id, queue_);
    }
}

void Tempest::run()
{
    spdlog::debug("Starting threads.");
    udp_thread_ = std::make_unique<std::thread>([this]() { udp_receiver_.run(); });
    if (ws_receiver_)
    {
        ws_thread_ = std::make_unique<std::thread>([this]() { ws_receiver_->run(); });
    }
    process_thread_ = std::make_unique<std::thread>([this]() { process(); });
}

void Tempest::join()
{
    udp_thread_->join();
    process_thread_->join();
}

void Tempest::add_handler(HandlerType func)
{
    handlers_.push_back(std::move(func));
}

void Tempest::process()
{
    std::string packet;
    while (true)
    {
        try
        {
            queue_->pop(packet);
        }
        catch (std::exception& e)
        {
            spdlog::error("Failed to pop off queue: {}", e.what());
            continue;
        }

        json data;
        try
        {
            data = json::parse(packet);
        }
        catch (std::runtime_error& e)
        {
            spdlog::error("Failed to parse JSON: {}", e.what());
            continue;
        }

        auto packet_type = data["type"].get<std::string>();
        if (packet_type == "obs_st")
        {
            const auto obs = data.get<Observation>();
            for (const auto& handler : handlers_)
            {
                handler(obs);
            }
        }
        else
        {
            spdlog::debug("Got packet type \"{}\", ignoring.", packet_type);
        }
    }
}

void from_json(const json& j, Observation& o)
{
    static constexpr auto MS_TO_MPH  = 2.23694;
    static constexpr auto C_TO_F     = [](const auto& c) { return ((c * 9.0 / 5.0) + 32); };
    static constexpr auto MM_TO_IN   = 0.0393701;
    static constexpr auto KM_TO_MI   = 0.621371;
    static constexpr auto MB_TO_INHG = 0.02953;


    // local UDP provides firmware_revision, serial_number, and hub_sn
    try
    {
        json_get(j.at("firmware_version"), o.firmware_revision);
    }
    catch (json::out_of_range&)
    {
        o.firmware_revision = std::nullopt;
    }

    try
    {
        json_get(j.at("serial_number"), o.station_sn);
    }
    catch (json::out_of_range&)
    {
        o.station_sn = std::nullopt;
    }

    try
    {
        json_get(j.at("hub_sb"), o.hub_sn);
    }
    catch(json::out_of_range&)
    {
        o.hub_sn = std::nullopt;
    }

    // denest observation array
    json obs = j.at("obs").at(0);

    if (!obs.is_array())
    {
        // since the UDP message just uses an array of values and not keys... it isnt backwards
        // compatible
        throw std::runtime_error(
          std::format("Observation array in JSON message must have 18 fields, got {}", obs.size()));
    }
    
    json_get(obs[0], o.epoch_time);
    o.wind = WindObservation{};
    json_get(obs[1], o.wind.value().lull_mph);
    json_get(obs[2], o.wind.value().avg_mph);
    json_get(obs[3], o.wind.value().gust_mph);
    json_get(obs[4], o.wind.value().dir_deg);
    json_get(obs[5], o.wind.value().sample_interval_secs);
    json_get(obs[6], o.pressure_inhg);
    json_get(obs[7], o.air_temp_f);
    json_get(obs[8], o.relative_humidity);
    json_get(obs[9], o.illuminance_lux);
    json_get(obs[10], o.uv_index);
    json_get(obs[11], o.solar_radiation_wm2);
    json_get(obs[12], o.rain_accum_in);
    json_get(obs[13], o.precipitation_type);
    json_get(obs[14], o.lightning_strike_dist_mi);
    json_get(obs[15], o.lightning_strike_count);
    json_get(obs[16], o.battery_volts);
    json_get(obs[17], o.report_interval);

    o.wind.value().lull_mph *= MS_TO_MPH;
    o.wind.value().avg_mph *= MS_TO_MPH;
    o.wind.value().gust_mph *= MS_TO_MPH;
    o.pressure_inhg.value() *= MB_TO_INHG;
    o.air_temp_f.value() = C_TO_F(o.air_temp_f.value());
    o.rain_accum_in.value() *= MM_TO_IN;
    o.lightning_strike_dist_mi.value() *= KM_TO_MI;

    // websocket API provides 4 additional fields at the end of the array, everything else is the same
    try
    {
        json_get(obs.at(18), o.local_daily_rain_accum_in);
        o.local_daily_rain_accum_in.value() *= MM_TO_IN;
    }
    catch (json::out_of_range&)
    {
        o.local_daily_rain_accum_in = std::nullopt;
    }

    try
    {
        json_get(obs.at(19), o.rain_accum_final_in);
        o.rain_accum_final_in.value() *= MM_TO_IN;
    }
    catch (json::out_of_range&)
    {
        o.rain_accum_final_in = std::nullopt;
    }

    try
    {
        json_get(obs.at(20), o.local_daily_rain_accum_final_in);
        o.rain_accum_final_in.value() *= MM_TO_IN;
    }
    catch (json::out_of_range&)
    {
        o.local_daily_rain_accum_final_in = std::nullopt;
    }

    try
    {
        json_get(obs.at(21), o.precipitation_analysis_type);
    }
    catch (json::out_of_range&)
    {
        o.precipitation_analysis_type = std::nullopt;
    }
}
}