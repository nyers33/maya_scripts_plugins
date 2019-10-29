cmake_minimum_required(VERSION 2.6)
project(gerstner_wave) # name of the plugin mll file

SET(PROJECT_PATH ${CMAKE_SOURCE_DIR})

set(MAYA_VERSION 2018)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

add_subdirectory(src)
