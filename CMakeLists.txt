cmake_minimum_required(VERSION 3.10)
project(Xonix)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SFML directory
set(SFML_DIR "C:/SFML/SFML-2.5.1/lib/cmake/SFML")

# Find SFML package (works with both 2.5 and 3.x)
find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system audio network)

# Copy images, fonts, and audio to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/images" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Fonts" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/audio" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

# Copy SFML DLLs to build directory
file(GLOB SFML_DLLS "C:/SFML/SFML-2.5.1/bin/*.dll")
file(COPY ${SFML_DLLS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

# Add executable
add_executable(xonix main.cpp Menu.cpp Authentication.cpp Profile.cpp MinHeap.cpp SaveGame.cpp PriorityQueue.cpp GameRoomQueue.cpp FriendSystem.cpp Inventory.cpp)

# Link SFML libraries
target_link_libraries(xonix PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)