set(LIB_NAME immediate_joy)
set(EXEC_NAME imjoy_demo)

# define header and source files of the di8joy library
set(LIB_HEADERS include/immediate_joy.hpp)
set(LIB_SOURCES src/immediate_joy.cpp)

add_library(${LIB_NAME} ${LIB_HEADERS} ${LIB_SOURCES})

add_executable(${EXEC_NAME} ${LIB_HEADERS} src/imjoy_demo.cpp)

# target link libraries have to be added AFTER add_executable or add_library!
target_include_directories(${LIB_NAME} PRIVATE include)

target_include_directories(${EXEC_NAME} PRIVATE include)
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../../include)

target_link_libraries(${EXEC_NAME} PRIVATE ${LIB_NAME})