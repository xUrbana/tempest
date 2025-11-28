
add_executable(tempest-db main.cpp)
target_link_libraries(tempest-db PRIVATE tempest)

file(COPY schema.sql DESTINATION ${CMAKE_CURRENT_BINARY_DIR})