# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

add_executable(NovaDemo WIN32 novademo.cpp)

if(Qt6_FOUND)
	target_link_libraries(NovaDemo PUBLIC NovaFramework Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Core5Compat)
else()
	target_link_libraries(NovaDemo PUBLIC NovaFramework Qt5::Core Qt5::Gui Qt5::Widgets)
endif()
