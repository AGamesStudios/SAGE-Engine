from sage_engine import ui


def test_ui_instance_shape():
    ui.theme.set_theme(str(ui.theme._current_path))
    btn = ui.Button()
    inst = ui.collect_instances()
    if hasattr(inst, "shape"):
        assert inst.shape[1] == 16
        rgba = ui.theme.color_rgba(btn.bg_color)
        assert all(abs(inst[0, i+11] - rgba[i]) < 1e-6 for i in range(4))
    else:
        assert len(inst[0]) == 16
        rgba = ui.theme.color_rgba(btn.bg_color)
        assert all(abs(inst[0][11+i] - rgba[i]) < 1e-6 for i in range(4))

