
from flask import Flask, jsonify, request, render_template_string, send_file, Response
import json, os, subprocess, threading, sys, shutil, tempfile, zipfile, time, hashlib, base64
from pathlib import Path

APP = Flask(__name__, static_folder=None)
ROOT = Path(os.getcwd())
REGISTRY = ROOT / "docs" / "plugins" / "registry.json"
PLUGINS_DIR = ROOT / "plugins"
STATE_DIR = ROOT / ".sage_state"
STATE_DIR.mkdir(exist_ok=True)
ENABLED_FILE = ROOT / "docs" / "plugins" / "enabled_plugins.json"
LOG_FILE = STATE_DIR / "plugin_ui.log"
STATUS_FILE = STATE_DIR / "status.json"
PUBKEY_FILE = ROOT / "docs" / "keys" / "public_ed25519.pub"

def _write_status(**kwargs):
    data = {"task": None, "progress": 0, "message": "", "ok": None}
    if STATUS_FILE.exists():
        try: data.update(json.loads(STATUS_FILE.read_text(encoding="utf-8")))
        except Exception: pass
    data.update(kwargs)
    STATUS_FILE.write_text(json.dumps(data, indent=2), encoding="utf-8")

def log(msg):
    ts = time.strftime("%Y-%m-%d %H:%M:%S")
    line = f"[{ts}] {msg}\n"
    with open(LOG_FILE, "a", encoding="utf-8") as f:
        f.write(line)
    _write_status(message=msg)

def load_registry():
    if REGISTRY.exists():
        return json.loads(REGISTRY.read_text(encoding="utf-8"))
    return []

def save_enabled(ids):
    ENABLED_FILE.parent.mkdir(parents=True, exist_ok=True)
    ENABLED_FILE.write_text(json.dumps(sorted(set(ids)), indent=2), encoding="utf-8")

def load_enabled():
    if ENABLED_FILE.exists():
        try:
            return set(json.loads(ENABLED_FILE.read_text(encoding="utf-8")))
        except Exception:
            return set()
    return set()

def _verify_sha256(file_path: Path, expected: str):
    if not expected: return True
    h = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(1024*1024), b""):
            h.update(chunk)
    ok = h.hexdigest().lower() == expected.lower()
    if not ok:
        log(f"sha256 mismatch for {file_path.name}")
    return ok

def _verify_signature(file_path: Path, sig_b64: str):
    # Optional ed25519 signature verification using PyNaCl and public key file
    if not sig_b64 or not PUBKEY_FILE.exists():
        return True, "signature skipped"
    try:
        from nacl.signing import VerifyKey
        from nacl.encoding import Base64Encoder, RawEncoder
    except Exception:
        log("PyNaCl not installed; skipping signature verify")
        return True, "pynacl missing"
    try:
        pub_bytes = base64.b64decode(PUBKEY_FILE.read_text(encoding="utf-8").strip())
        vk = VerifyKey(pub_bytes)
        sig = base64.b64decode(sig_b64)
        payload = file_path.read_bytes()
        vk.verify(payload, sig)
        return True, "signature ok"
    except Exception as e:
        log(f"signature verify failed: {e}")
        return False, str(e)

def _sandbox_env():
    safe_env = {k:v for k,v in os.environ.items() if k in ("SYSTEMROOT","WINDIR","HOME","USERPROFILE")}
    safe_env["PYTHONUNBUFFERED"]="1"
    safe_env["PATH"]=os.path.dirname(sys.executable)
    return safe_env

def _ensure_venv():
    # Create .sage_env and return python/pip paths
    import sys as _sys
    venv_dir = ROOT / ".sage_env"
    if not venv_dir.exists():
        import venv as _venv
        _venv.EnvBuilder(with_pip=True, clear=False).create(venv_dir)
    if os.name == "nt":
        py = venv_dir / "Scripts" / "python.exe"
        pip = venv_dir / "Scripts" / "pip.exe"
    else:
        py = venv_dir / "bin" / "python"
        pip = venv_dir / "bin" / "pip"
    return str(py), str(pip)

def _pip_install_pkg(pkg: str):
    py, pip = _ensure_venv()
    log(f"venv pip install {pkg}")
    subprocess.check_call([pip, "install", pkg], env=_sandbox_env())

def _download_to_tmp(url: str) -> Path:
    tmpdir = Path(tempfile.mkdtemp(prefix="sage_plugin_"))
    zpath = tmpdir / "plugin.zip"
    import urllib.request
    log(f"downloading {url}")
    urllib.request.urlretrieve(url, str(zpath))
    return zpath

def _extract_plugin_zip(zpath: Path) -> Path:
    with zipfile.ZipFile(zpath, "r") as z:
        z.extractall(zpath.parent)
    candidates = [p for p in zpath.parent.iterdir() if p.is_dir() and p.name!="__pycache__"]
    return candidates[0] if candidates else zpath.parent

def _installed_version(pid: str):
    pjson = PLUGINS_DIR / pid / "plugin.json"
    if pjson.exists():
        try:
            return json.loads(pjson.read_text(encoding="utf-8")).get("version")
        except Exception:
            return None
    return None

def install_plugin(entry):
    _write_status(task=f"install:{entry.get('id')}", progress=1, ok=None)
    pip_name = entry.get("pip")
    url = entry.get("url")
    if pip_name:
        try:
            _pip_install_pkg(pip_name)
            _write_status(progress=100, ok=True)
            return True, f"Installed via pip: {pip_name}"
        except Exception as e:
            log(f"pip install failed: {e}")
    if url:
        zpath = _download_to_tmp(url)
        if entry.get("sha256") and not _verify_sha256(zpath, entry["sha256"]):
            _write_status(progress=100, ok=False, message="SHA256 mismatch"); return False, "SHA256 mismatch"
        ok_sig, msg = _verify_signature(zpath, entry.get("sig",""))
        if not ok_sig:
            _write_status(progress=100, ok=False, message="Signature invalid"); return False, "Signature invalid"
        srcdir = _extract_plugin_zip(zpath)
        dst = PLUGINS_DIR / srcdir.name
        if dst.exists():
            shutil.rmtree(dst)
        shutil.move(str(srcdir), dst)
        _write_status(progress=100, ok=True)
        return True, f"Installed to {dst}"
    _write_status(progress=100, ok=False)
    return False, "No installation method"

def uninstall_plugin(pid: str):
    dst = PLUGINS_DIR / pid
    if dst.exists():
        shutil.rmtree(dst); return True
    return False

@APP.route("/")
def index():
    return render_template_string("""
<!doctype html>
<html><head>
<meta charset="utf-8"><title>SAGE Plugin Manager</title>
<style>
  :root{--bg:#0b0d10;--card:#151920;--text:#e8eef4;--muted:#a5b2c2;--accent:#5b9cff;--border:#1f2430}
  html,body{background:var(--bg);color:var(--text);font:14px/1.5 system-ui,Segoe UI,Arial;margin:0}
  .wrap{max-width:1024px;margin:32px auto;padding:0 16px}
  h1{margin:0 0 16px}
  .grid{display:grid;grid-template-columns:1fr;gap:16px}
  .card{background:var(--card);border:1px solid var(--border);border-radius:14px;padding:16px}
  .row{display:flex;align-items:center;justify-content:space-between;gap:12px;margin:8px 0}
  .btn{padding:6px 10px;border-radius:8px;border:1px solid var(--border);background:#0f131a;color:var(--text);cursor:pointer}
  .btn.primary{background:var(--accent);border-color:var(--accent);color:#08131f}
  .btn.danger{background:#ff5b67;border-color:#ff5b67;color:#2a0b0d}
  .muted{color:var(--muted);font-size:.9em}
  .progress{height:10px;background:#0e1218;border-radius:6px;overflow:hidden}
  .bar{height:10px;background:var(--accent);width:0%}
  .tag{font-size:.75em;padding:2px 6px;border:1px solid var(--border);border-radius:6px;color:var(--muted)}
  code{background:#0e1218;padding:2px 4px;border-radius:6px;color:#c6d7f2}
  .toast{position:fixed;right:16px;bottom:16px;max-width:300px;background:var(--card);border:1px solid var(--border);padding:12px;border-radius:10px;display:none}
</style>
</head>
<body>
<div class="wrap">
  <h1>SAGE Plugin Manager</h1>
  <div class="grid">
    <div class="card" id="registry"></div>
    <div class="card" id="local"></div>
    <div class="card">
      <div class="row"><b>Status</b>
        <span class="muted"><a href="#" onclick="refresh();return false;" style="color:var(--muted)">refresh</a></span></div>
      <div class="progress"><div class="bar" id="bar"></div></div>
      <pre id="status" class="muted" style="white-space:pre-wrap"></pre>
      <details><summary class="muted">Logs</summary><pre id="logs" style="white-space:pre-wrap"></pre></details>
    </div>
  </div>
</div>
<div class="toast" id="toast"></div>
<script>
function toast(msg){ const t=document.getElementById('toast'); t.textContent=msg; t.style.display='block'; setTimeout(()=>t.style.display='none',1800); }
async function refresh(){
  const [reg,loc,st] = await Promise.all([
    fetch('/api/registry').then(r=>r.json()),
    fetch('/api/local').then(r=>r.json()),
    fetch('/api/status').then(r=>r.json())
  ]);
  let rhtml = '<div class="row"><b>Registry</b><button class="btn" onclick="updateAll()">Update all</button></div>';
  for(const p of reg){
    let upd = p.has_update ? '<span class="tag">update available</span>' : '';
    rhtml += `<div class="row"><div><b>${p.id}</b> <span class="muted">v${p.version}</span> ${upd} <span class="muted">${p.description||''}</span></div>
              <div>
                <button class="btn primary" onclick="install('${p.id}')">Install/Update</button>
                <button class="btn" onclick="enable('${p.id}')">Enable</button>
                <button class="btn" onclick="disable('${p.id}')">Disable</button>
              </div></div>`;
  }
  document.getElementById('registry').innerHTML = rhtml;
  let lhtml = '<div class="row"><b>Local plugins</b><span class="muted">Installed in <code>plugins/</code></span></div><ul>';
  for(const x of loc.local){ lhtml += `<li>${x} <button class="btn danger" onclick="uninstall('${x}')">Uninstall</button></li>`; }
  lhtml += '</ul>'; document.getElementById('local').innerHTML = lhtml;
  document.getElementById('status').textContent = JSON.stringify(st,null,2);
  document.getElementById('bar').style.width = (st.progress||0)+'%';
}
async function install(id){ if(!confirm('Install or update '+id+'?')) return;
  await fetch('/api/install',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})}); toast('Started install '+id);
}
async function uninstall(id){ if(!confirm('Uninstall '+id+'?')) return;
  await fetch('/api/uninstall',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})}); toast('Uninstalled '+id); refresh();
}
async function enable(id){ await fetch('/api/enable',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})}); toast('Enabled '+id); refresh(); }
async function disable(id){ await fetch('/api/disable',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({id})}); toast('Disabled '+id); refresh(); }
async function updateAll(){ await fetch('/api/update_all',{method:'POST'}); toast('Update all started'); }
const ev = new EventSource('/events'); ev.onmessage = (e)=>{
  try{ const j = JSON.parse(e.data); document.getElementById('status').textContent = JSON.stringify(j,null,2); document.getElementById('bar').style.width=(j.progress||0)+'%'; }
  catch{} };
refresh();
</script>
</body></html>
""")

@APP.route("/events")
def events():
    def stream():
        last = None
        while True:
            try:
                if STATUS_FILE.exists():
                    data = STATUS_FILE.read_text(encoding="utf-8")
                    if data != last:
                        last = data
                        yield f"data: {data}\n\n"
                time.sleep(1.0)
            except GeneratorExit:
                break
            except Exception:
                time.sleep(1.0)
    return Response(stream(), mimetype="text/event-stream")

@APP.route("/api/registry")
def api_registry():
    reg = load_registry()
    out = []
    for e in reg:
        pid = e.get("id")
        cur = _installed_version(pid)
        e2 = dict(e)
        e2["installed_version"] = cur
        try:
            from packaging.version import Version
            e2["has_update"] = cur is not None and Version(str(e["version"])) > Version(str(cur))
        except Exception:
            e2["has_update"] = (cur != e["version"] and cur is not None)
        out.append(e2)
    return jsonify(out)

@APP.route("/api/local")
def api_local():
    PLUGINS_DIR.mkdir(exist_ok=True)
    items = [p.name for p in PLUGINS_DIR.iterdir() if p.is_dir()]
    return jsonify({"local": sorted(items)})

@APP.route("/api/install", methods=["POST"])
def api_install():
    data = request.get_json() or {}
    pid = data.get("id")
    entry = next((e for e in load_registry() if e.get("id")==pid), None)
    if not entry:
        return jsonify({"ok": False, "error": "not found"}), 404
    def worker():
        try:
            _write_status(task=f"install:{pid}", progress=5, ok=None)
            ok, msg = install_plugin(entry)
            log(f"install result: {ok} {msg}")
        except Exception as ex:
            log(f"install crash: {ex}"); _write_status(ok=False, message=str(ex), progress=100)
    threading.Thread(target=worker, daemon=True).start()
    return jsonify({"ok": True})

@APP.route("/api/uninstall", methods=["POST"])
def api_uninstall():
    data = request.get_json() or {}
    pid = data.get("id")
    ok = uninstall_plugin(pid)
    if ok: log(f"uninstalled {pid}")
    return jsonify({"ok": ok})

@APP.route("/api/enable", methods=["POST"])
def api_enable():
    data = request.get_json() or {}
    pid = data.get("id")
    enabled = load_enabled(); enabled.add(pid); save_enabled(list(enabled)); log(f"enabled: {pid}")
    return jsonify({"ok": True})

@APP.route("/api/disable", methods=["POST"])
def api_disable():
    data = request.get_json() or {}
    pid = data.get("id")
    enabled = load_enabled(); enabled.discard(pid); save_enabled(list(enabled)); log(f"disabled: {pid}")
    return jsonify({"ok": True})

@APP.route("/api/status")
def api_status():
    if STATUS_FILE.exists():
        try: return jsonify(json.loads(STATUS_FILE.read_text(encoding="utf-8")))
        except Exception: pass
    return jsonify({"task":None,"progress":0,"message":"","ok":None})

@APP.route("/api/logs")
def api_logs():
    if LOG_FILE.exists(): return send_file(str(LOG_FILE))
    return ("", 204)

@APP.route("/api/updates")
def api_updates():
    reg = json.loads(api_registry().get_data(as_text=True))
    to_update = [e["id"] for e in reg if e.get("has_update")]
    missing = [e["id"] for e in reg if e.get("installed_version") is None]
    return jsonify({"updates": to_update, "missing": missing})

@APP.route("/api/update_all", methods=["POST"])
def api_update_all():
    reg = load_registry()
    def worker():
        for e in reg:
            try:
                cur = _installed_version(e["id"])
                if cur is None or e.get("version") != cur:
                    _write_status(task=f"install:{e['id']}", progress=3, ok=None)
                    ok, msg = install_plugin(e)
                    log(f"update_all {e['id']}: {ok} {msg}")
            except Exception as ex:
                log(f"update_all crash for {e['id']}: {ex}")
        _write_status(task="idle", progress=100, ok=True, message="update_all finished")
    threading.Thread(target=worker, daemon=True).start()
    return jsonify({"ok": True})

def main():
    APP.run(host="127.0.0.1", port=5001, debug=False)

if __name__ == "__main__":
    main()


@APP.route("/password")
def page_password():
    return render_template_string("""
<!doctype html>
<html><head><meta charset="utf-8"><title>Password Generator</title>
<style>body{font-family:system-ui;max-width:720px;margin:24px auto}</style></head>
<body>
<h2>Password Generator</h2>
<label>Length <input id="len" type="number" value="16" min="6" max="128"></label><br>
<label>Sets: <label><input type="checkbox" class="set" value="upper" checked> upper</label>
<label><input type="checkbox" class="set" value="lower" checked> lower</label>
<label><input type="checkbox" class="set" value="digits" checked> digits</label>
<label><input type="checkbox" class="set" value="symbols"> symbols</label></label><br>
<label><input id="amb" type="checkbox" checked> avoid ambiguous</label>
<label><input id="ens" type="checkbox" checked> ensure each set</label><br>
<label>Custom: <input id="custom" type="text" placeholder="extra chars"></label><br>
<button onclick="gen()">Generate</button>
<pre id="out"></pre>
<script>
async function gen(){
  const len = parseInt(document.getElementById('len').value||'16');
  const sets = Array.from(document.querySelectorAll('.set:checked')).map(e=>e.value);
  const amb = document.getElementById('amb').checked;
  const ens = document.getElementById('ens').checked;
  const custom = document.getElementById('custom').value;
  let r = await fetch('/api/password', {method:'POST', headers:{'Content-Type':'application/json'},
    body: JSON.stringify({length:len, sets:sets, avoid_ambiguous:amb, ensure_each:ens, custom:custom})});
  let j = await r.json();
  document.getElementById('out').textContent = j.password;
}
</script>
</body></html>
""")

@APP.route("/api/password", methods=["POST"])
def api_password():
    data = request.get_json() or {}
    try:
        from plugins.passwordgen.generator import generate
        pwd = generate(**data)
        return jsonify({"ok": True, "password": pwd})
    except Exception as e:
        return jsonify({"ok": False, "error": str(e)}), 400
