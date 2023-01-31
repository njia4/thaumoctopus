cmake_minimum_required(VERSION 3.6)

project(thaumoctopus VERSION 0.0.1 DESCRIPTION "DRM based camera preview")

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBDRM REQUIRED libdrm)
message(STATUS "LIBDRM display mode enabled")
message(STATUS ${LIBDRM_INCLUDE_DIRS})

find_package(glog REQUIRED)
if (glog_FOUND)
	message(STATUS "glog enabled")
	message(STATUS "glog libraries: " ${glog_LIBRARIES})
endif (glog_FOUND)

add_library(preview SHARED src/preview.cpp)

set_target_properties(preview PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER include/preview.h
	)

target_link_libraries(preview ${LIBDRM_LIBRARIES} glog::glog)
target_include_directories(preview PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${LIBDRM_INCLUDE_DIRS})

configure_file(preview.pc.in preview.pc @ONLY)

install(TARGETS preview 
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)

install(FILES ${CMAKE_BINARY_DIR}/preview.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
