cmake_minimum_required(VERSION 3.5)

project (clog)

set(CMAKE_CXX_STANDARD 11)
#set(CXX_STANDARD_REQUIRED)

find_package(Boost 1.61.1 REQUIRED)
if(Boost_FOUND)
    message ("boost found")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message (FATAL_ERROR "Cannot find Boost")
endif()

find_package(Threads REQUIRED)
if(Threads_FOUND)
    message ("threads found")
endif()

############################################################
# Define the clog library
############################################################
set(ClogLibSrc src/clog.cpp)
set(ClogLib_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)

#share
add_library(clog SHARED ${ClogLibSrc})
add_library(clog::library ALIAS clog)
target_include_directories(clog PUBLIC ${ClogLib_INCLUDE_DIRS})
target_include_directories(clog PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (clog Threads::Threads)

#static
add_library(clog_static STATIC ${ClogLibSrc})
add_library(clog::staticlibrary ALIAS clog_static)
target_include_directories(clog_static PUBLIC ${ClogLib_INCLUDE_DIRS})
target_include_directories(clog_static PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries (clog_static Threads::Threads)

############################################################
# test executable
############################################################
add_executable(clog_example src/example.cpp)

# target_link_libraries(clog_example PRIVATE clog::library)
target_link_libraries(clog_example PRIVATE clog::staticlibrary)
