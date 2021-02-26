cmake_minimum_required( VERSION 3.8 )

project( mapgenerator )

set (CMAKE_CXX_STANDARD 17)

find_package(SFML COMPONENTS graphics window system REQUIRED)

file( GLOB SRCS 
Tests/*.c 
Tests/*.cpp 
Tests/*.cc 
Tests/*.h 
Tests/*.hpp

MapGenerator/*.c 
MapGenerator/*.cpp 
MapGenerator/*.cc 
MapGenerator/*.h 
MapGenerator/*.hpp
MapGenerator/Math/*.c 
MapGenerator/Math/*.cpp 
MapGenerator/Math/*.cc 
MapGenerator/Math/*.h 
MapGenerator/Math/*.hpp

DiskSampling/*.c 
DiskSampling/*.cpp 
DiskSampling/*.cc 
DiskSampling/*.h 
DiskSampling/*.hpp

MarkovNames/*.c 
MarkovNames/*.cpp 
MarkovNames/*.cc 
MarkovNames/*.h 
MarkovNames/*.hpp
)

include_directories(${SFML_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-system sfml-window sfml-audio)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/FreeSans.ttf"      # <--this is in-file
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
