@echo off
REM ===================================================================
REM Xonix Game - Direct Build and Run Script (Without CMake)
REM Authors: M. Amish (24i-2099), Saim Zaib (24i-2023)
REM ===================================================================
REM This script compiles and runs the Xonix game directly using g++
REM without requiring CMake. Make sure MinGW is in your PATH.
REM ===================================================================


REM Set SFML paths (modify these if your SFML is in a different location)
set SFML_DIR=C:\SFML
set SFML_INCLUDE=%SFML_DIR%\include
set SFML_LIB=%SFML_DIR%\lib

REM Check if SFML exists
if not exist "%SFML_INCLUDE%" (
    echo ERROR: SFML not found at %SFML_DIR%
    echo Please install SFML or update SFML_DIR in this script.
    pause
    exit /b 1
)

echo [1/4] Setting up build directory...
if not exist build mkdir build
if exist build\xonix.exe del build\xonix.exe
if exist build\*.o del build\*.o

echo [2/4] Compiling source files...
echo.

REM Compile each .cpp file to object file
echo Compiling main.cpp...
g++ -c src/main.cpp -o build/main.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling Menu.cpp...
g++ -c src/Menu.cpp -o build/Menu.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling Authentication.cpp...
g++ -c src/Authentication.cpp -o build/Authentication.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling Profile.cpp...
g++ -c src/Profile.cpp -o build/Profile.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling MinHeap.cpp...
g++ -c src/MinHeap.cpp -o build/MinHeap.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling SaveGame.cpp...
g++ -c src/SaveGame.cpp -o build/SaveGame.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling PriorityQueue.cpp...
g++ -c src/PriorityQueue.cpp -o build/PriorityQueue.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling GameRoomQueue.cpp...
g++ -c src/GameRoomQueue.cpp -o build/GameRoomQueue.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling FriendSystem.cpp...
g++ -c src/FriendSystem.cpp -o build/FriendSystem.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo Compiling Inventory.cpp...
g++ -c src/Inventory.cpp -o build/Inventory.o -I%SFML_INCLUDE% -DSFML_STATIC -std=c++17 -O2

echo.
echo [3/4] Linking executable...

REM Link all object files with SFML libraries
g++ build/main.o build/Menu.o build/Authentication.o build/Profile.o build/MinHeap.o build/SaveGame.o build/PriorityQueue.o build/GameRoomQueue.o build/FriendSystem.o build/Inventory.o ^
    -o build/xonix.exe ^
    -L%SFML_LIB% ^
    -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio ^
    -std=c++17

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Linking failed!
    pause
    exit /b 1
)

echo.
echo [4/4] Copying required DLLs...

REM Copy SFML DLLs to build directory
copy "%SFML_DIR%\bin\*.dll" build\ >nul 2>nul

echo.
echo ========================================
echo BUILD SUCCESSFUL!
echo ========================================
echo.
echo Executable: build\xonix.exe
echo All build files located in: build\
echo.

REM Ask if user wants to run the game
set /p RUN_GAME="Do you want to run the game now? (Y/N): "
if /i "%RUN_GAME%"=="Y" (
    echo.
    echo Starting Xonix Game...
    echo.
    start build\xonix.exe
) else (
    echo.
    echo You can run the game later by executing: build\xonix.exe
)

echo.
pause
