#include "tempest.hpp"
#include <iostream>
#include <print>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting tempest...");
    tempest::Tempest t("5788e7eb-6d44-49fa-8baf-c472eb1e5b64", "461959", false);
    t.add_handler([](const tempest::Observation& obs)
                  { std::println("{}", obs); });
    t.run();
    t.join();
}