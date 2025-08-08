# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

- **Build**: `make` or `make all`
- **Clean**: `make clean` (removes build artifacts and executable)
- **Rebuild**: `make rebuild` (clean + build)
- **Help**: `make help` (shows available targets)

The build system uses a mix of C and C++ compilation with gcc/g++ compilers. The output executable is named `program`.

## Architecture Overview

This is an OpenGL-based graphics application with text rendering capabilities. The architecture consists of several key components:

### Core Systems

**Graphics Pipeline**:
- OpenGL 4.5 Core Profile with GLFW for window management
- Custom shader system (`shader.h/cpp`) for loading and compiling vertex/fragment shaders
- Shaders are stored in `shaders/` directory (vertex.vs, fragment.fg, font.vs, font.fg)

**Rendering Systems**:
- **Sprite Rendering** (`sprites.h/cpp`): Object-oriented system for rendering circles, rectangles, lines, and triangles using instanced drawing
- **Font Rendering**: Two implementations exist:
  - Legacy system (`font_loader.h/cpp`) using custom font atlas parsing
  - Modern system (`font_renderer.h/cpp`) using FreeType2 for TTF font loading
- Uses texture atlases for efficient font rendering with bitmap packing

**Key Classes**:
- `CircleSquareSprite`: Main sprite rendering class with GPU buffer management
- `BaseSprite`: Base class providing common OpenGL resource management
- Font systems use C-style APIs with `extern "C"` linkages

### Project Structure

- `src/`: Source files (.c, .cpp)
- `include/`: Header files with third-party libraries (GLAD, KHR, STB)
- `build/`: Compiled object files
- `shaders/`: GLSL shader files
- `font/`: Font assets (.fnt, .png)
- `program`: Main executable

### Dependencies

- **OpenGL**: Graphics rendering (version 4.5+)
- **GLFW**: Window management and input
- **GLAD**: OpenGL function loader
- **GLM**: Mathematics library for vectors/matrices
- **FreeType2**: TTF font loading (modern font renderer)
- **STB Image**: Image loading

### Main Application Flow

The main loop in `program.cpp`:
1. Initializes GLFW window with OpenGL 4.5 context
2. Sets up font renderer with system font
3. Runs render loop with green background
4. Currently renders using instanced drawing with 1 triangle instance

## Development Notes

- The project is in active development with both legacy and modern font rendering systems
- Mixed C/C++ codebase with C-style APIs for font systems
- Uses custom OpenGL debug utilities (`GLDebug.h`)
- Makefile handles both C and C++ compilation automatically
- Font loading supports both custom bitmap fonts and TTF fonts via FreeType2

## Coordinate System

- **Y-axis direction**: Downward (Y=0 is at top, positive Y goes down)
- **Element indices**: Uses order `0,1,2,2,3,0` for quad rendering
  - Triangle 1: vertices 0,1,2 (top-left, top-right, bottom-right)  
  - Triangle 2: vertices 2,3,0 (bottom-right, bottom-left, top-left)