# define header and source files of the di8joy library
set(HEADERS di8joy_impl.hpp di8joy_mngr.hpp di8joy.hpp)
set(SOURCES di8joy_impl.cpp di8joy_mngr.cpp di8joy.cpp)

add_library(di8joy ${HEADERS} ${SOURCES})