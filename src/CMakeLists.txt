
file(GLOB tempest_lib_sources "*.cpp")

add_library(libtempest ${tempest_lib_sources})
target_compile_options(libtempest PUBLIC -Wall -O3 -g)
target_include_directories(libtempest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(libtempest PUBLIC nlohmann_json::nlohmann_json pqxx spdlog::spdlog)