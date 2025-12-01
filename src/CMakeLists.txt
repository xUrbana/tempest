add_library(tempest tempest.cpp)
#add_library(tempest::tempest_lib ALIAS tempest_lib)
target_compile_options(tempest PUBLIC -Wall -O0 -g)
target_include_directories(tempest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(tempest PUBLIC nlohmann_json::nlohmann_json pqxx spdlog::spdlog websockets)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(_core pybind.cpp WITH_SOABI)
target_link_libraries(_core PRIVATE tempest)
install(TARGETS _core DESTINATION pytempest)