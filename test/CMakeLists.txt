# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

add_executable(NovaTest WIN32 novatest.cpp)
target_link_libraries(NovaTest PRIVATE Nova Qt5::Core Qt5::Gui Qt5::Widgets)
