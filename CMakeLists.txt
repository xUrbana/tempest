cmake_minimum_required(VERSION 3.31.6)
project(tempest)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

FetchContent_Declare(spdlog URL https://github.com/gabime/spdlog/archive/refs/tags/v1.16.0.tar.gz)
FetchContent_MakeAvailable(spdlog)

#FetchContent_Declare(libpqxx URL https://github.com/jtv/libpqxx/archive/refs/tags/7.10.3.tar.gz)
#FetchContent_MakeAvailable(libpqxx)
find_package(libpqxx REQUIRED)

add_subdirectory(src)
add_subdirectory(apps)

