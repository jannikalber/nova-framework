# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

cmake_minimum_required(VERSION 3.10)

project(nova-framework LANGUAGES CXX VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS res/)

# Tests
option(NOVA_TESTS "If CMake should also build/provide tests for Nova." OFF)
mark_as_advanced(NOVA_TESTS)
if(NOVA_TESTS)
	enable_testing()
	add_subdirectory(test/)
endif()

# Target
add_library(novaf SHARED include/nova.h
            include/workbench.h src/workbench.cpp)

target_include_directories(novaf PUBLIC include/)
target_link_libraries(novaf PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
