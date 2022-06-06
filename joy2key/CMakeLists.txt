set(EXEC_NAME joy2key)

add_executable(${EXEC_NAME} WIN32 joy2key.cpp)

target_include_directories(${EXEC_NAME} PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(${EXEC_NAME} PRIVATE di8joy)