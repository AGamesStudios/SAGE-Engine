
#!/usr/bin/env python3
import sys, subprocess

def run(cmd):
    print("RUN:", " ".join(cmd), flush=True)
    try:
        r = subprocess.run(cmd, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        print(r.stdout)
        return 0
    except subprocess.CalledProcessError as e:
        print(e.stdout)
        return e.returncode

def main():
    py = sys.executable
    base = [py, "main.py", "--max-frames", "60", "--profile", "auto", "--render", "auto"]
    codes = []
    codes.append(run(base + ["--backend", "headless"]))
    codes.append(run(base + ["--backend", "sdl2"]))
    codes.append(run(base + ["--backend", "glfw"]))
    if all(c != 0 for c in codes):
        sys.exit(1)

if __name__ == "__main__":
    main()
