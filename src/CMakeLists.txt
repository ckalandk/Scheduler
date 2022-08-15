cmake_minimum_required(VERSION 3.22.2)

project(Scheduler 
	VERSION 1.0.0
	DESCRIPTION "Task Scheduler")


add_library(scheduler INTERFACE)
target_include_directories(scheduler INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(scheduler INTERFACE cxx_std_20)

add_library(Scheduler::Scheduler ALIAS scheduler)