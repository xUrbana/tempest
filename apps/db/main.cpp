#include "tempest.hpp"
#include <cstdlib>
#include <format>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>

inline std::string to_string(PrecipitationType t)
{
    switch (t)
    {
    case PrecipitationType::NONE:
        return "none";
    case PrecipitationType::RAIN:
        return "rain";
    case PrecipitationType::HAIL:
        return "hail";
    case PrecipitationType::RAIN_AND_HAIL:
        return "rain_and_hail";
    default:
        std::unreachable();
    }
}

class TempestDatabaseManager
{
  public:
    TempestDatabaseManager()
    {
        db_ = std::make_unique<pqxx::connection>(
            std::format("host={} port={} dbname={} user={} password={}", get_env("TEMPEST_DB_HOST", "localhost"),
                        get_env("TEMPEST_DB_PORT", "5432"), get_env("TEMPEST_DB_NAME", "tempest"),
                        get_env("TEMPEST_DB_USER", "tempest"), get_env("TEMPEST_DB_PASSWORD", "password")));

        init_db();
        tempest_.add_handler([this](const Observation &obs) { insert_observation(obs); });
    }

    void run()
    {
        tempest_.run();
        tempest_.join();
    }

  private:
    void init_db()
    {
        spdlog::info("Initializing database...");

        pqxx::work tx(*db_);

        tx.exec("drop type if exists precipitation_type cascade");

        tx.exec(R"(
        create type precipitation_type as enum(
            'none',
            'rain',
            'hail',
            'rain_and_hail'
        );
        )");

        tx.exec("drop table if exists observations cascade;");

        tx.exec(R"(
        create table if not exists observations(
            id serial primary key,
            time timestamp not null,
            firmware_revision smallint not null,
            station_sn text not null,
            hub_sn text not null,
            wind_lull_mph real not null,
            wind_avg_mph real not null,
            wind_gust_mph real not null,
            wind_dir_deg smallint not null,
            wind_sample_interval_secs smallint not null,
            pressure_inhg real not null,
            air_temp_f real not null,
            relative_humidity real not null,
            illuminance_lux integer not null,
            uv_index smallint not null,
            solar_radiation_wm2 integer not null,
            rain_accum_in real not null,
            precip_type precipitation_type not null,
            lightning_strike_dist_mi real not null,
            lightning_strike_count smallint not null,
            battery_volts real not null,
            report_interval smallint not null
        );  
        )");

        tx.commit();

        db_->prepare(
            "insert_obs",
            "insert into observations "
            "(time,firmware_revision,station_sn,hub_sn,wind_lull_mph,wind_avg_mph,wind_gust_mph,wind_dir_deg,wind_"
            "sample_interval_secs,pressure_inhg,air_temp_f,relative_humidity,illuminance_lux,uv_index,solar_"
            "radiation_wm2,rain_accum_in,precip_type,lightning_strike_dist_mi,lightning_strike_count,battery_volts,"
            "report_interval) values "
            "(to_timestamp($1),$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21);");
    }
    void insert_observation(const Observation &obs)
    {
        pqxx::work tx(*db_);

        tx.exec(pqxx::prepped{"insert_obs"}, pqxx::params{obs.epoch_time,
                                                          obs.firmware_revision,
                                                          obs.station_sn,
                                                          obs.hub_sn,
                                                          obs.wind.lull_mph,
                                                          obs.wind.avg_mph,
                                                          obs.wind.gust_mph,
                                                          obs.wind.dir_deg,
                                                          obs.wind.sample_interval_secs,
                                                          obs.pressure_inhg,
                                                          obs.air_temp_f,
                                                          obs.relative_humidity,
                                                          obs.illuminance_lux,
                                                          obs.uv_index,
                                                          obs.solar_radiation_wm2,
                                                          obs.rain_accum_in,
                                                          to_string(obs.precipitation_type),
                                                          obs.lightning_strike_dist_mi,
                                                          obs.lightning_strike_count,
                                                          obs.battery_volts,
                                                          obs.report_interval});

        tx.commit();
        spdlog::info("Inserted observation...");
    }

    std::string get_env(const std::string &name, const std::string &default_value)
    {
        if (const char *v = std::getenv(name.c_str()))
            return v;
        return default_value;
    }

    std::unique_ptr<pqxx::connection> db_;
    Tempest                           tempest_;
};

int main()
{
    TempestDatabaseManager tempest_db_mgr;
    tempest_db_mgr.run();
}