#include "tempest.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST(ObservationTest, ParseObservation)
{
    std::string json_str = R"(
    {
        "serial_number": "ST-00000001",
        "type": "obs_st",
        "hub_sn": "HB-00000001",
        "obs": [
            [
                1588707192,
                2.46,
                3.13,
                4.02,
                193,
                3,
                998.4,
                20.5,
                78.6,
                321,
                1,
                210,
                0.0,
                0,
                0,
                0,
                2.8,
                1
            ]
        ],
        "firmware_revision": 123
    }
    )";

    auto j = json::parse(json_str);
    auto o = j.get<tempest::Observation>();

    EXPECT_EQ(o.firmware_revision, 123);
    EXPECT_EQ(o.station_sn, "ST-00000001");
    EXPECT_EQ(o.hub_sn, "HB-00000001");
    EXPECT_EQ(o.epoch_time, 1588707192);

    // Floating point comparisons with small epsilon
    EXPECT_NEAR(o.wind.lull_mph, 2.46 * 2.23694, 0.01);
    EXPECT_NEAR(o.wind.avg_mph, 3.13 * 2.23694, 0.01);
    EXPECT_NEAR(o.wind.gust_mph, 4.02 * 2.23694, 0.01);
    EXPECT_EQ(o.wind.dir_deg, 193);
    EXPECT_EQ(o.wind.sample_interval_secs, 3);

    EXPECT_NEAR(o.pressure_inhg, 998.4 * 0.02953, 0.01);
    EXPECT_NEAR(o.air_temp_f, (20.5 * 9.0 / 5.0) + 32, 0.01);
    EXPECT_NEAR(o.relative_humidity, 78.6, 0.01);
    EXPECT_EQ(o.illuminance_lux, 321);
    EXPECT_EQ(o.uv_index, 1);
    EXPECT_EQ(o.solar_radiation_wm2, 210);
    EXPECT_NEAR(o.rain_accum_in, 0.0 * 0.0393701, 0.001);
    EXPECT_EQ(o.precipitation_type, tempest::PrecipitationType::NONE);
    EXPECT_NEAR(o.lightning_strike_dist_mi, 0 * 0.621371, 0.001);
    EXPECT_EQ(o.lightning_strike_count, 0);
    EXPECT_NEAR(o.battery_volts, 2.8, 0.01);
    EXPECT_EQ(o.report_interval, 1);
}

TEST(ObservationTest, ParseObservationExceptions)
{
    // Test invalid array size
    std::string json_str = R"(
    {
        "serial_number": "ST-00000001",
        "type": "obs_st",
        "hub_sn": "HB-00000001",
        "obs": [
            [
                1588707192
            ]
        ],
        "firmware_revision": 123
    }
    )";

    json j = json::parse(json_str);
    EXPECT_THROW(j.get<tempest::Observation>(), std::runtime_error);
}
