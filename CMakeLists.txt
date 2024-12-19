cmake_minimum_required(VERSION 3.8)
project(charvid)

find_package(SDL2 REQUIRED)

set(SRC ${CMAKE_SOURCE_DIR}/src)


add_executable(charvid
	${SRC}/main.c
)
target_include_directories(
	charvid PUBLIC
	${SRC}/include
	${SDL2_INCLUDE_DIRS}
)
target_link_libraries(
	charvid PUBLIC
	${SDL2_LIBRARIES}
	m
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
