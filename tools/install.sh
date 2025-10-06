#!/bin/bash

# ====================================
# SAGE Engine - Installation Script
# ====================================

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo ""
echo "===================================="
echo "   SAGE Engine Installation"
echo "===================================="
echo ""

# Configuration
BUILD_DIR="build"
INSTALL_DIR="$(pwd)/install"

echo -e "${YELLOW}[CONFIG]${NC} Build Directory: ${BUILD_DIR}"
echo -e "${YELLOW}[CONFIG]${NC} Install Directory: ${INSTALL_DIR}"
echo ""

# Step 1: Create build directory
echo -e "${YELLOW}[1/5]${NC} Creating build directory..."
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
    echo -e "${GREEN}✓${NC} Created ${BUILD_DIR}"
else
    echo -e "${GREEN}✓${NC} Using existing ${BUILD_DIR}"
fi
cd "$BUILD_DIR"

# Step 2: Configure CMake
echo ""
echo -e "${YELLOW}[2/5]${NC} Configuring CMake..."
cmake .. \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DSAGE_BUILD_TESTS=OFF \
    -DSAGE_BUILD_EXAMPLES=ON

echo -e "${GREEN}✓${NC} CMake configured successfully"

# Step 3: Build
echo ""
echo -e "${YELLOW}[3/5]${NC} Building SAGE Engine..."
cmake --build . --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo -e "${GREEN}✓${NC} Build completed"

# Step 4: Run tests (optional)
echo ""
echo -e "${YELLOW}[4/5]${NC} Running tests (optional)..."
if [ -f "Tests/SAGE_Tests" ]; then
    ./Tests/SAGE_Tests || echo -e "${YELLOW}⚠${NC} Some tests failed (not critical for installation)"
    echo -e "${GREEN}✓${NC} Tests completed"
else
    echo -e "${YELLOW}⚠${NC} Tests not built (skipped)"
fi

# Step 5: Install
echo ""
echo -e "${YELLOW}[5/5]${NC} Installing SAGE Engine..."
cmake --install .

echo -e "${GREEN}✓${NC} Installation completed"

cd ..

# Success message
echo ""
echo "===================================="
echo "   Installation Successful! 🎉"
echo "===================================="
echo ""
echo -e "${GREEN}SAGE Engine installed to:${NC}"
echo "  ${INSTALL_DIR}"
echo ""
echo -e "${YELLOW}To use SAGE in your project:${NC}"
echo ""
echo "  1. Create CMakeLists.txt:"
echo "     cmake_minimum_required(VERSION 3.20)"
echo "     project(MyGame)"
echo ""
echo "     set(CMAKE_PREFIX_PATH \"${INSTALL_DIR}\")"
echo "     find_package(SAGE REQUIRED)"
echo ""
echo "     add_executable(MyGame main.cpp)"
echo "     target_link_libraries(MyGame PRIVATE SAGE::SAGE_Engine)"
echo ""
echo "  2. Create main.cpp with #include <SAGE.h>"
echo ""
echo "  3. Build your project:"
echo "     mkdir build && cd build"
echo "     cmake .. -DCMAKE_PREFIX_PATH=\"${INSTALL_DIR}\""
echo "     cmake --build ."
echo ""
echo -e "${YELLOW}Examples:${NC}"
echo "  Check Examples/ folder for sample projects"
echo ""
echo -e "${YELLOW}Documentation:${NC}"
echo "  - INSTALL.md - Installation guide"
echo "  - QUICKSTART.md - API reference"
echo "  - EXAMPLES.md - Code examples"
echo ""
