# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

add_executable(nova_test WIN32 novatest.cpp)
target_link_libraries(nova_test PRIVATE novaf Qt5::Core Qt5::Gui Qt5::Widgets)
