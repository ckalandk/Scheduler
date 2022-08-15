cmake_minimum_required(VERSION 3.22.2)

project(tests)


add_executable(scheduler_test main.cpp )

target_include_directories(
scheduler_test
  INTERFACE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
scheduler_test
  Scheduler::Scheduler
)
