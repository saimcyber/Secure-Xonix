# Xonix Game - Setup Requirements

## Prerequisites for Running the Game

This document outlines all the requirements needed to build and run the Xonix game on a blank PC that has only VS Code and MinGW configured.

---

## 1. Required Software

### 1.1 Development Tools (Already Installed)
- ✅ **Visual Studio Code** - Code editor
- ✅ **MinGW-w64** - C++ compiler for Windows

### 1.2 Build Tools (Need to Install)

#### **CMake** (Version 3.10 or higher)
CMake is required to generate the build files for the project.

**Download:** https://cmake.org/download/

**Installation Steps:**
1. Download the Windows x64 Installer
2. Run the installer
3. **Important:** During installation, select "Add CMake to the system PATH for all users"
4. Verify installation by opening PowerShell and typing:
   ```powershell
   cmake --version
   ```

---

## 2. Required Libraries

### 2.1 SFML (Simple and Fast Multimedia Library) - Version 2.5 or higher

SFML is the core multimedia library used for graphics, window management, audio, and input handling.

**Download:** https://www.sfml-dev.org/download.php

**Installation Steps:**

1. **Download SFML 2.6.1 for MinGW:**
   - Go to https://www.sfml-dev.org/download/sfml/2.6.1/
   - Download: `SFML-2.6.1-windows-gcc-13.1.0-mingw-64-bit.zip`
   - (Or the latest version compatible with your MinGW version)

2. **Extract SFML:**
   - Extract the downloaded ZIP file
   - Move the extracted folder to `C:\SFML`
   - Your directory structure should look like:
     ```
     C:\SFML\
       ├── bin\
       ├── include\
       ├── lib\
       └── ...
     ```

3. **Add SFML to System Path:**
   - Open System Environment Variables
   - Add `C:\SFML\bin` to your PATH variable
   - This allows the system to find SFML DLL files at runtime

---

## 3. Project Configuration

### 3.1 Update CMakeLists.txt

The current `CMakeLists.txt` has a hardcoded SFML path. Update line 8:

**Current:**
```cmake
set(SFML_DIR "C:/SFML/lib/cmake/SFML")
```

**Make sure this matches your SFML installation path.** If you installed SFML at `C:\SFML`, the current path is correct.

---

## 4. Project Assets (Already Included)

The following assets are already included in the project:

### 4.1 Images (in `/images` folder)
- `background.jpg` - Menu background
- `Black.jpeg` - Alternative background
- `enemy.jpeg` / `enemy.png` - Enemy sprites
- `gameover.png` - Game over screen
- `logo.png` - Game logo
- `SCORE-TITLE.png` - Score display title
- `tiles.png` - Game tile sprites

### 4.2 Fonts (in `/Fonts` folder)
- `AlexandriaFLF.ttf` - Main game font
- `OpenSans.ttf` - Alternative font

### 4.3 Audio (in `/audio` folder)
- `change.wav` - Menu navigation sound
- `enter.wav` - Selection confirmation sound

---

## 5. Build Instructions

### 5.1 Open Project in VS Code
```powershell
cd "d:\STUDY\UNIVERSITY\Data Structures\PROJECT\XonixGame"
code .
```

### 5.2 Create Build Directory
Open the integrated terminal in VS Code and run:
```powershell
mkdir build
cd build
```

### 5.3 Generate Build Files with CMake
```powershell
cmake .. -G "MinGW Makefiles"
```

### 5.4 Compile the Project
```powershell
mingw32-make
```

---

## 6. Running the Game

After successful compilation, run the game:

```powershell
.\xonix.exe
```

The game will look for the following files in the build directory:
- `images/` folder (automatically copied by CMake)
- `Fonts/` folder (automatically copied by CMake)
- `audio/` folder (automatically copied by CMake)
- `accounts.txt` (created automatically on first run)
- `friends.txt` (created automatically on first run)
- `inventory.txt` (created automatically on first run)
- `scores.txt` (created automatically during gameplay)

---

## 7. Troubleshooting

### Issue: "SFML not found"
**Solution:** Verify that `SFML_DIR` in `CMakeLists.txt` points to the correct path where SFML is installed.

### Issue: "Missing DLL files" when running
**Solution:** 
- Make sure `C:\SFML\bin` is in your system PATH
- Or copy all `.dll` files from `C:\SFML\bin` to the `build/` directory

### Issue: CMake not recognized
**Solution:** Reinstall CMake and ensure you selected "Add to PATH" during installation.

### Issue: mingw32-make not found
**Solution:** Verify MinGW is properly installed and its `bin` folder is in your system PATH.

### Issue: Font/Image/Audio files not found
**Solution:** Run CMake again to ensure files are copied:
```powershell
cd build
cmake ..
```

---

## 8. System Requirements

### Minimum Hardware:
- **CPU:** Intel Core i3 or equivalent
- **RAM:** 2 GB
- **GPU:** Integrated graphics with OpenGL 1.1 support
- **Storage:** 100 MB free space

### Recommended Hardware:
- **CPU:** Intel Core i5 or better
- **RAM:** 4 GB or more
- **GPU:** Dedicated graphics card
- **Storage:** 200 MB free space

---

## 9. VS Code Extensions (Recommended)

While not required, these extensions improve the development experience:

1. **C/C++** (ms-vscode.cpptools)
2. **CMake Tools** (ms-vscode.cmake-tools)
3. **CMake** (twxs.cmake)

Install from VS Code Extensions marketplace (Ctrl+Shift+X).

---

## 10. Complete Setup Checklist

- [ ] MinGW-w64 installed and in PATH
- [ ] CMake installed (3.10+) and in PATH
- [ ] SFML 2.5+ downloaded and extracted to `C:\SFML`
- [ ] `C:\SFML\bin` added to system PATH
- [ ] CMakeLists.txt SFML_DIR path verified
- [ ] Build directory created
- [ ] CMake configuration successful
- [ ] Compilation successful with mingw32-make
- [ ] Game executable runs without errors

---

## 11. Quick Start Commands

Here's the complete sequence to build and run from scratch:

```powershell
# Navigate to project directory
cd "d:\STUDY\UNIVERSITY\Data Structures\PROJECT\XonixGame"

# Create and enter build directory
mkdir build
cd build

# Configure with CMake
cmake .. -G "MinGW Makefiles"

# Build the project
mingw32-make

# Run the game
.\xonix.exe
```

---

## Support

If you encounter any issues not covered in this guide, check:
1. SFML official documentation: https://www.sfml-dev.org/tutorials/
2. CMake documentation: https://cmake.org/documentation/
3. MinGW-w64 documentation: https://www.mingw-w64.org/

---

**Last Updated:** November 19, 2025  
**Game Version:** Data Structures Project - November 2025
