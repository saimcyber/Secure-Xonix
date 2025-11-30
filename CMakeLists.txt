cmake_minimum_required(VERSION 3.10)
project(Xonix)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "C:/SFML/SFML-2.5.1/lib/cmake/SFML")

find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system audio network)

# Copy assets to build directory if exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
    # Also copy Fonts and images to root of build directory for compatibility
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/Fonts")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/Fonts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/images")
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/images" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
    endif()
endif()

# Copy audio directory if exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/audio")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/audio" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
endif()

# Copy SFML DLLs to build directory
file(GLOB SFML_DLLS "C:/SFML/SFML-2.5.1/bin/*.dll")
file(COPY ${SFML_DLLS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

# Add executable
add_executable(xonix 
    src/main.cpp 
    src/Menu.cpp 
    src/Authentication.cpp 
    src/Profile.cpp 
    src/MinHeap.cpp 
    src/SaveGame.cpp 
    src/PriorityQueue.cpp 
    src/GameRoomQueue.cpp 
    src/FriendSystem.cpp 
    src/Inventory.cpp
)

# Link SFML libraries
target_link_libraries(xonix PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)