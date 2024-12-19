cmake_minimum_required(VERSION 3.8)
project(charvid)

set(SRC ${CMAKE_SOURCE_DIR}/src)


add_executable(charvid
	${SRC}/main.c
)
target_include_directories(
	charvid PUBLIC
	${SRC}/include
)
target_link_libraries(
	charvid PUBLIC
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
