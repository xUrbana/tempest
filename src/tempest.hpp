#pragma once
#include "udp_receiver.hpp"
#include "ws_receiver.hpp"
#include <boost/asio.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>
#include <optional>
#include <format>

namespace tempest
{
//using ws_ctx_ptr = websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context>;
using json = nlohmann::json;

enum class PrecipitationType
{
    NONE          = 0,
    RAIN          = 1,
    HAIL          = 2,
    RAIN_AND_HAIL = 3
};

enum class PrecipitationAnalysisType
{
  NONE = 0,
  RAIN_CHECK_DISPLAY_ON = 1,
  RAIN_CHECK_DISPLAY_OFF = 2
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
    std::optional<uint16_t>          firmware_revision;
    std::optional<std::string>       station_sn;
    std::optional<std::string>       hub_sn;
    std::optional<uint64_t>          epoch_time;
    std::optional<WindObservation>   wind;
    std::optional<double>            pressure_inhg;
    std::optional<double>            air_temp_f;
    std::optional<double>            relative_humidity;
    std::optional<uint64_t>          illuminance_lux;
    std::optional<uint16_t>          uv_index;
    std::optional<uint64_t>          solar_radiation_wm2;
    std::optional<double>            rain_accum_in;
    std::optional<PrecipitationType> precipitation_type;
    std::optional<double>            lightning_strike_dist_mi;
    std::optional<uint16_t>          lightning_strike_count;
    std::optional<double>            battery_volts;
    std::optional<uint16_t>          report_interval;
    std::optional<double>            local_daily_rain_accum_in;
    std::optional<double>            rain_accum_final_in;
    std::optional<double>            local_daily_rain_accum_final_in;
    std::optional<PrecipitationAnalysisType> precipitation_analysis_type;
};

class Tempest
{
  public:
    using HandlerType = std::function<void(const Observation&)>;
    Tempest(const std::string &token = "", const std::string &device_id = "", bool verbose = false);
    void run();
    void join();
    void add_handler(HandlerType func);

  private:
    // Disable copy and move since this class contains sockets
    Tempest(const Tempest&)            = delete;
    Tempest(Tempest&&)                 = delete;
    Tempest& operator=(const Tempest&) = delete;
    Tempest& operator=(Tempest&&)      = delete;

    void process();

    std::shared_ptr<UDPReceiver::QueueType> queue_;
    UDPReceiver                             udp_receiver_;
    std::unique_ptr<WebsocketReceiver>      ws_receiver_;
    std::vector<HandlerType>                handlers_;
    std::unique_ptr<std::thread>            udp_thread_;
    std::unique_ptr<std::thread>            process_thread_;
    std::unique_ptr<std::thread>            ws_thread_;
};

template <typename T>
void json_get(const json &j, std::optional<T> &o)
{
  o = j.get<T>();
}

template <typename T>
void json_get(const json &j, T &o)
{
  o = j.get<T>();
}

void from_json(const json& j, Observation& o);
}

template<>
struct std::formatter<tempest::Observation>
{
  constexpr auto parse(auto &ctx) const 
  { 
    return ctx.begin(); 
  }

  constexpr auto format(const tempest::Observation& o, auto& ctx) const
  {
      return std::format_to(ctx.out(), "Observation(temp={}, lightning_count={})", o.air_temp_f.value_or(0), o.lightning_strike_count.value_or(0));
  }
};