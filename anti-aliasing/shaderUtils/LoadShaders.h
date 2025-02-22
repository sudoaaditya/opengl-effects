
#ifndef __LOAD_SHADERS_H
#define __LOAD_SHADERS_H


#include <GL/glew.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef struct {
    GLenum type;
    const char *filename;
    GLuint shader;
} ShaderInfo;


GLuint LoadShaders ( ShaderInfo*, FILE* );
GLuint DetachShaders ( GLuint, FILE* );


#ifdef __cplusplus 
};
#endif

#endif