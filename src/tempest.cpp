#include "tempest.hpp"
#include <spdlog/spdlog.h>
#include <thread>

namespace tempest
{
Tempest::Tempest(bool verbose)
  : queue_(std::make_shared<UDPReceiver::QueueType>())
  , receiver_(io_context_, 50222, queue_)
{
    spdlog::set_level(verbose ? spdlog::level::debug : spdlog::level::info);
}

void Tempest::run()
{
    spdlog::info("Starting threads...");
    io_thread_ = std::make_unique<std::thread>([this]() { io_context_.run(); });
    pc_thread_ = std::make_unique<std::thread>([this]() { process(); });
}

void Tempest::join()
{
    io_thread_->join();
    pc_thread_->join();
}

void Tempest::add_handler(HandlerType func)
{
    handlers_.push_back(func);
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

        spdlog::debug("Got packet...");

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
            spdlog::debug(data.dump());
        }
        else
        {
            spdlog::debug("Got packet type \"{}\", ignoring...", packet_type);
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
    j.at("firmware_revision").get_to(o.firmware_revision);
    j.at("serial_number").get_to(o.station_sn);
    j.at("hub_sn").get_to(o.hub_sn);

    // denest observation array
    json obs = j.at("obs").at(0);

    if (!obs.is_array() || obs.size() != 18)
    {
        // since the UDP message just uses an array of values and not keys... it isnt backwards
        // compatible
        throw std::runtime_error(
          std::format("Observation array in JSON message must have 18 fields, got {}", obs.size()));
    }

    obs[0].get_to(o.epoch_time);
    obs[1].get_to(o.wind.lull_mph);
    o.wind.lull_mph *= MS_TO_MPH;
    obs[2].get_to(o.wind.avg_mph);
    o.wind.avg_mph *= MS_TO_MPH;
    obs[3].get_to(o.wind.gust_mph);
    o.wind.gust_mph *= MS_TO_MPH;
    obs[4].get_to(o.wind.dir_deg);
    obs[5].get_to(o.wind.sample_interval_secs);
    obs[6].get_to(o.pressure_inhg);
    o.pressure_inhg *= MB_TO_INHG;
    obs[7].get_to(o.air_temp_f);
    o.air_temp_f = C_TO_F(o.air_temp_f);
    obs[8].get_to(o.relative_humidity);
    obs[9].get_to(o.illuminance_lux);
    obs[10].get_to(o.uv_index);
    obs[11].get_to(o.solar_radiation_wm2);
    obs[12].get_to(o.rain_accum_in);
    o.rain_accum_in *= MM_TO_IN;
    obs[13].get_to(o.precipitation_type);
    obs[14].get_to(o.lightning_strike_dist_mi);
    o.lightning_strike_dist_mi *= KM_TO_MI;
    obs[15].get_to(o.lightning_strike_count);
    obs[16].get_to(o.battery_volts);
    obs[17].get_to(o.report_interval);
}
}