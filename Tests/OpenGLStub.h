#pragma once

// Простая заглушка для сред без GL-контекста в тестах
// Нужна для случаев, когда создаются текстуры/спрайты, но не вызываются GL-функции.
#ifndef GL_VERSION
using GLuint = unsigned int;
using GLenum = unsigned int;
using GLsizei = int;
using GLint = int;
using GLfloat = float;
using GLboolean = unsigned char;
using GLbitfield = unsigned int;
#endif
