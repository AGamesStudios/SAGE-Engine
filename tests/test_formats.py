import importlib
import sys
import engine

from engine.formats import load_sageaudio, save_sageaudio, load_sagemesh, save_sagemesh

engine_mesh_utils = importlib.import_module('engine.mesh_utils')
engine_mesh_utils = importlib.reload(engine_mesh_utils)
sys.modules['engine.mesh_utils'] = engine_mesh_utils
engine.mesh_utils = engine_mesh_utils
importlib.reload(importlib.import_module('engine.formats.sagemesh'))
create_square_mesh = engine_mesh_utils.create_square_mesh


def test_sageaudio(tmp_path):
    path = tmp_path / 'sound.sageaudio'
    data = {'file': 'jump.wav', 'volume': 0.5}
    save_sageaudio(data, path)
    loaded = load_sageaudio(path)
    assert loaded['file'] == 'jump.wav'
    assert loaded['volume'] == 0.5


def test_sagemesh(tmp_path):
    mesh = create_square_mesh()
    path = tmp_path / 'square.sagemesh'
    save_sagemesh(mesh, path)
    loaded = load_sagemesh(path)
    assert loaded.vertices == mesh.vertices
    assert loaded.indices == mesh.indices
