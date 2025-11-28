cmake_minimum_required(VERSION 3.31.6)

enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    test_observation.cpp
    test_tsqueue.cpp
)

target_link_libraries(unit_tests PRIVATE
    gtest_main
    tempest
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
