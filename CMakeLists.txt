cmake_minimum_required(VERSION 3.16)

project(mob)
add_subdirectory(src)

if (WIN32)
    set_property(DIRECTORY ${PROJECT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT mob)
endif()