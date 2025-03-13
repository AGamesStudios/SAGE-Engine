import glfw
from OpenGL.GL import *
from OpenGL.GL.shaders import compileShader, compileProgram
import glm
import pywavefront
import numpy as np
import json
import time
import importlib.util
from PIL import Image
import ctypes
import os
import warnings
import pygame  # Используется для аудио

# Константы для анизотропной фильтрации
GL_TEXTURE_MAX_ANISOTROPY_EXT = 0x84FE
GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT = 0x84FF
MAX_ANISOTROPY = 1.0

warnings.filterwarnings("ignore", category=UserWarning, module="pywavefront")

# Инициализация аудио системы (pygame)
pygame.mixer.init()

# -----------------------------------------------------------
# КЛАСС ОКНА
# -----------------------------------------------------------
class Window:
    """Управление окном и контекстом OpenGL."""
    def __init__(self, width: int, height: int, title: str, hide_cursor: bool = True):
        if not glfw.init():
            raise Exception("Не удалось инициализировать GLFW")

        self.window = glfw.create_window(width, height, title, None, None)
        if not self.window:
            glfw.terminate()
            raise Exception("Не удалось создать окно")

        glfw.make_context_current(self.window)
        glfw.set_window_size_callback(self.window, self._resize_callback)

        self.width = width
        self.height = height
        self._update_projection()

        if hide_cursor:
            glfw.set_input_mode(self.window, glfw.CURSOR, glfw.CURSOR_DISABLED)

        # Настройка OpenGL
        glClearColor(0.1, 0.1, 0.1, 1.0)
        glEnable(GL_DEPTH_TEST)
        glEnable(GL_CULL_FACE)
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)

        # Проверка поддержки анизотропной фильтрации
        global MAX_ANISOTROPY
        extensions = glGetString(GL_EXTENSIONS).decode().split()
        if 'GL_EXT_texture_filter_anisotropic' in extensions:
            MAX_ANISOTROPY = glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
            print(f"[INFO] Анизотропная фильтрация поддерживается, макс. уровень: {MAX_ANISOTROPY}")
        else:
            print("[WARN] Анизотропная фильтрация не поддерживается")

    def _resize_callback(self, window, width, height):
        glViewport(0, 0, width, height)
        self.width, self.height = width, height
        self._update_projection()

    def _update_projection(self):
        # Проекция 45° (FOV), соотношение сторон = width/height
        self.projection = glm.perspective(glm.radians(45.0), self.width / self.height, 0.1, 100.0)

    def poll_events(self):
        glfw.poll_events()

    def should_close(self) -> bool:
        return glfw.window_should_close(self.window)

    def swap_buffers(self):
        glfw.swap_buffers(self.window)

    def terminate(self):
        glfw.terminate()

# -----------------------------------------------------------
# КЛАСС ШЕЙДЕРА
# -----------------------------------------------------------
class Shader:
    """Управление шейдерной программой."""
    def __init__(self, vertex_src: str, fragment_src: str):
        self.program = compileProgram(
            compileShader(vertex_src, GL_VERTEX_SHADER),
            compileShader(fragment_src, GL_FRAGMENT_SHADER)
        )
        # Получаем локации uniform-переменных
        self.uniforms = {
            "model": glGetUniformLocation(self.program, "model"),
            "view": glGetUniformLocation(self.program, "view"),
            "projection": glGetUniformLocation(self.program, "projection"),
            "normalMatrix": glGetUniformLocation(self.program, "normalMatrix"),
            "lightPos": glGetUniformLocation(self.program, "lightPos"),
            "lightColor": glGetUniformLocation(self.program, "lightColor"),
            "lightType": glGetUniformLocation(self.program, "lightType"),
            "lightDirection": glGetUniformLocation(self.program, "lightDirection"),
            "cameraPos": glGetUniformLocation(self.program, "cameraPos"),
            "tex": glGetUniformLocation(self.program, "tex"),
            "hasTexture": glGetUniformLocation(self.program, "hasTexture"),
            "normalMap": glGetUniformLocation(self.program, "normalMap"),
            "hasNormalMap": glGetUniformLocation(self.program, "hasNormalMap"),
            "materialAmbient": glGetUniformLocation(self.program, "materialAmbient"),
            "materialDiffuse": glGetUniformLocation(self.program, "materialDiffuse"),
            "materialSpecular": glGetUniformLocation(self.program, "materialSpecular"),
            "materialShininess": glGetUniformLocation(self.program, "materialShininess")
        }

    def use(self):
        glUseProgram(self.program)

    # Методы для установки uniform-переменных
    def set_mat4(self, name: str, matrix: glm.mat4):
        loc = self.uniforms.get(name, -1)
        if loc != -1:
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm.value_ptr(matrix))

    def set_vec3(self, name: str, vector: glm.vec3):
        loc = self.uniforms.get(name, -1)
        if loc != -1:
            glUniform3fv(loc, 1, glm.value_ptr(vector))

    def set_float(self, name: str, value: float):
        loc = self.uniforms.get(name, -1)
        if loc != -1:
            glUniform1f(loc, value)

    def set_int(self, name: str, value: int):
        loc = self.uniforms.get(name, -1)
        if loc != -1:
            glUniform1i(loc, value)

    def set_bool(self, name: str, value: bool):
        loc = self.uniforms.get(name, -1)
        if loc != -1:
            glUniform1i(loc, int(value))

# -----------------------------------------------------------
# КЛАСС МЕША (3D-МОДЕЛИ)
# -----------------------------------------------------------
class Mesh:
    """Представление 3D-меша, загруженного из OBJ-файла, с поддержкой нормал маппинга."""
    def __init__(self, obj_file, texture_file=None, normal_map_file=None, flip_vertical=True):
        """
        flip_vertical: переворачивать ли текстуру по вертикали при загрузке
        (нужно для некоторых форматов, чтобы совпали UV-координаты).
        """
        # Загружаем модель через pywavefront
        scene = pywavefront.Wavefront(obj_file, collect_faces=True)

        vertices = []
        normals = []
        texcoord_minmax = [9999, -9999, 9999, -9999]  # minU, maxU, minV, maxV

        # Пробегаемся по всем мешам и их треугольникам
        for mesh in scene.mesh_list:
            for face in mesh.faces:
                for vertex_data in face:
                    if isinstance(vertex_data, int):
                        v_idx = vertex_data
                        vt_idx = None
                        vn_idx = None
                    else:
                        v_idx = vertex_data[0]
                        vt_idx = vertex_data[1] if len(vertex_data) > 1 else None
                        vn_idx = vertex_data[2] if len(vertex_data) > 2 else None

                    pos = scene.vertices[v_idx]

                    if vt_idx is not None and vt_idx < len(scene.tex_coords):
                        uv = scene.tex_coords[vt_idx]
                        # Сохраняем UV
                        # Заодно отследим min/max
                        if uv[0] < texcoord_minmax[0]:
                            texcoord_minmax[0] = uv[0]
                        if uv[0] > texcoord_minmax[1]:
                            texcoord_minmax[1] = uv[0]
                        if uv[1] < texcoord_minmax[2]:
                            texcoord_minmax[2] = uv[1]
                        if uv[1] > texcoord_minmax[3]:
                            texcoord_minmax[3] = uv[1]
                    else:
                        uv = [0.0, 0.0]

                    if vn_idx is not None and vn_idx < len(scene.normals):
                        norm = scene.normals[vn_idx]
                    else:
                        norm = [0.0, 0.0, 1.0]

                    vertices.extend([pos[0], pos[1], pos[2], uv[0], uv[1]])
                    normals.extend([norm[0], norm[1], norm[2]])

        self.vertices = np.array(vertices, dtype=np.float32)
        self.normals = np.array(normals, dtype=np.float32)
        self.vertex_count = len(self.vertices) // 5

        print(f"[INFO] Загрузка модели: {obj_file}")
        print(f"       Всего вершин: {self.vertex_count}")
        print(f"       UV min=({texcoord_minmax[0]:.2f}, {texcoord_minmax[2]:.2f})  "
              f"max=({texcoord_minmax[1]:.2f}, {texcoord_minmax[3]:.2f})")

        # Создаём массив касательных (для нормал маппинга)
        self.tangents = self.calculate_tangents()

        # Создание VAO и VBO
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        # 0 - вершины (pos + uv)
        self.vbo_vertices = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo_vertices)
        glBufferData(GL_ARRAY_BUFFER, self.vertices.nbytes, self.vertices, GL_STATIC_DRAW)
        # Позиция (3 float)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 20, ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        # Текстурные координаты (2 float)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 20, ctypes.c_void_p(12))
        glEnableVertexAttribArray(1)

        # 1 - нормали
        self.vbo_normals = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo_normals)
        glBufferData(GL_ARRAY_BUFFER, self.normals.nbytes, self.normals, GL_STATIC_DRAW)
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 12, ctypes.c_void_p(0))
        glEnableVertexAttribArray(2)

        # 2 - касательные
        self.vbo_tangents = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo_tangents)
        glBufferData(GL_ARRAY_BUFFER, self.tangents.nbytes, self.tangents, GL_STATIC_DRAW)
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 12, ctypes.c_void_p(0))
        glEnableVertexAttribArray(3)

        glBindVertexArray(0)

        # -------------------
        # ЗАГРУЗКА ТЕКСТУР
        # -------------------
        self.texture = None
        self.has_texture = False
        self.normal_map = None
        self.has_normal_map = False

        # Материал по умолчанию
        self.material = {
            "ambient": glm.vec3(0.2, 0.2, 0.2),
            "diffuse": glm.vec3(0.8, 0.8, 0.8),
            "specular": glm.vec3(0.0, 0.0, 0.0),
            "shininess": 32.0
        }

        # Если в OBJ/MTL есть материал, подхватываем
        texture_path = texture_file if (texture_file and os.path.exists(texture_file)) else None
        if not texture_path:
            for mat in scene.materials.values():
                if hasattr(mat, 'map_Kd') and mat.map_Kd:
                    guess_path = os.path.join(os.path.dirname(obj_file), mat.map_Kd)
                    if os.path.exists(guess_path):
                        texture_path = guess_path
                if hasattr(mat, 'Ka'):
                    self.material["ambient"] = glm.vec3(*mat.Ka)
                if hasattr(mat, 'Kd'):
                    self.material["diffuse"] = glm.vec3(*mat.Kd)
                if hasattr(mat, 'Ks'):
                    self.material["specular"] = glm.vec3(*mat.Ks)
                if hasattr(mat, 'Ns'):
                    self.material["shininess"] = float(mat.Ns)

        # Загрузка диффузной текстуры (если есть)
        if texture_path and os.path.exists(texture_path):
            self.texture = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self.texture)

            # Настройки обёртки и фильтрации
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

            if MAX_ANISOTROPY > 1.0:
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MAX_ANISOTROPY)

            try:
                with Image.open(texture_path) as img:
                    if flip_vertical:
                        img = img.transpose(Image.FLIP_TOP_BOTTOM)
                    img = img.convert("RGBA")
                    img_data = img.tobytes()
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
                                 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data)
                    glGenerateMipmap(GL_TEXTURE_2D)
                    self.has_texture = True
                    print(f"[INFO] Диффузная текстура загружена: {texture_path}")
            except Exception as e:
                print(f"[ERROR] Ошибка загрузки текстуры {texture_path}: {e}")
            finally:
                glBindTexture(GL_TEXTURE_2D, 0)

        # Загрузка нормальной карты
        if normal_map_file and os.path.exists(normal_map_file):
            self.normal_map = glGenTextures(1)
            glBindTexture(GL_TEXTURE_2D, self.normal_map)

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

            if MAX_ANISOTROPY > 1.0:
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MAX_ANISOTROPY)

            try:
                with Image.open(normal_map_file) as img:
                    if flip_vertical:
                        img = img.transpose(Image.FLIP_TOP_BOTTOM)
                    img = img.convert("RGBA")
                    img_data = img.tobytes()
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height,
                                 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data)
                    glGenerateMipmap(GL_TEXTURE_2D)
                    self.has_normal_map = True
                    print(f"[INFO] Нормальная карта загружена: {normal_map_file}")
            except Exception as e:
                print(f"[ERROR] Ошибка загрузки нормальной карты {normal_map_file}: {e}")
            finally:
                glBindTexture(GL_TEXTURE_2D, 0)

    def calculate_tangents(self):
        """Вычисление касательных (tangents) для нормал маппинга."""
        tangents = np.zeros((self.vertex_count, 3), dtype=np.float32)
        # В self.vertices каждые 5 значений: [pos_x, pos_y, pos_z, u, v]
        verts = self.vertices.reshape((self.vertex_count, 5))

        for i in range(0, self.vertex_count, 3):
            pos0 = verts[i, 0:3]
            pos1 = verts[i+1, 0:3]
            pos2 = verts[i+2, 0:3]

            uv0 = verts[i, 3:5]
            uv1 = verts[i+1, 3:5]
            uv2 = verts[i+2, 3:5]

            edge1 = pos1 - pos0
            edge2 = pos2 - pos0
            deltaUV1 = uv1 - uv0
            deltaUV2 = uv2 - uv0

            # Эпсилон во избежание деления на 0
            denom = (deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0]) + 1e-8
            f = 1.0 / denom
            tangent = f * (deltaUV2[1] * edge1 - deltaUV1[1] * edge2)

            tangents[i] = tangent
            tangents[i+1] = tangent
            tangents[i+2] = tangent

        return tangents.flatten()

    def draw(self, shader: Shader):
        """Рисуем модель, передавая все необходимые текстуры и параметры материала."""
        glBindVertexArray(self.vao)

        # Устанавливаем флаги наличия текстур
        shader.set_bool("hasTexture", self.has_texture)
        shader.set_bool("hasNormalMap", self.has_normal_map)

        # Привязываем диффузную текстуру к TEXTURE0
        if self.has_texture and self.texture:
            glActiveTexture(GL_TEXTURE0)
            glBindTexture(GL_TEXTURE_2D, self.texture)
            shader.set_int("tex", 0)

        # Привязываем нормальную карту к TEXTURE1
        if self.has_normal_map and self.normal_map:
            glActiveTexture(GL_TEXTURE1)
            glBindTexture(GL_TEXTURE_2D, self.normal_map)
            shader.set_int("normalMap", 1)

        # Передаём материал в шейдер
        shader.set_vec3("materialAmbient", self.material["ambient"])
        shader.set_vec3("materialDiffuse", self.material["diffuse"])
        shader.set_vec3("materialSpecular", self.material["specular"])
        shader.set_float("materialShininess", self.material["shininess"])

        # Рисуем все треугольники
        glDrawArrays(GL_TRIANGLES, 0, self.vertex_count)

        # Отвязываем
        glBindTexture(GL_TEXTURE_2D, 0)
        glBindVertexArray(0)

    def cleanup(self):
        """Освобождаем ресурсы."""
        if self.vao:
            glDeleteVertexArrays(1, [self.vao])

        if hasattr(self, 'vbo_vertices') and self.vbo_vertices:
            glDeleteBuffers(1, [self.vbo_vertices])
        if hasattr(self, 'vbo_normals') and self.vbo_normals:
            glDeleteBuffers(1, [self.vbo_normals])
        if hasattr(self, 'vbo_tangents') and self.vbo_tangents:
            glDeleteBuffers(1, [self.vbo_tangents])

        if self.texture:
            glDeleteTextures(1, [self.texture])
        if self.normal_map:
            glDeleteTextures(1, [self.normal_map])

# -----------------------------------------------------------
# КЛАСС ТРАНСФОРМАЦИЙ
# -----------------------------------------------------------
class Transform:
    """Управление трансформацией (позиция, поворот, масштаб)."""
    def __init__(self, position=glm.vec3(0), rotation=glm.vec3(0), scale=glm.vec3(1)):
        self.position = glm.vec3(position)
        self.rotation = glm.vec3(rotation)
        self.scale = glm.vec3(scale)
        self._matrix = glm.mat4(1.0)
        self._needs_update = True

    def translate(self, offset: glm.vec3, local: bool = False):
        """Перемещение объекта. Если local=True, смещение идёт в локальных координатах."""
        if local:
            rot_matrix = glm.mat4(1.0)
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.x), glm.vec3(1, 0, 0))
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.y), glm.vec3(0, 1, 0))
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.z), glm.vec3(0, 0, 1))
            offset = glm.vec3(rot_matrix * glm.vec4(offset, 1.0))

        self.position += offset
        self._needs_update = True

    def rotate(self, angle: float, axis: glm.vec3, local: bool = False):
        """Поворот объекта. Если local=True, ось вращения преобразуется в локальную."""
        if local:
            rot_matrix = glm.mat4(1.0)
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.x), glm.vec3(1, 0, 0))
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.y), glm.vec3(0, 1, 0))
            rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.z), glm.vec3(0, 0, 1))
            axis = glm.vec3(rot_matrix * glm.vec4(axis, 1.0))

        self.rotation += angle * axis
        self._needs_update = True

    def set_scale(self, scale: glm.vec3):
        self.scale = glm.vec3(scale)
        self._needs_update = True

    def get_matrix(self) -> glm.mat4:
        """Возвращает итоговую матрицу трансформации."""
        if self._needs_update:
            m = glm.mat4(1.0)
            m = glm.translate(m, self.position)
            m = glm.rotate(m, glm.radians(self.rotation.x), glm.vec3(1, 0, 0))
            m = glm.rotate(m, glm.radians(self.rotation.y), glm.vec3(0, 1, 0))
            m = glm.rotate(m, glm.radians(self.rotation.z), glm.vec3(0, 0, 1))
            m = glm.scale(m, self.scale)
            self._matrix = m
            self._needs_update = False
        return self._matrix

    def get_local_axes(self) -> tuple:
        """Возвращает локальные оси (forward, right, up)."""
        rot_matrix = glm.mat4(1.0)
        rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.x), glm.vec3(1, 0, 0))
        rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.y), glm.vec3(0, 1, 0))
        rot_matrix = glm.rotate(rot_matrix, glm.radians(self.rotation.z), glm.vec3(0, 0, 1))

        forward = glm.normalize(glm.vec3(rot_matrix * glm.vec4(0, 0, -1, 0)))
        right   = glm.normalize(glm.vec3(rot_matrix * glm.vec4(1, 0, 0, 0)))
        up      = glm.normalize(glm.vec3(rot_matrix * glm.vec4(0, 1, 0, 0)))
        return (forward, right, up)

# -----------------------------------------------------------
# КЛАСС АНИМАЦИИ
# -----------------------------------------------------------
class Animation:
    """Управление анимацией с ключевыми кадрами (позиция, поворот, масштаб)."""
    def __init__(self, keyframes):
        # Сортируем по времени
        self.keyframes = sorted(keyframes, key=lambda kf: kf[0])
        self.current_time = 0.0

    def update(self, delta_time: float):
        self.current_time += delta_time
        if not self.keyframes:
            return None, None, None

        # Находим текущие и следующие ключи
        next_idx = 0
        for i, (kf_time, _, _, _) in enumerate(self.keyframes):
            if self.current_time < kf_time:
                next_idx = i
                break
        else:
            # Если дошли до конца, обнулим время (зацикливание)
            next_idx = 0
            self.current_time = 0.0

        prev_idx = max(0, next_idx - 1)
        t1, pos1, rot1, scl1 = self.keyframes[prev_idx]
        t2, pos2, rot2, scl2 = self.keyframes[next_idx]

        if t2 == t1:
            return pos1, rot1, scl1

        # Интерполяция
        factor = (self.current_time - t1) / (t2 - t1)
        pos  = pos1 + (pos2 - pos1) * factor
        rot  = rot1 + (rot2 - rot1) * factor
        scl  = scl1 + (scl2 - scl1) * factor
        return pos, rot, scl

# -----------------------------------------------------------
# КЛАСС КАМЕРЫ
# -----------------------------------------------------------
class Camera:
    """Управление камерой (положение, вращение, скрипт)."""
    def __init__(self, transform: Transform, script=None):
        self.transform = transform
        self.script = script

    def get_view(self) -> glm.mat4:
        """Формируем матрицу вида через lookAt."""
        eye = self.transform.position
        forward, _, _ = self.transform.get_local_axes()
        return glm.lookAt(eye, eye + forward, glm.vec3(0, 1, 0))

    def update(self, delta_time: float, platform):
        """Вызываем скрипт, если он есть."""
        if self.script:
            self.script(self, delta_time, platform)

# -----------------------------------------------------------
# КЛАСС СВЕТА
# -----------------------------------------------------------
class Light:
    """Источник света: позиция/направление, цвет, тип (point или directional)."""
    def __init__(self, position: glm.vec3, color: glm.vec3, light_type="point", direction=None):
        self.position = position
        self.color = color
        self.type = light_type  # "point" или "directional"
        if direction is None:
            self.direction = glm.vec3(0, -1, 0)
        else:
            self.direction = glm.vec3(*direction)

# -----------------------------------------------------------
# АУДИО-ИСТОЧНИК
# -----------------------------------------------------------
class AudioSource:
    """Представление источника звука (pygame.mixer)."""
    def __init__(self, audio_file, position=glm.vec3(0), volume=1.0, loop=False, autoplay=False):
        self.audio_file = audio_file
        self.position = glm.vec3(position)
        self.volume = volume
        self.loop = loop
        self.autoplay = autoplay
        self.sound = None

        try:
            self.sound = pygame.mixer.Sound(audio_file)
            self.sound.set_volume(volume)
            if self.autoplay:
                self.play()
        except Exception as e:
            print(f"[ERROR] Ошибка загрузки аудио {audio_file}: {e}")

    def play(self):
        if self.sound:
            loops = -1 if self.loop else 0
            self.sound.play(loops=loops)

    def stop(self):
        if self.sound:
            self.sound.stop()

    def update(self, listener_pos):
        """Простая модель позиционного звука: затухание по расстоянию."""
        if self.sound:
            dist = glm.distance(self.position, listener_pos)
            attenuation = 1.0 / (1.0 + dist * 0.1)
            self.sound.set_volume(self.volume * attenuation)

# -----------------------------------------------------------
# МЕНЕДЖЕР АУДИО
# -----------------------------------------------------------
class AudioManager:
    """Управление всеми источниками звука."""
    def __init__(self):
        self.audio_sources = []

    def add_audio(self, audio_source: AudioSource):
        self.audio_sources.append(audio_source)

    def update(self, listener_pos):
        for source in self.audio_sources:
            source.update(listener_pos)

# -----------------------------------------------------------
# КЛАСС ROLE (ОБЪЕКТ СЦЕНЫ)
# -----------------------------------------------------------
class Role:
    """Любой объект на сцене: меш, шейдер, трансформация, анимация, физика и скрипт."""
    def __init__(self, mesh: Mesh, shader: Shader, transform: Transform,
                 script=None, animation: Animation=None, physics_enabled=False):
        self.mesh = mesh
        self.shader = shader
        self.transform = transform
        self.script = script
        self.animation = animation
        self.physics_enabled = physics_enabled

        self.velocity = glm.vec3(0)
        self.grounded = False

    def update(self, delta_time: float, platform):
        """Обновление состояния объекта (анимация, физика, скрипты)."""
        # Анимация
        if self.animation:
            pos, rot, scl = self.animation.update(delta_time)
            if pos is not None:
                self.transform.position = pos
                self.transform.rotation = rot
                self.transform.set_scale(scl)

        # Простая физика (гравитация)
        if self.physics_enabled:
            self.velocity += glm.vec3(0, -9.8 * delta_time, 0)
            self.transform.translate(self.velocity * delta_time)
            if self.transform.position.y < 0:
                self.transform.position.y = 0
                self.velocity.y = 0
                self.grounded = True
            else:
                self.grounded = False

        # Скрипт
        if self.script:
            self.script(self, delta_time, platform)

    def draw(self):
        """Отрисовка объекта."""
        # Устанавливаем матрицу модели и нормалей
        model = self.transform.get_matrix()
        self.shader.set_mat4("model", model)

        normal_matrix = glm.transpose(glm.inverse(glm.mat3(model)))
        self.shader.set_mat4("normalMatrix", normal_matrix)

        # Рисуем меш
        self.mesh.draw(self.shader)

    def cleanup(self):
        """Освобождаем ресурсы меша."""
        self.mesh.cleanup()

# -----------------------------------------------------------
# ПЛАТФОРМА (СЦЕНА)
# -----------------------------------------------------------
class Platform:
    """Содержит камеры, объекты, источники света, аудио и шейдер."""
    def __init__(self, config_file: str, window: Window):
        self.window = window
        self.roles = []
        self.cameras = []
        self.lights = []
        self.active_camera_idx = 0
        self.shader = None
        self.audio_manager = AudioManager()

        self.load_config(config_file)

    def load_config(self, config_file: str):
        """Загрузка сцены из JSON-файла."""
        try:
            with open(config_file, 'r') as f:
                config = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError) as e:
            raise Exception(f"Ошибка загрузки конфига {config_file}: {e}")

        # Камеры
        if not config.get('cameras'):
            raise Exception("Отсутствует секция 'cameras' в конфиге")
        for cam_data in config['cameras']:
            transform = Transform(
                glm.vec3(*cam_data['position']),
                glm.vec3(*cam_data['rotation']),
                glm.vec3(*cam_data['scale'])
            )
            script = self._load_script(cam_data.get('script'))
            camera = Camera(transform, script)
            self.cameras.append(camera)
            if cam_data.get('active', False):
                self.active_camera_idx = len(self.cameras) - 1

        # Источник света
        if 'light' not in config:
            raise Exception("Отсутствует секция 'light' в конфиге")
        light_data = config['light']
        light_type = light_data.get('type', 'point')
        direction = light_data.get('direction', [0, -1, 0])
        new_light = Light(
            glm.vec3(*light_data['position']),
            glm.vec3(*light_data['color']),
            light_type,
            direction
        )
        self.lights.append(new_light)

        # Создаём шейдер
        self.shader = Shader(
            # Вершинный шейдер
            """#version 330 core
            layout(location = 0) in vec3 aPos;
            layout(location = 1) in vec2 aTexCoord;
            layout(location = 2) in vec3 aNormal;
            layout(location = 3) in vec3 aTangent;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            uniform mat4 normalMatrix;

            out vec2 TexCoord;
            out vec3 FragPos;
            out vec3 Normal;
            out vec3 Tangent;
            out vec3 Bitangent;

            void main() {
                mat3 normalMat = mat3(normalMatrix);
                vec3 T = normalize(normalMat * aTangent);
                vec3 N = normalize(normalMat * aNormal);
                // Порядок cross(N, T) или cross(T, N) зависит от модели.
                // Если карта нормалей "перевёрнута", попробуйте менять порядок:
                vec3 B = normalize(cross(N, T));

                TexCoord = aTexCoord;
                FragPos = vec3(model * vec4(aPos, 1.0));
                Normal  = N;
                Tangent = T;
                Bitangent = B;

                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }""",
            # Фрагментный шейдер
            """#version 330 core
            out vec4 FragColor;

            in vec2 TexCoord;
            in vec3 FragPos;
            in vec3 Normal;
            in vec3 Tangent;
            in vec3 Bitangent;

            uniform vec3 lightPos;
            uniform vec3 lightColor;
            uniform int  lightType;
            uniform vec3 lightDirection;
            uniform vec3 cameraPos;

            uniform sampler2D tex;
            uniform bool hasTexture;

            uniform sampler2D normalMap;
            uniform bool hasNormalMap;

            uniform vec3 materialAmbient;
            uniform vec3 materialDiffuse;
            uniform vec3 materialSpecular;
            uniform float materialShininess;

            void main() {
                // Базовая нормаль
                vec3 norm = Normal;

                // Если есть нормальная карта, берём нормаль из неё (TBN)
                if (hasNormalMap) {
                    mat3 TBN = mat3(Tangent, Bitangent, Normal);
                    vec3 sampledNormal = texture(normalMap, TexCoord).rgb;
                    // Преобразуем [0..1] -> [-1..1]
                    sampledNormal = normalize(sampledNormal * 2.0 - 1.0);
                    norm = normalize(TBN * sampledNormal);
                }

                // Направление света
                vec3 lightDir;
                if (lightType == 1) {
                    // directional
                    lightDir = normalize(-lightDirection);
                } else {
                    // point
                    lightDir = normalize(lightPos - FragPos);
                }

                // Диффузная составляющая
                float diff = max(dot(norm, lightDir), 0.0);

                // Расчёт зеркальной (спекуляр) компоненты (Блинн–Фонг)
                vec3 viewDir = normalize(cameraPos - FragPos);
                vec3 halfDir = normalize(lightDir + viewDir);
                float spec = pow(max(dot(norm, halfDir), 0.0), materialShininess);

                // Цвет из текстуры или из materialDiffuse
                vec3 baseColor = hasTexture ? texture(tex, TexCoord).rgb : materialDiffuse;

                // Ambient
                vec3 ambient = materialAmbient * baseColor * lightColor;

                // Diffuse
                vec3 diffuse  = diff * baseColor * lightColor;

                // Specular
                vec3 specular = spec * materialSpecular * lightColor;

                vec3 finalColor = ambient + diffuse + specular;

                FragColor = vec4(finalColor, 1.0);
            }"""
        )

        # Загрузка объектов (roles)
        for obj_data in config.get('roles', []):
            mesh = Mesh(
                obj_data['obj_file'],
                obj_data.get('texture_file'),
                obj_data.get('normal_map_file'),
                flip_vertical=True  # при необходимости поменять
            )
            transform = Transform(
                glm.vec3(*obj_data['position']),
                glm.vec3(*obj_data['rotation']),
                glm.vec3(*obj_data['scale'])
            )
            script = self._load_script(obj_data.get('script'))

            animation = None
            if 'animation' in obj_data:
                keyframes = []
                for kf in obj_data['animation']:
                    keyframes.append((
                        kf['time'],
                        glm.vec3(*kf['position']),
                        glm.vec3(*kf['rotation']),
                        glm.vec3(*kf['scale'])
                    ))
                animation = Animation(keyframes)

            physics_enabled = obj_data.get('physics_enabled', False)

            role = Role(mesh, self.shader, transform, script, animation, physics_enabled)
            self.roles.append(role)

        # Загрузка аудио-источников
        if 'audios' in config:
            for audio_data in config['audios']:
                audio_file = audio_data['file']
                position = audio_data.get('position', [0, 0, 0])
                volume = audio_data.get('volume', 1.0)
                loop = audio_data.get('loop', False)
                autoplay = audio_data.get('autoplay', False)

                audio_source = AudioSource(
                    audio_file,
                    glm.vec3(*position),
                    volume,
                    loop,
                    autoplay
                )
                self.audio_manager.add_audio(audio_source)

    def _load_script(self, script_path: str):
        """Загрузка пользовательского скрипта (если указан)."""
        if script_path and os.path.exists(script_path):
            try:
                spec = importlib.util.spec_from_file_location("script", script_path)
                module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(module)
                if hasattr(module, 'update'):
                    return module.update
                else:
                    print(f"[WARNING] Скрипт {script_path} не содержит функцию 'update'.")
                    return None
            except Exception as e:
                print(f"[ERROR] Ошибка загрузки скрипта {script_path}: {e}")
                return None
        return None

    def update(self, delta_time: float):
        """Обновление сцены: объекты, камера, аудио."""
        for role in self.roles:
            role.update(delta_time, self)

        if self.cameras and 0 <= self.active_camera_idx < len(self.cameras):
            self.cameras[self.active_camera_idx].update(delta_time, self)
            # Позиция слушателя для аудио
            listener_pos = self.cameras[self.active_camera_idx].transform.position
            self.audio_manager.update(listener_pos)

    def render(self, projection: glm.mat4):
        """Рендеринг всех объектов."""
        view = glm.mat4(1.0)
        if self.cameras and 0 <= self.active_camera_idx < len(self.cameras):
            view = self.cameras[self.active_camera_idx].get_view()

        if self.shader and self.roles:
            self.shader.use()
            # Матрицы
            self.shader.set_mat4("view", view)
            self.shader.set_mat4("projection", projection)

            # Камера (положение)
            if self.cameras and 0 <= self.active_camera_idx < len(self.cameras):
                camera_pos = self.cameras[self.active_camera_idx].transform.position
                self.shader.set_vec3("cameraPos", camera_pos)

            # Свет
            if self.lights:
                light = self.lights[0]
                self.shader.set_vec3("lightPos", light.position)
                self.shader.set_vec3("lightColor", light.color)
                if light.type == "directional":
                    self.shader.set_int("lightType", 1)
                    self.shader.set_vec3("lightDirection", light.direction)
                else:
                    self.shader.set_int("lightType", 0)
                    self.shader.set_vec3("lightDirection", glm.vec3(0, 0, 0))

            # Отрисовка объектов
            for role in self.roles:
                role.draw()

    def cleanup(self):
        """Освобождение ресурсов."""
        for role in self.roles:
            role.cleanup()

    def set_active_camera(self, index: int):
        if 0 <= index < len(self.cameras):
            self.active_camera_idx = index
        else:
            print(f"[WARNING] Недопустимый индекс камеры {index}, всего камер: {len(self.cameras)}")

# -----------------------------------------------------------
# КЛАСС ДВИЖКА
# -----------------------------------------------------------
class Engine:
    """Основной класс движка, управляющий приложением."""
    def __init__(self, width: int, height: int, title: str, config_file: str):
        self.window = Window(width, height, title)
        self.platform = Platform(config_file, self.window)
        self.last_time = time.perf_counter()

    def run(self):
        """Главный цикл приложения."""
        while not self.window.should_close():
            current_time = time.perf_counter()
            delta_time = current_time - self.last_time
            self.last_time = current_time

            self.window.poll_events()
            self.platform.update(delta_time)

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            self.platform.render(self.window.projection)
            self.window.swap_buffers()

        self.platform.cleanup()
        self.window.terminate()

# -----------------------------------------------------------
# ТОЧКА ВХОДА
# -----------------------------------------------------------
if __name__ == "__main__":
    engine = Engine(800, 600, "SAGE Engine 0.486", "scene.json")
    engine.run()
