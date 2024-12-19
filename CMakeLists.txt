cmake_minimum_required(VERSION 3.8)
project(charvid)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

set(SRC ${CMAKE_SOURCE_DIR}/src)

add_executable(charvid
	${SRC}/main.c
)

target_include_directories(
	charvid PUBLIC
	${SDL2_INCLUDE_DIRS}
)
target_link_libraries(
	charvid PUBLIC
	${SDL2_LIBRARIES}
)

install(TARGETS charvid
    COMPONENT linapp
    RUNTIME DESTINATION "/bin/"
    LIBRARY DESTINATION "/lib/"
    DESTINATION "/bin/"
)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "KK") #required
INCLUDE(CPack)
