cmake_minimum_required(VERSION 3.1)
project(baker VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DASIO_STANDALONE)
add_definitions(-DVERSION="${PROJECT_VERSION}")

find_package(Threads REQUIRED)

set(SOURCES
    pgm/args.cpp    pgm/args.hpp
    pie/device.cpp  pie/device.hpp
    pie/types.cpp   pie/types.hpp
    src/main.cpp
    src/remote.cpp  src/remote.cpp
    src/util.cpp    src/util.hpp
)

set(SET_UID_SOURCES
    pgm/args.cpp    pgm/args.hpp
    pie/device.cpp  pie/device.hpp
    pie/types.cpp   pie/types.hpp
    src/set-uid.cpp
)

include(GNUInstallDirs)

########################
# executable
add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} osc++)

add_executable(set-uid ${SET_UID_SOURCES})
target_link_libraries(set-uid ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME} set-uid DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES etc/sample.conf DESTINATION /etc/${PROJECT_NAME})
install(FILES udev/50-baker.rules DESTINATION /lib/udev/rules.d)
install(FILES systemd/baker@.service DESTINATION /lib/systemd/system)

install(FILES LICENSE.md README.md DESTINATION ${CMAKE_INSTALL_DOCDIR})

########################
# package
set(CPACK_STRIP_FILES ON)
set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "P.I. Engineering X-Keys to OSC translator")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dimitry Ishenko <dimitry (dot) ishenko (at) (gee) mail (dot) com>")

set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libosc++ (>= 1.0)")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# source
set(CPACK_SOURCE_GENERATOR TBZ2)
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${PROJECT_NAME}-${PROJECT_VERSION})
set(CPACK_SOURCE_IGNORE_FILES /\.git.*;/build.*;\.user$;/\.vscode.*)

include(CPack)
