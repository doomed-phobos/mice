cmake_minimum_required(VERSION 3.2)

project(mice-lib)
find_package(Threads REQUIRED)
find_package(Libinput REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_search_module(LIBUDEV REQUIRED
   libudev)

option(MICE_EXAMPLE "Build example?" ON)

add_library(mice-lib STATIC mice.cc li.cc)
target_link_libraries(mice-lib PRIVATE Libinput::Libinput ${LIBUDEV_LIBRARIES})
target_include_directories(mice-lib PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(mice-lib PRIVATE ${PROJECT_SOURCE_DIR} ${LIBUDEV_INCLUDE_DIRS})

if(MICE_EXAMPLE)
   add_executable(example example.cc)
   target_link_libraries(example mice-lib Threads::Threads)
endif()
