#pragma once
#include "thread_safe_queue.hpp"
#include "udp_receiver.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Tempest
{
  public:
    Tempest();
    void run();

  private:
    // Disable copy and move since this class contains sockets
    Tempest(const Tempest &)            = delete;
    Tempest(Tempest &&)                 = delete;
    Tempest &operator=(const Tempest &) = delete;
    Tempest &operator=(Tempest &&)      = delete;

    void process();

    boost::asio::io_context              io_context_;
    std::shared_ptr<Receiver::QueueType> queue_;
    Receiver                             receiver_;
};

enum class PrecipitationType : uint8_t
{
    NONE          = 0,
    RAIN          = 1,
    HAIL          = 2,
    RAIN_AND_HAIL = 3
};

struct WindObservation
{
    double   lull_mph;
    double   avg_mph;
    double   gust_mph;
    uint16_t dir_deg;
    uint16_t sample_interval_secs;
};

struct Observation
{
    uint16_t          firmware_revision;
    std::string       station_sn;
    std::string       hub_sn;
    uint64_t          epoch_time;
    WindObservation   wind;
    double            pressure_mb;
    double            air_temp_f;
    double            relative_humidity;
    uint64_t          illuminance_lux;
    uint8_t           uv_index;
    uint64_t          solar_radiation_wm2;
    double            rain_accum_mm;
    PrecipitationType precipitation_type;
    double            lightning_strike_dist_km;
    uint16_t          lightning_strike_count;
    double            battery_volts;
    uint16_t          report_interval;
};

void from_json(const json &j, Observation &o);