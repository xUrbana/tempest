
file(GLOB_RECURSE sources *.cpp *.hpp)

add_executable(tempest ${sources})

target_compile_options(tempest PUBLIC -std=c++23 -Wall -O0 -g)

target_link_libraries(tempest PRIVATE nlohmann_json::nlohmann_json)