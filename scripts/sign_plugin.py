
#!/usr/bin/env python3
"""
Sign a plugin zip with ed25519 and write base64 signature to stdout.
Usage:
  python scripts/sign_plugin.py --key private_key_base64.txt --file docs/releases/sage3d_plugin_v0.1.0.zip
The private key file must contain base64 of a 32-byte ed25519 seed.
"""
import argparse, base64, sys, pathlib
try:
    from nacl.signing import SigningKey
except Exception:
    print("PyNaCl not installed. pip install pynacl", file=sys.stderr); sys.exit(1)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--key", required=True, help="path to base64 private key seed")
    ap.add_argument("--file", required=True, help="zip file to sign")
    args = ap.parse_args()
    seed = base64.b64decode(pathlib.Path(args.key).read_text().strip())
    sk = SigningKey(seed)
    data = pathlib.Path(args.file).read_bytes()
    sig = sk.sign(data).signature
    print(base64.b64encode(sig).decode())

if __name__ == "__main__":
    main()
