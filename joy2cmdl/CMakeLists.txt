set(EXEC_NAME joy2cmdl)

add_executable(${EXEC_NAME} joy2cmdl.cpp )

target_include_directories(${EXEC_NAME} PRIVATE include)
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../../include)

target_link_libraries(${EXEC_NAME} PRIVATE di8joy)