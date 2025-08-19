
#!/usr/bin/env python3
"""
Update docs/plugins/registry.json "sig" for each zip in docs/releases using ed25519 seed.

Requires:
  - env PLUGIN_SIGN_SEED_B64 = base64 of 32-byte ed25519 seed (NOT a PEM; raw seed in base64)
  - PyNaCl installed (`pip install pynacl`)

Usage:
  python scripts/sign_registry.py
"""
import os, json, base64, pathlib, sys, hashlib

def sha256sum(p: pathlib.Path) -> str:
    h = hashlib.sha256()
    with open(p, "rb") as f:
        for chunk in iter(lambda: f.read(1024*1024), b""):
            h.update(chunk)
    return h.hexdigest()

def main():
    try:
        from nacl.signing import SigningKey
    except Exception:
        print("PyNaCl not installed; cannot sign. Skipping.", file=sys.stderr)
        sys.exit(0)

    seed_b64 = os.getenv("PLUGIN_SIGN_SEED_B64", "").strip()
    if not seed_b64:
        print("PLUGIN_SIGN_SEED_B64 not set. Skipping.", file=sys.stderr)
        sys.exit(0)

    seed = base64.b64decode(seed_b64)
    sk = SigningKey(seed)
    pk_b64 = base64.b64encode(bytes(sk.verify_key)).decode()

    root = pathlib.Path(__file__).resolve().parent.parent
    releases = root / "docs" / "releases"
    registry_path = root / "docs" / "plugins" / "registry.json"
    if not registry_path.exists():
        print("registry.json not found; nothing to sign.", file=sys.stderr)
        sys.exit(0)
    reg = json.loads(registry_path.read_text(encoding="utf-8"))

    files = {p.name: p for p in (releases.glob("*.zip") if releases.exists() else [])}

    changed = False
    for entry in reg:
        url = entry.get("url", "")
        if not url.endswith(".zip"):
            continue
        name = url.split("/")[-1]
        if name not in files:
            continue
        data = files[name].read_bytes()
        sig = sk.sign(data).signature
        entry["sig"] = base64.b64encode(sig).decode()
        entry["sha256"] = sha256sum(files[name])
        changed = True

    if changed:
        registry_path.write_text(json.dumps(reg, indent=2), encoding="utf-8")
        pubout = root / "docs" / "keys" / "public_ed25519.pub"
        pubout.parent.mkdir(parents=True, exist_ok=True)
        pubout.write_text(pk_b64 + "\n", encoding="utf-8")
        print("registry.json signed and public key written.")
    else:
        print("Nothing signed.")

if __name__ == "__main__":
    main()
