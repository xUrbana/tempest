#include "tempest.hpp"
#include <iostream>
#include <print>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting tempest...");
    tempest::Tempest t(false);
    t.add_handler([](const tempest::Observation& obs) { std::println("{}", obs); });
    t.run();
    t.join();
}