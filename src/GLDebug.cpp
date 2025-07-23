#include <GLDebug.h>
GLenum glCheckError_(const char* file, int line) {
    GLenum errorCode;
    char error[256]; // Buffer for error message
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        // Initialize error string
        error[0] = '\0';
        
        // Map error codes to strings
        switch (errorCode) {
            case GL_INVALID_ENUM:
                strcpy(error, "INVALID_ENUM");
                break;
            case GL_INVALID_VALUE:
                strcpy(error, "INVALID_VALUE");
                break;
            case GL_INVALID_OPERATION:
                strcpy(error, "INVALID_OPERATION");
                break;
            case GL_OUT_OF_MEMORY:
                strcpy(error, "OUT_OF_MEMORY");
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                strcpy(error, "INVALID_FRAMEBUFFER_OPERATION");
                break;
            default:
                snprintf(error, sizeof(error), "UNKNOWN_ERROR (%d)", errorCode);
                break;
        }
        // Print error message with file and line
        printf("%s | %s (%d)\n", error, file, line);
    }
    return errorCode;
}

GLenum glCheckShader(GLuint shader) {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
    return success;
}
