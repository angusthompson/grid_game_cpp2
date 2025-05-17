cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

add_executable( main 
                src/main.cpp 
                src/mechanics/MapGenerator.cpp
                src/mechanics/Fertility.cpp
                src/mechanics/FoW.cpp
                src/mechanics/Tribe.cpp
                src/Tools/UITools.cpp
                src/Tools/MapTools.cpp
                src/Tools/ObjectTools.cpp)

target_link_libraries(main PRIVATE sfml-graphics sfml-window sfml-system)

