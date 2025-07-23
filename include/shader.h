#define SHADER_H
#ifdef SHADER_H

#include <glad/glad.h>
#include <fstream>
#include <glm/common.hpp>
#include <cassert>
#include <iostream>
#include <sstream>

using std::ifstream;
using std::ios;

char* read_stream_to_cstr(ifstream &stream);

void checkCompileErrors(unsigned int shader, std::string type);

void compile(const char* vertexPath, const char* fragmentPath, unsigned int* program_id);
#endif
