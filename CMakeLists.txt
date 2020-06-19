cmake_minimum_required( VERSION 3.10.2 )
project(iol_parallel_image_to_video_converter)

# set the compiler flags and c++ version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -I")
#set(MAKE_LINK_DEF_FILE_FLAG "${MAKE_LINK_DEF_FILE_FLAG} -lpthread")
set(CMAKE_CXX_STANDARD 11)
# Search path for CMake include files.
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/)

set(CMAKE_BUILD_TYPE Debug)

# set the directory that the executable file is going to build
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# add files to executable
set(SOURCES
        main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} pthread)
