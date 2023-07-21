cmake_minimum_required(VERSION 3.2)

project(mice-lib)
find_package(Threads REQUIRED)
find_package(Libinput REQUIRED)

option(MICE_EXAMPLE "Build example?" ON)

add_library(mice-lib STATIC mice.cc li.cc)
target_link_libraries(mice-lib PRIVATE Libinput::Libinput)
target_include_directories(mice-lib PUBLIC ${PROJECT_SOURCE_DIR})

if(MICE_EXAMPLE)
   add_executable(example example.cc)
   target_link_libraries(example mice-lib Threads::Threads)
endif()
