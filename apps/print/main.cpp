#include "tempest.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting tempest...");
    tempest::Tempest t;
    t.add_handler([](const tempest::Observation& obs)
                  { std::cout << "Temp: " << obs.air_temp_f << '\n'; });
    t.run();
    t.join();
}