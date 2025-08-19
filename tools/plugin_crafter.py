
#!/usr/bin/env python3
import argparse, json, os
from pathlib import Path

TEMPLATES = {
  "shader": {
    "plugin.json": {
      "id": "your.shader.sample",
      "name": "Sample Shader",
      "version": "0.1.0",
      "category": "shader",
      "languages": ["glsl"],
      "services": ["shader_factory"],
      "entry": "plugin.py"
    },
    "plugin.py": """
def register(api):
    def make_shader():
        return {"vertex":"shaders/basic.vert", "fragment":"shaders/basic.frag"}
    api.provide_service("shader_factory", make_shader)
"""
  },
  "image": {
    "plugin.json": {
      "id": "your.image.loader",
      "name": "Image Loader",
      "version": "0.1.0",
      "category": "image",
      "languages": ["python"],
      "services": ["image_loader"],
      "entry": "plugin.py"
    },
    "plugin.py": """
from PIL import Image
def register(api):
    def load_image(path):
        return Image.open(path).convert("RGBA")
    api.provide_service("image_loader", load_image)
"""
  },
  "input": {
    "plugin.json": {
      "id": "your.input.keyboard",
      "name": "Keyboard Input",
      "version": "0.1.0",
      "category": "input",
      "languages": ["python"],
      "services": ["input_poll"],
      "entry": "plugin.py"
    },
    "plugin.py": """
def register(api):
    def poll(window):
        # placeholder
        return {}
    api.provide_service("input_poll", poll)
"""
  }
}

def main():
    p = argparse.ArgumentParser("SAGE Plugin Crafter")
    p.add_argument("type", choices=list(TEMPLATES.keys()))
    p.add_argument("--out", required=True, help="Target directory (e.g. plugins/myshader)")
    p.add_argument("--id", help="Override plugin id")
    p.add_argument("--name", help="Override plugin name")
    args = p.parse_args()

    tpl = TEMPLATES[args.type]
    out = Path(args.out)
    (out / "shaders").mkdir(parents=True, exist_ok=True)
    for fn, content in tpl.items():
        if isinstance(content, dict):
            if args.id: content["id"] = args.id
            if args.name: content["name"] = args.name
            (out / fn).write_text(json.dumps(content, indent=2), encoding="utf-8")
        else:
            (out / fn).write_text(content, encoding="utf-8")
    # minimal shader files for shader template
    if args.type == "shader":
        (out / "shaders" / "basic.vert").write_text("""#version 330 core
layout(location=0) in vec3 aPos; void main(){ gl_Position=vec4(aPos,1.0); }""", encoding="utf-8")
        (out / "shaders" / "basic.frag").write_text("""#version 330 core
out vec4 FragColor; void main(){ FragColor=vec4(1.0); }""", encoding="utf-8")
    print("Created plugin scaffold at", out)

if __name__ == "__main__":
    main()
