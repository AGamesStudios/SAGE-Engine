#!/usr/bin/env python3
"""
SAGE CLI Installer
Installs sage command globally
"""

import os
import sys
import shutil
from pathlib import Path

def install_windows():
    """Install SAGE CLI on Windows"""
    print("Installing SAGE CLI for Windows...")
    
    # Get script directory
    tools_dir = Path(__file__).parent.absolute()
    sage_script = tools_dir / "sage.py"
    
    # Create batch file wrapper
    batch_content = f"""@echo off
python "{sage_script}" %*
"""
    
    # Find a suitable installation directory
    user_scripts = Path.home() / "AppData" / "Local" / "Programs" / "Python" / "Scripts"
    
    if not user_scripts.exists():
        # Try alternative locations
        user_scripts = Path.home() / ".local" / "bin"
        user_scripts.mkdir(parents=True, exist_ok=True)
    
    batch_file = user_scripts / "sage.bat"
    
    print(f"Installing to: {batch_file}")
    batch_file.write_text(batch_content)
    
    # Check if directory is in PATH
    path_dirs = os.environ.get('PATH', '').split(os.pathsep)
    if str(user_scripts) not in path_dirs:
        print(f"\nWARNING: {user_scripts} is not in your PATH")
        print(f"Add the following to your PATH:")
        print(f"  {user_scripts}")
        print(f"\nOr run this command:")
        print(f'  setx PATH "%PATH%;{user_scripts}"')
    else:
        print(f"\n✓ Installation complete!")
        print(f"\nUsage:")
        print(f"  sage --help")
        print(f"  sage install")
        print(f"  sage build")
        print(f"  sage test")
        print(f"  sage create MyGame")

def install_unix():
    """Install SAGE CLI on Linux/macOS"""
    print("Installing SAGE CLI for Unix...")
    
    # Get script directory
    tools_dir = Path(__file__).parent.absolute()
    sage_script = tools_dir / "sage.py"
    
    # Make script executable
    sage_script.chmod(0o755)
    
    # Create symlink in user bin
    user_bin = Path.home() / ".local" / "bin"
    user_bin.mkdir(parents=True, exist_ok=True)
    
    sage_link = user_bin / "sage"
    
    if sage_link.exists():
        sage_link.unlink()
    
    print(f"Creating symlink: {sage_link} -> {sage_script}")
    sage_link.symlink_to(sage_script)
    
    # Check if directory is in PATH
    path_dirs = os.environ.get('PATH', '').split(os.pathsep)
    if str(user_bin) not in path_dirs:
        print(f"\nWARNING: {user_bin} is not in your PATH")
        print(f"\nAdd the following to your ~/.bashrc or ~/.zshrc:")
        print(f'  export PATH="$HOME/.local/bin:$PATH"')
        print(f"\nThen run:")
        print(f"  source ~/.bashrc  # or ~/.zshrc")
    else:
        print(f"\n✓ Installation complete!")
        print(f"\nUsage:")
        print(f"  sage --help")
        print(f"  sage install")
        print(f"  sage build")
        print(f"  sage test")
        print(f"  sage create MyGame")

def main():
    """Main installer"""
    print("="*60)
    print("SAGE Engine CLI Installer")
    print("="*60)
    print()
    
    if sys.platform == 'win32':
        install_windows()
    else:
        install_unix()

if __name__ == '__main__':
    main()
