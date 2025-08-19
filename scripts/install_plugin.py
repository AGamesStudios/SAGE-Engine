#!/usr/bin/env python3
\"\"\"Simple plugin installer for SAGE Engine.

Usage:
  python scripts/install_plugin.py --id sage2d
  python scripts/install_plugin.py --id sage2d --from pip
  python scripts/install_plugin.py --id sage2d --from url --url <zip_url>
  python scripts/install_plugin.py --list  # list registry
\"\"\"
import argparse, json, os, sys, shutil, tempfile, urllib.request, zipfile, subprocess
from pathlib import Path

ROOT = Path(os.getcwd())
PLUGINS_DIR = ROOT / "plugins"
REGISTRY_PATH = ROOT / "docs" / "plugins" / "registry.json"

def load_registry():
    if REGISTRY_PATH.exists():
        return json.loads(REGISTRY_PATH.read_text(encoding="utf-8"))
    return []

def find_entry(reg, pid):
    for e in reg:
        if e.get("id")==pid or e.get("name")==pid:
            return e
    return None

def install_from_pip(name):
    print("Installing from pip:", name)
    subprocess.check_call([sys.executable, "-m", "pip", "install", name])


import hashlib
def verify_file_sha256(path: str, expected_hex: str) -> bool:
    if not expected_hex:
        return True
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest().lower() == expected_hex.lower()

def install_from_url(url):
    print("Downloading plugin from:", url)
    PLUGINS_DIR.mkdir(exist_ok=True)
    tmp = tempfile.mkdtemp(prefix="sage_plugin_")
    try:
        zip_path = os.path.join(tmp, "plugin.zip")
        urllib.request.urlretrieve(url, zip_path)
        with zipfile.ZipFile(zip_path, "r") as z:
            z.extractall(tmp)
        # find top-level plugin folder
        entries = [p for p in os.listdir(tmp) if os.path.isdir(os.path.join(tmp, p)) and p!='__pycache__']
        if entries:
            src = os.path.join(tmp, entries[0])
            dst = PLUGINS_DIR / entries[0]
            if dst.exists():
                print("Plugin already exists, removing old version:", dst)
                shutil.rmtree(dst)
            shutil.move(src, dst)
            print("Installed plugin to", dst)
        else:
            # fallback: move all files into plugins/<name>
            dst = PLUGINS_DIR / "plugin_from_url"
            if dst.exists():
                shutil.rmtree(dst)
            shutil.move(tmp, dst)
            print("Installed plugin to", dst)
    finally:
        try: shutil.rmtree(tmp)
        except Exception: pass

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--id", help="plugin id to install")
    parser.add_argument("--from", dest="src", choices=["registry","pip","url"], default="registry")
    parser.add_argument("--url", help="URL to plugin zip (if --from url)")
    parser.add_argument("--list", action="store_true")
    args = parser.parse_args()

    reg = load_registry()
    if args.list:
        print("Available plugins from registry:")
        for e in reg:
            print(f\" - {e['id']}: {e.get('description','')}\")
        return

    if not args.id:
        parser.print_help()
        return

    entry = find_entry(reg, args.id)
    if args.src == "registry":
        if not entry:
            print("Plugin not found in registry:", args.id); sys.exit(1)
        if entry.get("pip"):
            install_from_pip(entry["pip"]); return
        if entry.get("url"):
            install_from_url(entry["url"]); return
        print("Registry entry has no pip/url for installation")
        return
    if args.src == "pip":
        if entry and entry.get("pip"):
            install_from_pip(entry["pip"]); return
        print("No pip name known for", args.id); sys.exit(1)
    if args.src == "url":
        url = args.url or (entry and entry.get("url"))
        if not url:
            print("No url provided"); sys.exit(1)
        install_from_url(url)

if __name__ == '__main__':
    main()
