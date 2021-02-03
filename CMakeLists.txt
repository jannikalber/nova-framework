# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

cmake_minimum_required(VERSION 3.10)

project(nova-framework LANGUAGES CXX VERSION 1.0)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_library(novaf SHARED include/nova.h
            include/workbench.h src/workbench.cpp)

target_include_directories(novaf PUBLIC include/)
target_link_libraries(novaf PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)
