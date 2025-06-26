"""OpenGL renderer using :class:`QOpenGLWidget`."""

from __future__ import annotations

import math
        # store a reference to the renderer once assigned
        self.renderer = None

    def __init__(self, width: int = 640, height: int = 480,
                 title: str = "SAGE 2D", widget: Optional[GLWidget] = None,
                 keep_aspect: bool = True,
                 background: tuple[int, int, int] = (0, 0, 0)):
        self.width = width
        self.height = height
        self.title = title
        self.widget = widget if widget is not None else self.create_widget()
        self.keep_aspect = bool(keep_aspect)
        self.background = tuple(background)



    def create_widget(self) -> GLWidget:
        """Return the :class:`GLWidget` used for rendering."""
        return GLWidget()



class GLWidget(QOpenGLWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.renderer: Optional['OpenGLRenderer'] = None

    def initializeGL(self):
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_TEXTURE_2D)
        if self.renderer:
            self.renderer.setup_view()

    def paintGL(self):
        if self.renderer:
            self.renderer.paint()

        self.textures = {}
        self._blank_texture = None
        self._icon_cache = {}
        self._hover_axis = None
        self._drag_axis = None
        self._cursor_pos = None
        self._gizmo_mode = 'move'
    """Renderer using PyOpenGL and QOpenGLWidget."""

    width: int = 640
        self._gizmo_mode: str = 'move'
        self._gizmo_mode: str = 'move'
    height: int = 480
    title: str = "SAGE 2D"
    widget: Optional[GLWidget] = None
    keep_aspect: bool = True
    background: tuple[int, int, int] = (0, 0, 0)

    def create_widget(self) -> GLWidget:
        """Return the :class:`GLWidget` used for rendering."""
        return GLWidget()

    def __post_init__(self):
        if self.widget is None:
            self.widget = self.create_widget()
        self.widget.renderer = self
        self.widget.resize(self.width, self.height)
        self._should_close = False
        self.textures: dict[int, int] = {}
        self._blank_texture: int | None = None
        self._icon_cache: dict[str, int] = {}
        self._scene = None
        self._camera = None
        self._draw_gizmos = True
        self._selected_obj = None
        self._hover_axis: str | None = None
        self._drag_axis: str | None = None
        self._cursor_pos: tuple[float, float] | None = None
        self.keep_aspect = bool(self.keep_aspect)
        self.background = tuple(self.background)

    def set_window_size(self, width: int, height: int):
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height
        # only update the GL projection if a valid context exists
        ctx = self.widget.context() if self.widget else None
        if ctx and ctx.isValid():
            self.setup_view()

    def should_close(self) -> bool:
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        glClearColor(color[0]/255.0, color[1]/255.0, color[2]/255.0, 1.0)
        glClear(GL_COLOR_BUFFER_BIT)

    def setup_view(self):
        from OpenGL.GL import glMatrixMode, glLoadIdentity, glOrtho, GL_PROJECTION, GL_MODELVIEW
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        # center the origin so camera transforms are stable
        glOrtho(
            -self.width / 2,
            self.width / 2,
            -self.height / 2,
            self.height / 2,
            -1,
            1,
        )
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def _get_blank_texture(self) -> int:
        """Return a 1x1 white texture used for colored objects."""
        if self._blank_texture is None:
            data = b"\xff\xff\xff\xff"
            self._blank_texture = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self._blank_texture)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, data
            )
        return self._blank_texture

    def _get_texture(self, obj) -> int:
        if obj.image is None:
            return self._get_blank_texture()
        tex = self.textures.get(id(obj.image))
        if tex:
            return tex
        img = obj.image
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        data = img.tobytes()
        tex_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex_id)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data
        )
        self.textures[id(obj.image)] = tex_id
        return tex_id

    def _get_icon_texture(self, name: str) -> int:
        """Load an icon image from ``sage_editor/icons`` and cache it."""
        tex = self._icon_cache.get(name)
        if tex:
            return tex
        path = Path(__file__).resolve().parent.parent.parent / 'sage_editor' / 'icons' / name
        if not path.is_file():
            return self._get_blank_texture()
        img = Image.open(path).convert('RGBA')
        img = img.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        data = img.tobytes()
        tex = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, tex)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, data
        )
        self._icon_cache[name] = tex
        return tex

    def _draw_icon(self, x: float, y: float, tex: int, zoom: float, size: float = 32.0):
        """Render a billboard icon at ``(x, y)`` in world units."""
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, tex)
        glColor4f(1.0, 1.0, 1.0, 1.0)
        glPushMatrix()
        glTranslatef(x * scale, y * scale * sign, 0)
        inv_zoom = 1.0 / zoom if zoom else 1.0
        half = size / 2.0 * inv_zoom
        glBegin(GL_QUADS)
        glTexCoord2f(0.0, 0.0); glVertex2f(-half, -half)
        glTexCoord2f(1.0, 0.0); glVertex2f( half, -half)
        glTexCoord2f(1.0, 1.0); glVertex2f( half,  half)
        glTexCoord2f(0.0, 1.0); glVertex2f(-half,  half)
        glEnd()
        glPopMatrix()

    def _draw_frustum(self, cam: Camera):
        """Draw a rectangle representing the camera's view."""
        left, bottom, w, h = cam.view_rect()
        sign = 1.0 if units.Y_UP else -1.0
        glBindTexture(GL_TEXTURE_2D, 0)
        glColor4f(1.0, 1.0, 0.0, 1.0)
        glBegin(GL_LINE_LOOP)
        glVertex2f(left, bottom * sign)
        glVertex2f(left + w, bottom * sign)
        glVertex2f(left + w, (bottom + h) * sign)
        glVertex2f(left, (bottom + h) * sign)
        glEnd()

    def _draw_origin(self, size: float = 1.0):
        glBindTexture(GL_TEXTURE_2D, 0)
        glBegin(GL_LINES)
        glColor4f(1.0, 0.0, 0.0, 1.0)
        glVertex2f(-size, 0.0)
        glVertex2f(size, 0.0)
                    mode: str = 'move',
        if mode == 'move':
            color_x = 1.0 if not (hover in ("x", "xy") or dragging in ("x", "xy")) else 0.5
            glColor4f(color_x, 0.0, 0.0, 1.0)
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(size, 0.0)
            glEnd()
            glBegin(GL_TRIANGLES)
            glVertex2f(size, 0.0)
            glVertex2f(size - head, head / 2)
            glVertex2f(size - head, -head / 2)
            glEnd()
        elif mode == 'scale':
            color_sx = 1.0 if not (hover == 'sx' or dragging == 'sx') else 0.5
            glColor4f(color_sx, 0.0, 0.0, 1.0)
            glBegin(GL_QUADS)
            glVertex2f(size - sq, -sq)
            glVertex2f(size + sq, -sq)
            glVertex2f(size + sq, sq)
            glVertex2f(size - sq, sq)
            glEnd()
        if mode == 'move':
            color_y = 1.0 if not (hover in ("y", "xy") or dragging in ("y", "xy")) else 0.5
            glColor4f(0.0, color_y, 0.0, 1.0)
            glBegin(GL_LINES)
            glVertex2f(0.0, 0.0)
            glVertex2f(0.0, size * sign)
            glEnd()
            glBegin(GL_TRIANGLES)
            if units.Y_UP:
                glVertex2f(0.0, size)
                glVertex2f(-head / 2, size - head)
                glVertex2f(head / 2, size - head)
            else:
                glVertex2f(0.0, -size)
                glVertex2f(-head / 2, -size + head)
                glVertex2f(head / 2, -size + head)
            glEnd()
        elif mode == 'scale':
            color_sy = 1.0 if not (hover == 'sy' or dragging == 'sy') else 0.5
            glColor4f(0.0, color_sy, 0.0, 1.0)
            glBegin(GL_QUADS)
            if units.Y_UP:
                glVertex2f(-sq, size - sq)
                glVertex2f(sq, size - sq)
                glVertex2f(sq, size + sq)
                glVertex2f(-sq, size + sq)
            else:
                glVertex2f(-sq, -size - sq)
                glVertex2f(sq, -size - sq)
                glVertex2f(sq, -size + sq)
                glVertex2f(-sq, -size + sq)
            glEnd()
        if mode == 'rotate':
            color_rot = 1.0 if not (hover == 'rot' or dragging == 'rot') else 0.5
            glColor4f(color_rot, color_rot, 0.0, 1.0)
            glLineWidth(ring_w)
            glBegin(GL_LINE_LOOP)
            for i in range(32):
                ang = (i / 32.0) * math.tau
                glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
            glEnd()
            glLineWidth(4)
            glBegin(GL_LINE_LOOP)
            for i in range(32):
                ang = (i / 32.0) * math.tau
                glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
            glEnd()
            glLineWidth(4)
        if obj is None:
            return
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        zoom = camera.zoom if camera else 1.0
        inv = 1.0 / zoom if zoom else 1.0
        size = 50 * inv
        head = 10 * inv
        rad = 4 * inv
        sq = 6 * inv
        ring_r = size * 1.2
        ring_w = 3 * inv
        glBindTexture(GL_TEXTURE_2D, 0)
        glPushMatrix()
        glTranslatef(obj.x * scale, obj.y * scale * sign, 0)
        glLineWidth(4)
        # X axis translation
        color_x = 1.0 if not (hover in ("x", "xy") or dragging in ("x", "xy")) else 0.5
        glColor4f(color_x, 0.0, 0.0, 1.0)
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(size, 0.0)
        glEnd()
        glBegin(GL_TRIANGLES)
        glVertex2f(size, 0.0)
        glVertex2f(size - head, head / 2)
        glVertex2f(size - head, -head / 2)
        glEnd()
        # X axis scale square
        color_sx = 1.0 if not (hover == 'sx' or dragging == 'sx') else 0.5
        glColor4f(color_sx, 0.0, 0.0, 1.0)
        glBegin(GL_QUADS)
        glVertex2f(size - sq, -sq)
        glVertex2f(size + sq, -sq)
        glVertex2f(size + sq, sq)
        glVertex2f(size - sq, sq)
        glEnd()

        # Y axis translation
        color_y = 1.0 if not (hover in ("y", "xy") or dragging in ("y", "xy")) else 0.5
        glColor4f(0.0, color_y, 0.0, 1.0)
        glBegin(GL_LINES)
        glVertex2f(0.0, 0.0)
        glVertex2f(0.0, size * sign)
        glEnd()
        glBegin(GL_TRIANGLES)
        if units.Y_UP:
            glVertex2f(0.0, size)
            glVertex2f(-head / 2, size - head)
            glVertex2f(head / 2, size - head)
        else:
            glVertex2f(0.0, -size)
            glVertex2f(-head / 2, -size + head)
            glVertex2f(head / 2, -size + head)
        glEnd()
        # Y axis scale square
        color_sy = 1.0 if not (hover == 'sy' or dragging == 'sy') else 0.5
        glColor4f(0.0, color_sy, 0.0, 1.0)
        glBegin(GL_QUADS)
        if units.Y_UP:
            glVertex2f(-sq, size - sq)
            glVertex2f(sq, size - sq)
            glVertex2f(sq, size + sq)
            glVertex2f(-sq, size + sq)
        else:
            glVertex2f(-sq, -size - sq)
            glVertex2f(sq, -size - sq)
            glVertex2f(sq, -size + sq)
                   cursor: tuple[float, float] | None = None,
                   mode: str = 'move'):
        self._gizmo_mode = mode
                self._draw_gizmo(
                    self._selected_obj, camera, self._gizmo_mode,
                    self._hover_axis, self._drag_axis
                )
                   cursor: tuple[float, float] | None = None,
                   mode: str = 'move'):
        self._gizmo_mode = mode
                self._draw_gizmo(
                    self._selected_obj, camera, self._gizmo_mode,
                    self._hover_axis, self._drag_axis
                )
        glLineWidth(ring_w)
        glBegin(GL_LINE_LOOP)
        for i in range(32):
            ang = (i / 32.0) * math.tau
            glVertex2f(math.cos(ang) * ring_r, math.sin(ang) * ring_r * sign)
        glEnd()
        glLineWidth(4)

        # pivot point
        center_col = 0.5 if not (hover == 'xy' or dragging == 'xy') else 0.25
        glColor4f(center_col, center_col, center_col, 1.0)
        glBegin(GL_QUADS)
        glVertex2f(-rad, -rad)
        glVertex2f(rad, -rad)
        glVertex2f(rad, rad)
        glVertex2f(-rad, rad)
        glEnd()
        glLineWidth(1)
        glPopMatrix()

    def _apply_viewport(self, camera: Camera | None) -> None:
        """Set GL viewport respecting the camera aspect ratio."""
        from OpenGL.GL import glViewport
        w = self.widget.width() if self.widget else self.width
        h = self.widget.height() if self.widget else self.height
        if not self.keep_aspect or camera is None:
            glViewport(0, 0, w, h)
            return
        cam_ratio = camera.width / camera.height if camera.height else 1.0
        win_ratio = w / h if h else cam_ratio
        if cam_ratio > win_ratio:
            vp_w = w
            vp_h = int(w / cam_ratio)
            x = 0
            y = int((h - vp_h) / 2)
        else:
            vp_h = h
            vp_w = int(h * cam_ratio)
            x = int((w - vp_w) / 2)
            y = 0
        glViewport(x, y, vp_w, vp_h)

    def _apply_projection(self, camera: Camera | None) -> None:
        from OpenGL.GL import glMatrixMode, glLoadIdentity, glOrtho, GL_PROJECTION, GL_MODELVIEW
        w = (camera.width if (self.keep_aspect and camera) else self.width)
        h = (camera.height if (self.keep_aspect and camera) else self.height)
        sign = 1.0 if units.Y_UP else -1.0
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(-w / 2, w / 2, -h / 2 * sign, h / 2 * sign, -1, 1)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()

    def paint(self):
        # called from GLWidget.paintGL
        self.clear(self.background)
        if self._scene:
            self._render_scene(self._scene, self._camera)

    def draw_scene(self, scene, camera: Camera | None = None, gizmos: bool = True,
                   selected: GameObject | None = None,
                   hover: str | None = None, dragging: str | None = None,
                   cursor: tuple[float, float] | None = None):
        """Store the scene and camera then schedule a repaint."""
        self._scene = scene
        self._camera = camera
        self._draw_gizmos = gizmos
        self._selected_obj = selected
        self._hover_axis = hover
        self._drag_axis = dragging
        self._cursor_pos = cursor
        self.widget.update()

    def _render_scene(self, scene, camera: Camera | None):
        self._apply_viewport(camera)
        self._apply_projection(camera)
        glPushMatrix()
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        if camera:
            glScalef(camera.zoom, camera.zoom, 1.0)
            glTranslatef(-camera.x * scale,
                         -camera.y * scale * sign,
                         0)
        scene.sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                if self._draw_gizmos:
                    self._draw_frustum(obj)
                    tex = self._get_icon_texture('camera.png')
                    self._draw_icon(
                        obj.x, obj.y, tex, camera.zoom if camera else 1.0
                    )
                continue
            tex = self._get_texture(obj)
            glBindTexture(GL_TEXTURE_2D, tex)
            glPushMatrix()
            glTranslatef(obj.x * scale, obj.y * scale, 0)
            glRotatef(obj.angle, 0, 0, 1)
            glScalef(obj.scale_x * scale, obj.scale_y * scale, 1)
            w = obj.width
            h = obj.height
            glColor4f(*(c/255.0 for c in (obj.color or (255, 255, 255, 255))))
            glBegin(GL_QUADS)
            glTexCoord2f(0.0, 0.0); glVertex2f(-w/2, -h/2)
            glTexCoord2f(1.0, 0.0); glVertex2f( w/2, -h/2)
            glTexCoord2f(1.0, 1.0); glVertex2f( w/2,  h/2)
            glTexCoord2f(0.0, 1.0); glVertex2f(-w/2,  h/2)
            glEnd()
            glPopMatrix()
            if self._draw_gizmos:
                tex_icon = self._get_icon_texture('object.png')
                self._draw_icon(
                    obj.x, obj.y, tex_icon, camera.zoom if camera else 1.0
                )
        if self._draw_gizmos:
            if self._selected_obj:
                self._draw_gizmo(self._selected_obj, camera,
                                 self._hover_axis, self._drag_axis)
            self._draw_origin(50 * scale)
            if self._cursor_pos is not None:
                self._draw_cursor(self._cursor_pos[0], self._cursor_pos[1],
                                   camera)
        glPopMatrix()

    def present(self):
        self.widget.update()

    def close(self):
        self._should_close = True
        if self.widget:
            self.widget.close()
