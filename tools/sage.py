#!/usr/bin/env python3
"""
SAGE Engine CLI - Command Line Interface for SAGE Engine
Version: Alpha
"""

import argparse
import os
import sys
import subprocess
import shutil
import json
from pathlib import Path
from typing import Optional, List

class Colors:
    """ANSI color codes for terminal output"""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class SAGECLI:
    """SAGE Engine Command Line Interface"""
    
    VERSION = "0.1.0-alpha"
    
    def __init__(self):
        self.engine_root = Path(__file__).parent.parent.absolute()
        # Projects OUTSIDE of engine directory
        self.projects_dir = Path.home() / "SAGEProjects"
        self.config_file = Path.home() / ".sage" / "config.json"
        self.config = self.load_config()
    
    def load_config(self) -> dict:
        """Load CLI configuration"""
        if self.config_file.exists():
            with open(self.config_file, 'r') as f:
                return json.load(f)
        
        # Default projects in user home directory
        default_projects = str(Path.home() / "SAGEProjects")
        
        return {
            "default_compiler": "auto",
            "default_build_type": "Release",
            "projects_directory": default_projects,
            "engine_path": str(self.engine_root)
        }
    
    def save_config(self):
        """Save CLI configuration"""
        self.config_file.parent.mkdir(parents=True, exist_ok=True)
        with open(self.config_file, 'w') as f:
            json.dump(self.config, f, indent=2)
    
    def print_header(self, text: str):
        """Print colored header"""
        print(f"\n{Colors.BOLD}{Colors.HEADER}{'='*60}{Colors.ENDC}")
        print(f"{Colors.BOLD}{Colors.HEADER}{text:^60}{Colors.ENDC}")
        print(f"{Colors.BOLD}{Colors.HEADER}{'='*60}{Colors.ENDC}\n")
    
    def print_success(self, text: str):
        """Print success message"""
        print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")
    
    def print_error(self, text: str):
        """Print error message"""
        print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")
    
    def print_warning(self, text: str):
        """Print warning message"""
        print(f"{Colors.WARNING}⚠ {text}{Colors.ENDC}")
    
    def print_info(self, text: str):
        """Print info message"""
        print(f"{Colors.OKCYAN}ℹ {text}{Colors.ENDC}")
    
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> bool:
        """Run shell command and return success status"""
        try:
            result = subprocess.run(cmd, cwd=cwd, check=True, 
                                  capture_output=True, text=True)
            return True
        except subprocess.CalledProcessError as e:
            self.print_error(f"Command failed: {' '.join(cmd)}")
            if e.stderr:
                print(e.stderr)
            return False
    
    # ============================================================
    # ENGINE COMMANDS
    # ============================================================
    
    def cmd_install(self, args):
        """Install SAGE Engine"""
        self.print_header("Installing SAGE Engine")
        
        # Check prerequisites
        self.print_info("Checking prerequisites...")
        
        # Check CMake
        if not shutil.which('cmake'):
            self.print_error("CMake not found. Please install CMake 3.15+")
            return False
        
        # Check Git
        if not shutil.which('git'):
            self.print_error("Git not found. Please install Git")
            return False
        
        self.print_success("Prerequisites check passed")
        
        # Initialize submodules
        self.print_info("Initializing git submodules...")
        if not self.run_command(['git', 'submodule', 'update', '--init', '--recursive'],
                               cwd=self.engine_root):
            return False
        self.print_success("Submodules initialized")
        
        # Configure build
        build_dir = self.engine_root / "build"
        self.print_info(f"Configuring CMake build...")
        
        cmake_args = ['cmake', '-S', '.', '-B', 'build']
        
        if sys.platform == 'win32':
            cmake_args.extend(['-G', 'Visual Studio 17 2022'])
        
        if not self.run_command(cmake_args, cwd=self.engine_root):
            return False
        
        self.print_success("CMake configuration complete")
        
        # Build engine
        if args.skip_build:
            self.print_info("Skipping build (--skip-build)")
        else:
            if not self.cmd_build(args):
                return False
        
        self.print_success("SAGE Engine installation complete!")
        return True
    
    def cmd_build(self, args):
        """Build SAGE Engine"""
        self.print_header(f"Building SAGE Engine ({args.config})")
        
        build_dir = self.engine_root / "build"
        if not build_dir.exists():
            self.print_error("Build directory not found. Run 'sage install' first.")
            return False
        
        self.print_info(f"Building {args.config} configuration...")
        
        build_cmd = ['cmake', '--build', 'build', '--config', args.config]
        
        if args.parallel:
            build_cmd.extend(['--parallel', str(args.parallel)])
        
        if not self.run_command(build_cmd, cwd=self.engine_root):
            return False
        
        self.print_success(f"Build complete: {args.config}")
        return True
    
    def cmd_test(self, args):
        """Run SAGE Engine tests"""
        self.print_header("Running SAGE Engine Tests")
        
        # Determine test executable path
        if sys.platform == 'win32':
            test_exe = self.engine_root / "build" / "bin" / args.config / "SAGETests.exe"
        else:
            test_exe = self.engine_root / "build" / "bin" / args.config / "SAGETests"
        
        if not test_exe.exists():
            self.print_error(f"Test executable not found: {test_exe}")
            self.print_info("Build the engine first: sage build")
            return False
        
        self.print_info(f"Running tests from: {test_exe}")
        
        test_cmd = [str(test_exe)]
        
        if args.filter:
            test_cmd.extend([args.filter])
        
        if args.verbose:
            test_cmd.append('--success')
        
        try:
            result = subprocess.run(test_cmd, cwd=test_exe.parent)
            if result.returncode == 0:
                self.print_success("All tests passed!")
                return True
            else:
                self.print_error("Some tests failed")
                return False
        except Exception as e:
            self.print_error(f"Failed to run tests: {e}")
            return False
    
    def cmd_clean(self, args):
        """Clean build artifacts"""
        self.print_header("Cleaning SAGE Engine")
        
        build_dir = self.engine_root / "build"
        
        if not build_dir.exists():
            self.print_warning("Build directory doesn't exist")
            return True
        
        if args.deep:
            self.print_info("Removing entire build directory...")
            shutil.rmtree(build_dir)
            self.print_success("Deep clean complete")
        else:
            self.print_info("Cleaning build artifacts...")
            if self.run_command(['cmake', '--build', 'build', '--target', 'clean'],
                              cwd=self.engine_root):
                self.print_success("Clean complete")
            else:
                self.print_warning("Clean command failed, trying manual cleanup...")
                bin_dir = build_dir / "bin"
                if bin_dir.exists():
                    shutil.rmtree(bin_dir)
                self.print_success("Manual clean complete")
        
        return True
    
    def cmd_info(self, args):
        """Display SAGE Engine information"""
        self.print_header("SAGE Engine Information")
        
        print(f"{Colors.BOLD}Version:{Colors.ENDC} {self.VERSION}")
        print(f"{Colors.BOLD}Engine Root:{Colors.ENDC} {self.engine_root}")
        print(f"{Colors.BOLD}Projects Directory:{Colors.ENDC} {self.config['projects_directory']}")
        print(f"{Colors.BOLD}Default Compiler:{Colors.ENDC} {self.config['default_compiler']}")
        print(f"{Colors.BOLD}Default Build Type:{Colors.ENDC} {self.config['default_build_type']}")
        
        # Check build status
        build_dir = self.engine_root / "build"
        if build_dir.exists():
            self.print_success("Build directory exists")
            
            # Check for executables
            if sys.platform == 'win32':
                test_exe = build_dir / "bin" / "Release" / "SAGETests.exe"
            else:
                test_exe = build_dir / "bin" / "Release" / "SAGETests"
            
            if test_exe.exists():
                self.print_success("Engine is built (Release)")
            else:
                self.print_warning("Engine not built or incomplete")
        else:
            self.print_warning("Build directory not found. Run 'sage install'")
        
        return True
    
    # ============================================================
    # PROJECT COMMANDS
    # ============================================================
    
    def cmd_create(self, args):
        """Create new SAGE project"""
        self.print_header(f"Creating Project: {args.name}")
        
        # Create projects directory
        projects_root = Path(self.config['projects_directory'])
        projects_root.mkdir(parents=True, exist_ok=True)
        
        # Create project directory
        project_dir = projects_root / args.name
        
        if project_dir.exists():
            self.print_error(f"Project already exists: {project_dir}")
            return False
        
        self.print_info(f"Creating project at: {project_dir}")
        project_dir.mkdir(parents=True)
        
        # Create project structure
        (project_dir / "src").mkdir()
        (project_dir / "assets").mkdir()
        (project_dir / "assets" / "textures").mkdir()
        (project_dir / "assets" / "audio").mkdir()
        (project_dir / "assets" / "shaders").mkdir()
        
        # Create CMakeLists.txt
        engine_path = self.config.get('engine_path', str(self.engine_root))
        cmake_content = f"""cmake_minimum_required(VERSION 3.15)
project({args.name})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SAGE Engine path (can be overridden with -DSAGE_ENGINE_DIR=...)
if(NOT DEFINED SAGE_ENGINE_DIR)
    set(SAGE_ENGINE_DIR "{engine_path}")
endif()

message(STATUS "SAGE Engine directory: ${{SAGE_ENGINE_DIR}}")

# Check if SAGE Engine exists
if(NOT EXISTS "${{SAGE_ENGINE_DIR}}/CMakeLists.txt")
    message(FATAL_ERROR "SAGE Engine not found at: ${{SAGE_ENGINE_DIR}}")
endif()

# Add SAGE Engine as subdirectory
# Binary dir prevents conflicts with engine's own build
add_subdirectory(${{SAGE_ENGINE_DIR}} ${{CMAKE_BINARY_DIR}}/sage_engine EXCLUDE_FROM_ALL)

# Project executable
add_executable({args.name}
    src/main.cpp
)

# Link with SAGE Engine
target_link_libraries({args.name} PRIVATE SAGEEngine)

# Include SAGE headers
target_include_directories({args.name} PRIVATE ${{SAGE_ENGINE_DIR}}/Engine)

# Copy assets to build directory
add_custom_command(TARGET {args.name} POST_BUILD
    COMMAND ${{CMAKE_COMMAND}} -E copy_directory
    ${{CMAKE_SOURCE_DIR}}/assets $<TARGET_FILE_DIR:{args.name}>/assets
    COMMENT "Copying assets to build directory"
)

# Set output directory
set_target_properties({args.name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${{CMAKE_BINARY_DIR}}/bin
)
"""
        (project_dir / "CMakeLists.txt").write_text(cmake_content, encoding='utf-8')
        
        # Create main.cpp
        main_content = """#include <SAGE/SAGE.h>
#include <iostream>

using namespace SAGE;

class MyGame : public Application {
public:
    MyGame() : Application("SAGE Game", 1280, 720) {}
    
    void OnInit() override {
        std::cout << "Game Initialized!" << std::endl;
        
        // Create a simple entity
        auto entity = registry.CreateEntity();
        registry.AddComponent<TransformComponent>(entity, Vector2(400, 300));
        
        std::cout << "Entity created: " << entity << std::endl;
    }
    
    void OnUpdate(float deltaTime) override {
        // Game logic here
    }
    
    void OnRender() override {
        // Rendering here
    }
    
    void OnShutdown() override {
        std::cout << "Game Shutdown" << std::endl;
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
"""
        (project_dir / "src" / "main.cpp").write_text(main_content, encoding='utf-8')
        
        # Create README
        engine_path = self.config.get('engine_path', str(self.engine_root))
        readme_content = f"""# {args.name}

SAGE Engine game project.

## Quick Start

Build and run with SAGE CLI:

```bash
# Build project
sage project build

# Run project
sage project run
```

## Manual Build

```bash
# Configure (SAGE Engine path is auto-detected)
cmake -S . -B build

# Or specify custom engine location:
cmake -S . -B build -DSAGE_ENGINE_DIR=/path/to/SAGE-Engine

# Build
cmake --build build --config Release

# Run (Windows)
.\\build\\bin\\{args.name}.exe

# Run (Linux/macOS)
./build/bin/{args.name}
```

## Project Structure

```
{args.name}/
├── src/
│   └── main.cpp          # Main application code
├── assets/
│   ├── textures/         # Image files (.png, .jpg)
│   ├── audio/            # Sound files (.wav, .ogg)
│   └── shaders/          # GLSL shader files
├── CMakeLists.txt        # Build configuration
└── README.md            # This file
```

## Development

### Adding New Source Files

Edit `CMakeLists.txt`:

```cmake
add_executable({args.name}
    src/main.cpp
    src/Player.cpp        # Add new files here
    src/Enemy.cpp
)
```

### Adding Assets

Place assets in respective folders:
- Textures: `assets/textures/`
- Audio: `assets/audio/`
- Shaders: `assets/shaders/`

Assets are automatically copied to build directory.

## SAGE Engine

**Engine Location:** `{engine_path}`

**Documentation:**
- [API Reference]({engine_path}/docs/API_REFERENCE.md)
- [User Guide]({engine_path}/docs/USER_GUIDE.md)
- [CLI Guide]({engine_path}/docs/CLI_GUIDE.md)

**Repository:** https://github.com/AGamesStudios/SAGE-Engine

## CLI Commands

```bash
sage project build           # Build project
sage project build --config Debug  # Debug build
sage project run             # Run project
sage project clean           # Clean build files
```

## Troubleshooting

**Problem:** CMake can't find SAGE Engine

**Solution:**
```bash
# Set engine path in CMake
cmake -S . -B build -DSAGE_ENGINE_DIR=/path/to/SAGE-Engine
```

**Problem:** Missing assets in game

**Solution:** Assets are copied during build. Rebuild project:
```bash
sage project build
```

For more help: https://github.com/AGamesStudios/SAGE-Engine/issues
"""
        (project_dir / "README.md").write_text(readme_content, encoding='utf-8')
        
        # Create .gitignore
        gitignore_content = """# Build directories
build/
bin/
lib/

# IDE files
.vs/
.vscode/
*.user

# Compiled files
*.exe
*.dll
*.so
*.dylib
*.obj
*.o

# CMake
CMakeCache.txt
CMakeFiles/
"""
        (project_dir / ".gitignore").write_text(gitignore_content, encoding='utf-8')
        
        self.print_success(f"Project created: {project_dir}")
        
        self.print_info(f"\nProject location:")
        print(f"  {project_dir}")
        
        self.print_info(f"\nNext steps:")
        print(f"  cd {project_dir}")
        print(f"  sage project build")
        print(f"  sage project run")
        
        self.print_info(f"\nProject files:")
        print(f"  src/main.cpp      - Main application")
        print(f"  assets/           - Game assets")
        print(f"  CMakeLists.txt    - Build configuration")
        print(f"  README.md         - Project documentation")
        
        return True
    
    def cmd_project_build(self, args):
        """Build current project"""
        project_dir = Path.cwd()
        
        if not (project_dir / "CMakeLists.txt").exists():
            self.print_error("Not a SAGE project directory")
            self.print_info("Run this command from a project directory created with 'sage create'")
            return False
        
        self.print_header(f"Building Project: {project_dir.name}")
        
        # Configure if needed
        build_dir = project_dir / "build"
        if not build_dir.exists() or args.reconfigure:
            self.print_info("Configuring CMake...")
            cmake_args = ['cmake', '-S', '.', '-B', 'build']
            
            # Add generator for Windows
            if sys.platform == 'win32':
                cmake_args.extend(['-G', 'Visual Studio 17 2022'])
            
            # Pass engine path from config
            engine_path = self.config.get('engine_path', str(self.engine_root))
            cmake_args.append(f'-DSAGE_ENGINE_DIR={engine_path}')
            
            if not self.run_command(cmake_args, cwd=project_dir):
                self.print_error("CMake configuration failed")
                self.print_info(f"Check that SAGE Engine exists at: {engine_path}")
                return False
        
        # Build
        self.print_info(f"Building {args.config}...")
        build_cmd = ['cmake', '--build', 'build', '--config', args.config]
        
        if args.parallel:
            build_cmd.extend(['--parallel', str(args.parallel)])
        
        if not self.run_command(build_cmd, cwd=project_dir):
            return False
        
        self.print_success("Project build complete!")
        
        # Show executable location
        if sys.platform == 'win32':
            exe_path = project_dir / "build" / "bin" / f"{project_dir.name}.exe"
        else:
            exe_path = project_dir / "build" / "bin" / project_dir.name
        
        if exe_path.exists():
            self.print_info(f"Executable: {exe_path}")
        
        return True
    
    def cmd_project_run(self, args):
        """Run current project"""
        project_dir = Path.cwd()
        project_name = project_dir.name
        
        # Try bin directory first (new structure)
        if sys.platform == 'win32':
            exe_path = project_dir / "build" / "bin" / f"{project_name}.exe"
        else:
            exe_path = project_dir / "build" / "bin" / project_name
        
        # Fallback to old structure
        if not exe_path.exists():
            if sys.platform == 'win32':
                exe_path = project_dir / "build" / args.config / f"{project_name}.exe"
            else:
                exe_path = project_dir / "build" / args.config / project_name
        
        if not exe_path.exists():
            self.print_error(f"Executable not found: {exe_path}")
            self.print_info("Build the project first: sage project build")
            return False
        
        self.print_info(f"Running: {project_name}")
        print(f"Location: {exe_path}")
        print()
        
        try:
            # Run in the build/bin directory so assets are found
            subprocess.run([str(exe_path)], cwd=exe_path.parent)
            return True
        except KeyboardInterrupt:
            self.print_info("\nGame stopped by user")
            return True
        except Exception as e:
            self.print_error(f"Failed to run project: {e}")
            return False
    
    def cmd_project_clean(self, args):
        """Clean project build"""
        project_dir = Path.cwd()
        build_dir = project_dir / "build"
        
        if not build_dir.exists():
            self.print_warning("Build directory doesn't exist")
            return True
        
        self.print_info("Cleaning project...")
        shutil.rmtree(build_dir)
        self.print_success("Project cleaned")
        return True
    
    def cmd_list_projects(self, args):
        """List all SAGE projects"""
        self.print_header("SAGE Projects")
        
        projects_root = Path(self.config['projects_directory'])
        
        if not projects_root.exists():
            self.print_warning("No projects directory found")
            return True
        
        projects = [d for d in projects_root.iterdir() 
                   if d.is_dir() and (d / "CMakeLists.txt").exists()]
        
        if not projects:
            self.print_info("No projects found")
            return True
        
        for project in sorted(projects):
            print(f"\n{Colors.BOLD}{project.name}{Colors.ENDC}")
            print(f"  Location: {project}")
            
            # Check build status
            if (project / "build").exists():
                self.print_success("  Built")
            else:
                self.print_warning("  Not built")
        
        print()
        return True

def main():
    """Main entry point"""
    cli = SAGECLI()
    
    parser = argparse.ArgumentParser(
        description='SAGE Engine CLI - Command Line Interface',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  sage install              Install SAGE Engine
  sage build                Build engine (Release)
  sage build --config Debug Build engine (Debug)
  sage test                 Run all tests
  sage create MyGame        Create new project
  sage project build        Build current project
  sage project run          Run current project
        """
    )
    
    parser.add_argument('--version', action='version', 
                       version=f'SAGE CLI {cli.VERSION}')
    
    subparsers = parser.add_subparsers(dest='command', help='Commands')
    
    # Engine commands
    install_parser = subparsers.add_parser('install', help='Install SAGE Engine')
    install_parser.add_argument('--skip-build', action='store_true',
                               help='Skip building after installation')
    install_parser.set_defaults(config='Release')
    
    build_parser = subparsers.add_parser('build', help='Build SAGE Engine')
    build_parser.add_argument('--config', choices=['Debug', 'Release'], 
                             default='Release', help='Build configuration')
    build_parser.add_argument('--parallel', type=int, help='Parallel build jobs')
    
    test_parser = subparsers.add_parser('test', help='Run engine tests')
    test_parser.add_argument('--config', choices=['Debug', 'Release'],
                            default='Release', help='Test configuration')
    test_parser.add_argument('--filter', help='Test filter pattern')
    test_parser.add_argument('--verbose', action='store_true',
                            help='Verbose output')
    
    clean_parser = subparsers.add_parser('clean', help='Clean build artifacts')
    clean_parser.add_argument('--deep', action='store_true',
                             help='Remove entire build directory')
    
    info_parser = subparsers.add_parser('info', help='Show engine information')
    
    # Project commands
    create_parser = subparsers.add_parser('create', help='Create new project')
    create_parser.add_argument('name', help='Project name')
    
    list_parser = subparsers.add_parser('list', help='List all projects')
    
    project_parser = subparsers.add_parser('project', help='Project commands')
    project_subparsers = project_parser.add_subparsers(dest='project_command')
    
    proj_build = project_subparsers.add_parser('build', help='Build project')
    proj_build.add_argument('--config', choices=['Debug', 'Release'],
                           default='Release', help='Build configuration')
    proj_build.add_argument('--reconfigure', action='store_true',
                           help='Reconfigure CMake')
    proj_build.add_argument('--parallel', type=int, help='Parallel build jobs')
    
    proj_run = project_subparsers.add_parser('run', help='Run project')
    proj_run.add_argument('--config', choices=['Debug', 'Release'],
                         default='Release', help='Run configuration')
    
    proj_clean = project_subparsers.add_parser('clean', help='Clean project')
    
    args = parser.parse_args()
    
    # Execute command
    if args.command == 'install':
        success = cli.cmd_install(args)
    elif args.command == 'build':
        success = cli.cmd_build(args)
    elif args.command == 'test':
        success = cli.cmd_test(args)
    elif args.command == 'clean':
        success = cli.cmd_clean(args)
    elif args.command == 'info':
        success = cli.cmd_info(args)
    elif args.command == 'create':
        success = cli.cmd_create(args)
    elif args.command == 'list':
        success = cli.cmd_list_projects(args)
    elif args.command == 'project':
        if args.project_command == 'build':
            success = cli.cmd_project_build(args)
        elif args.project_command == 'run':
            success = cli.cmd_project_run(args)
        elif args.project_command == 'clean':
            success = cli.cmd_project_clean(args)
        else:
            parser.print_help()
            success = False
    else:
        parser.print_help()
        success = False
    
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
