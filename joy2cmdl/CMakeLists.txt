set(EXEC_NAME joy2cmdl)

add_executable(${EXEC_NAME} string_trim.hpp joy2cmdl.cpp )

target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${EXEC_NAME} PRIVATE di8joy)