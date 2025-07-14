from __future__ import annotations



class ModelingMixin:
    """Helper methods for mesh editing."""

    def set_selection_mode(self, mode: str) -> None:
        self.selection_mode = mode
        self.selected_vertices.clear()
        self.selected_edges.clear()
        self.selected_face = False
        if mode == "vertex":
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(True)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(False)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(False)
        elif mode == "edge":
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(False)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(True)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(False)
        else:
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(False)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(False)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(True)

    def extrude_selection(self) -> None:
        if self.selection_mode != "vertex":
            return
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        mesh = obj.mesh
        new_selection: set[int] = set()
        for idx in sorted(self.selected_vertices, reverse=True):
            if 0 <= idx < len(mesh.vertices):
                vx, vy = mesh.vertices[idx]
                mesh.vertices.insert(idx + 1, (vx, vy))
                self.selected_vertices = {
                    i + 1 if i > idx else i for i in self.selected_vertices
                }
                new_selection.add(idx + 1)
        if new_selection:
            self.selected_vertices = new_selection
        self.draw_scene(update_list=False)

    def new_face_from_edge(self) -> None:
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        verts = obj.mesh.vertices
        if self.selected_edges:
            idx = next(iter(self.selected_edges))
        elif (
            len(self.selected_vertices) == 2
            and all(0 <= i < len(verts) for i in self.selected_vertices)
        ):
            a, b = sorted(self.selected_vertices)
            if b == a + 1 or (a == len(verts) - 1 and b == 0):
                idx = a
            else:
                return
        else:
            return
        from engine.mesh_utils import create_polygon_mesh
        nx, ny = self._edge_normal(verts, idx)
        off = 0.5
        a = verts[idx]
        b = verts[(idx + 1) % len(verts)]
        new_b = (b[0] + nx * off, b[1] + ny * off)
        new_a = (a[0] + nx * off, a[1] + ny * off)
        new_verts = verts[: idx + 1] + [new_b, new_a] + verts[idx + 1 :]
        poly = create_polygon_mesh(new_verts)
        obj.mesh.vertices = poly.vertices
        obj.mesh.indices = poly.indices
        self.selected_edges = {idx + 1}
        self.selected_vertices = {idx + 1, idx + 2}
        self.draw_scene(update_list=False)

    def loop_cut(self) -> None:
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        from engine.mesh_utils import create_polygon_mesh
        verts = obj.mesh.vertices
        new_verts: list[tuple[float, float]] = []
        for i, v in enumerate(verts):
            nxt = verts[(i + 1) % len(verts)]
            new_verts.append(v)
            mid = ((v[0] + nxt[0]) / 2, (v[1] + nxt[1]) / 2)
            new_verts.append(mid)
        poly = create_polygon_mesh(new_verts)
        obj.mesh.vertices = poly.vertices
        obj.mesh.indices = poly.indices
        self.draw_scene(update_list=False)

    def toggle_fill(self) -> None:
        obj = self.selected_obj
        if obj is None or not hasattr(obj, "filled"):
            return
        self.undo_stack.snapshot(self.scene)
        obj.filled = not obj.filled
        self.update_properties()
        self.draw_scene(update_list=False)

    def union_selected(self) -> None:
        objs = [o for o in self.selected_objs if getattr(o, "mesh", None) is not None]
        if len(objs) < 2:
            return
        from engine.mesh_utils import Mesh, union_meshes
        self.undo_stack.snapshot(self.scene)
        meshes = []
        for obj in objs:
            verts = [self.mesh_to_world(obj, vx, vy) for vx, vy in obj.mesh.vertices]
            meshes.append(Mesh(list(verts), list(obj.mesh.indices or [])))

        result = union_meshes(meshes)
        min_x = min(v[0] for v in result.vertices)
        max_x = max(v[0] for v in result.vertices)
        min_y = min(v[1] for v in result.vertices)
        max_y = max(v[1] for v in result.vertices)

        base = objs[0]
        base.x = (min_x + max_x) / 2
        base.y = (min_y + max_y) / 2
        base.width = max_x - min_x
        base.height = max_y - min_y
        base.angle = 0.0
        base.scale_x = base.scale_y = 1.0
        base.pivot_x = base.pivot_y = 0.5

        base.mesh = Mesh(
            [self.world_to_mesh(base, x, y) for x, y in result.vertices],
            result.indices,
        )

        for obj in objs[1:]:
            self.scene.remove_object(obj)

        self.selected_objs = [base]
        self.selected_obj = base
        self.update_object_list()
        self.draw_scene()

    def translate_selection(self, dx: float, dy: float) -> None:
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        verts = obj.mesh.vertices
        if self.selection_mode == "vertex":
            indices = set(self.selected_vertices)
        elif self.selection_mode == "edge":
            indices = set()
            for idx in self.selected_edges:
                indices.add(idx)
                indices.add((idx + 1) % len(verts))
        else:
            if not self.selected_face:
                return
            indices = set(range(len(verts)))
        for i in indices:
            vx, vy = verts[i]
            wx, wy = self.mesh_to_world(obj, vx, vy)
            wx += dx
            wy += dy
            if self.snap_to_grid:
                wx = self.snap_value(wx, self.move_step)
                wy = self.snap_value(wy, self.move_step)
            verts[i] = self.world_to_mesh(obj, wx, wy)
        self.draw_scene(update_list=False)
