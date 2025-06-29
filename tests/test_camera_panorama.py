from engine.core.camera import Camera
from engine.core.objects import object_to_dict, object_from_dict


def test_camera_panorama_serialization(tmp_path):
    p = tmp_path / "sky.png"
    cam = Camera(panorama=str(p), pano_fx=0.01, pano_fy=0.02)
    data = object_to_dict(cam)
    assert data["panorama"] == str(p)
    cam2 = object_from_dict(data)
    assert cam2.panorama == str(p)
    assert cam2.pano_fx == 0.01
    assert cam2.pano_fy == 0.02
