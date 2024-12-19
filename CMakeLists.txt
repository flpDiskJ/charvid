cmake_minimum_required(VERSION 3.8)
project(caud)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

set(CMAKE_CXX_STANDARD 20)
set(SRC ${CMAKE_SOURCE_DIR}/src)

add_executable(caud
	${SRC}/main.cpp
)

target_include_directories(
	caud PUBLIC
	${SDL2_INCLUDE_DIRS}
)
target_link_libraries(
	caud PUBLIC
	${SDL2_LIBRARIES}
)

install(TARGETS caud
    COMPONENT linapp
    RUNTIME DESTINATION "/bin/"
    LIBRARY DESTINATION "/lib/"
    DESTINATION "/bin/"
)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "KK") #required
INCLUDE(CPack)
