cmake_minimum_required (VERSION 3.1)
project (sandbox CXX)

set (CMAKE_CXX_STANDARD 14)

# https://stackoverflow.com/questions/31641063/single-threaded-application-with-boost-and-cmake
set(Boost_USE_MULTITHREADED OFF)
find_package(Boost 1.48.0 REQUIRED COMPONENTS
              filesystem program_options log)

add_definitions(-DBOOST_LOG_DYN_LINK)

if (Boost_FOUND)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS}")

  add_executable(sandbox main.cc)
  target_link_libraries(sandbox ${Boost_LIBRARIES})
endif()
