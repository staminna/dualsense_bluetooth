cmake_minimum_required(VERSION 3.10)
project(ps5_kontroller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SDL2 paths for Homebrew installation
set(SDL2_PATH "/opt/homebrew/opt/sdl2")
set(SDL2_INCLUDE_DIR "${SDL2_PATH}/include")
set(SDL2_LIBRARY "${SDL2_PATH}/lib/libSDL2.dylib")

if(NOT EXISTS ${SDL2_LIBRARY})
    message(FATAL_ERROR "SDL2 library not found at ${SDL2_LIBRARY}")
endif()

# Add oscpack
add_subdirectory(oscpack)

# Add executable
add_executable(ps5_kontroller main.cpp)

# Include directories
target_include_directories(ps5_kontroller PRIVATE 
    ${SDL2_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/oscpack
)

# Link libraries
target_link_libraries(ps5_kontroller PRIVATE
    ${SDL2_LIBRARY}
    oscpack
)

# Set rpath for macOS
set_target_properties(ps5_kontroller PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "@executable_path;${SDL2_PATH}/lib"
    INSTALL_NAME_DIR "@rpath"
)
