#include "font_renderer.h"
#include <shader.h>

char* read_stream_to_cstr(ifstream &stream){
  char* cstr = NULL;
  unsigned int file_size_char = stream.seekg(0, ios::end).tellg();
  cstr = (char *) malloc(sizeof(float) * (file_size_char + 1));//NULL character
  assert(cstr);
  stream.seekg(0);
  stream.read(cstr,file_size_char);
  cstr[file_size_char] = '\0';
  return cstr;
}

void checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

void compile(const char* vertex_shader, const char* fragment_shader, unsigned int* program_id){
  // 1. compile shaders
  unsigned int vertex, fragment;
  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vertex_shader, NULL);
  glCompileShader(vertex);
  checkCompileErrors(vertex, "VERTEX");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fragment_shader, NULL);
  glCompileShader(fragment);
  checkCompileErrors(fragment, "FRAGMENT");
  // shader Program
  *program_id = glCreateProgram();
  glAttachShader(*program_id, vertex);
  glAttachShader(*program_id, fragment);
  glLinkProgram(*program_id);
  checkCompileErrors(*program_id, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer necessary
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}
