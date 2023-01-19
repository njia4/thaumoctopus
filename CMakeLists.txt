cmake_minimum_required(VERSION 3.6)

project(thaumoctopus VERSION 0.0.1 DESCRIPTION "DRM based camera preview")

set(CMAKE_STATIC_LIBRARY_PREFIX "")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)
message(STATUS "LIBDRM display mode enabled")
message(STATUS ${LIBDRM_INCLUDE_DIRS})

add_library(preview SHARED 
	src/preview.cpp
	)

set_target_properties(preview PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER include/preview.h
	)

target_link_libraries(preview ${LIBDRM_LIBRARIES})
target_include_directories(preview PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${LIBDRM_INCLUDE_DIRS}
	)

# message(STATUS "Installing packages")
# message(STATUS "Install library directory" ${CMAKE_INSTALL_INCLUDEDIR})
# message(STATUS "Install include directory" ${CMAKE_INSTALL_LIBDIR})
# install(TARGETS preview
#     LIBRARY DESTINATION /usr/lib
#     PUBLIC_HEADER DESTINATION /usr/include)
# install(FILES ${CMAKE_BINARY_DIR}/mylib.pc
    # DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
