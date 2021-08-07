# Copyright (c) 2021 by Jannik Alber.
# All rights reserved.

cmake_minimum_required(VERSION 3.10)

project(NovaFramework
        LANGUAGES CXX
        VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt5 REQUIRED
             COMPONENTS Core Gui Widgets WinExtras)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS res/)

# Tests
option(NOVA_TESTS "If CMake should also build/provide tests for Nova." OFF)
mark_as_advanced(NOVA_TESTS)
if(NOVA_TESTS)
	add_subdirectory(test/)
endif()

# Documentation
find_package(Doxygen)
if(Doxygen_FOUND)
	option(NOVA_INSTALL_DOC "If Nova's documentation should be installed and built everytime." OFF)
	mark_as_advanced(NOVA_INSTALL_DOC)
	
	set(DOXYGEN_PROJECT_NAME "Nova Framework")
	set(DOXYGEN_SHOW_FILES No)
	set(DOXYGEN_SHOW_USED_FILES No)
	set(DOXYGEN_MACRO_EXPANSION Yes)
	
	if(NOVA_INSTALL_DOC)
		doxygen_add_docs(doc include/ ALL)
		install(DIRECTORY ${CMAKE_BINARY_DIR}/html/
		        DESTINATION doc/)
	else()
		doxygen_add_docs(doc include/)
	endif()
endif()

# Target
set(NOVA_PUBLIC_HEADERS
    include/nova.h
    include/workbench.h
    include/actionprovider.h
    include/quickdialog.h
    include/searchbar.h
    include/progress.h
    include/notification.h
    include/toolwindow.h
    include/settings.h)
add_library(Nova SHARED
            # moc needs adding the headers
            ${NOVA_PUBLIC_HEADERS}

            src/workbench.cpp
            src/actionprovider.cpp
            src/quickdialog.cpp
            src/searchbar.cpp
            src/progress.cpp
            src/notification.cpp
            src/toolwindow.cpp
            src/settings.cpp)

set_target_properties(Nova PROPERTIES
                      OUTPUT_NAME novaf
                      PUBLIC_HEADER "${NOVA_PUBLIC_HEADERS}")
target_include_directories(Nova PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                           $<INSTALL_INTERFACE:include/>)
target_link_libraries(Nova PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets Qt5::WinExtras)

# Installation rules
install(TARGETS Nova
        EXPORT NovaFrameworkConfig
        ARCHIVE DESTINATION lib/
        LIBRARY DESTINATION lib/
        RUNTIME DESTINATION bin/
        PUBLIC_HEADER DESTINATION include/nova/)
install(EXPORT NovaFrameworkConfig
        DESTINATION lib/cmake/)
