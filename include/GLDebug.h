#include <glad/glad.h>
#include <cstring>
#include <iostream>
#define glCheckError() glCheckError_(__FILE__, __LINE__)

GLenum glCheckError_(const char* file, int line);
GLenum glCheckShader(GLuint shader);
