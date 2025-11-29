cmake_minimum_required(VERSION 3.10)
project(Xonix)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set SFML directory
set(SFML_DIR "C:/SFML/lib/cmake/SFML")

# Find SFML package (works with both 2.5 and 3.x)
find_package(SFML 2.5 REQUIRED COMPONENTS graphics window system audio network)

# Copy assets and data to build directory
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/data" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")

# Copy SFML DLLs to build directory
file(GLOB SFML_DLLS "C:/SFML/bin/*.dll")
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