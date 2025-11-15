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
    
    if not sage_script.exists():
        print(f"ERROR: sage.py not found at {sage_script}")
        return False
    
    # Create batch file wrapper
    batch_content = f"""@echo off
python "{sage_script}" %*
"""
    
    # Find a suitable installation directory
    user_scripts = Path.home() / ".local" / "bin"
    user_scripts.mkdir(parents=True, exist_ok=True)
    
    batch_file = user_scripts / "sage.bat"
    
    print(f"Installing to: {batch_file}")
    
    try:
        batch_file.write_text(batch_content, encoding='utf-8')
    except Exception as e:
        print(f"ERROR: Failed to write batch file: {e}")
        return False
    
    # Check if directory is in PATH
    path_dirs = os.environ.get('PATH', '').split(os.pathsep)
    if str(user_scripts) not in path_dirs:
        print(f"\n⚠ WARNING: {user_scripts} is not in your PATH")
        print(f"\nTo add it permanently:")
        print(f'  1. Run: setx PATH "%PATH%;{user_scripts}"')
        print(f'  2. Restart your terminal')
        print(f"\nOr add temporarily for this session:")
        print(f'  $env:PATH += ";{user_scripts}"')
    else:
        print(f"\n✓ Installation complete!")
    
    print(f"\nUsage:")
    print(f"  sage --help")
    print(f"  sage install")
    print(f"  sage build")
    print(f"  sage create MyGame")
    
    return True

def install_unix():
    """Install SAGE CLI on Linux/macOS"""
    print("Installing SAGE CLI for Unix...")
    
    # Get script directory
    tools_dir = Path(__file__).parent.absolute()
    sage_script = tools_dir / "sage.py"
    
    if not sage_script.exists():
        print(f"ERROR: sage.py not found at {sage_script}")
        return False
    
    # Make script executable
    try:
        sage_script.chmod(0o755)
    except Exception as e:
        print(f"WARNING: Failed to make script executable: {e}")
    
    # Create symlink in user bin
    user_bin = Path.home() / ".local" / "bin"
    user_bin.mkdir(parents=True, exist_ok=True)
    
    sage_link = user_bin / "sage"
    
    if sage_link.exists() or sage_link.is_symlink():
        sage_link.unlink()
    
    print(f"Creating symlink: {sage_link} -> {sage_script}")
    
    try:
        sage_link.symlink_to(sage_script)
    except Exception as e:
        print(f"ERROR: Failed to create symlink: {e}")
        return False
    
    # Check if directory is in PATH
    path_dirs = os.environ.get('PATH', '').split(os.pathsep)
    if str(user_bin) not in path_dirs:
        print(f"\n⚠ WARNING: {user_bin} is not in your PATH")
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
    print(f"  sage create MyGame")
    
    return True

def main():
    """Main installer"""
    print("="*60)
    print("SAGE Engine CLI Installer")
    print("="*60)
    print()
    
    try:
        if sys.platform == 'win32':
            success = install_windows()
        else:
            success = install_unix()
        
        if not success:
            print("\n❌ Installation failed!")
            sys.exit(1)
    except Exception as e:
        print(f"\n❌ Installation error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
