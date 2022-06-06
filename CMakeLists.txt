cmake_minimum_required(VERSION 3.22)

project(
  joy2key
  VERSION 0.1.0
  LANGUAGES CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# set a default build type: Debug | RelWithDebInfo | Release | MinSizeRel
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
  message(STATUS "Build type not specified: use Debug mode by default.")
endif()

if(CMAKE_BUILD_TYPE EQUAL "Debug")
  if(MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
  else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
  endif()
endif()

add_subdirectory(di8joy)    # Direct Input 8 library for joystick & buttons
add_subdirectory(joy2cmdl)  # joy2cmdl - inital version for comand line output
add_subdirectory(joy2key)   # joy2key main window
