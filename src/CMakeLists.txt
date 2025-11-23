add_library(tempest tempest.cpp)
target_compile_options(tempest PUBLIC -Wall -O3 -g)
target_include_directories(tempest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tempest PUBLIC nlohmann_json::nlohmann_json pqxx spdlog::spdlog)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(pytempest pybind.cpp)
target_link_libraries(pytempest PRIVATE tempest)
install(TARGETS pytempest DESTINATION .)