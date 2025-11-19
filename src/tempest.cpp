#include "tempest.hpp"
#include <iostream>
#include <print>
#include <thread>

Tempest::Tempest() : queue_(std::make_shared<Receiver::QueueType>()), receiver_(io_context_, 50222, queue_)
{
}

void Tempest::run()
{
    std::thread io_thread([this]() { io_context_.run(); });
    std::thread pc_thread([this]() { process(); });
    io_thread.join();
    pc_thread.join();
}

void Tempest::process()
{
    Packet packet;
    while (true)
    {
        try
        {
            queue_->pop(packet);
        }
        catch (std::exception &e)
        {
            std::print(std::cerr, "{}\n", e.what());
        }

        json data = json::parse(packet.data);

        if (data.at("type") == "obs_st")
        {
            auto obs = data.get<Observation>();
            std::println("observation temp={}", obs.air_temp_f);
        }
    }
}

void from_json(const json &j, Observation &o)
{
    static constexpr double MS_TO_MPH = 2.23694;
    static constexpr auto   C_TO_F    = [](const auto &c) { return ((c * 9.0 / 5.0) + 32); };
    j.at("firmware_revision").get_to(o.firmware_revision);
    j.at("serial_number").get_to(o.station_sn);
    j.at("hub_sn").get_to(o.hub_sn);

    // denest observation array
    json obs = j.at("obs").at(0);

    assert(obs.size() == 18);

    obs[0].get_to(o.epoch_time);
    obs[1].get_to(o.wind.lull_mph);
    o.wind.lull_mph *= MS_TO_MPH;
    obs[2].get_to(o.wind.avg_mph);
    o.wind.avg_mph *= MS_TO_MPH;
    obs[3].get_to(o.wind.gust_mph);
    o.wind.gust_mph *= MS_TO_MPH;
    obs[4].get_to(o.wind.dir_deg);
    obs[5].get_to(o.wind.sample_interval_secs);
    obs[6].get_to(o.pressure_mb);
    obs[7].get_to(o.air_temp_f);
    o.air_temp_f = C_TO_F(o.air_temp_f);
    obs[8].get_to(o.relative_humidity);
    obs[9].get_to(o.illuminance_lux);
    obs[10].get_to(o.uv_index);
    obs[11].get_to(o.solar_radiation_wm2);
    obs[12].get_to(o.rain_accum_mm);
    obs[13].get_to(o.precipitation_type);
    obs[14].get_to(o.lightning_strike_dist_km);
    obs[15].get_to(o.lightning_strike_count);
    obs[16].get_to(o.battery_volts);
    obs[17].get_to(o.report_interval);
}