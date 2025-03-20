#include <cstdlib>
#include <iostream>

#include <gl/glew.h>
#include <gl/GL.h>
#include "LoadShaders.h"

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

#ifdef __cplusplus
extern "C" {
#endif

static const GLchar* ReadShader (const char *filename, FILE *fptr) {

    FILE *infile = fopen(filename, "rb");

    if(infile == NULL) {
        fprintf(fptr, "Uable to open file %s \n", filename);
        fflush(fptr);
        return NULL;        
    } else {
        fprintf(fptr, "File opened %s \n", filename);
        fflush(fptr);
    }

    fseek(infile, 0, SEEK_END);
    int len = ftell(infile);
    fseek(infile, 0, SEEK_SET);

    GLchar *source = new GLchar[len + 1];

    fread(source, 1, len, infile);
    fclose(infile);

    source[len] = 0;

    return const_cast<const GLchar*>(source);
}

GLuint LoadShaders (ShaderInfo *shaders, FILE *fptr) {
    
    if(shaders == NULL) {
        return 0;
    }

    fprintf(fptr, "\nLoadShaders\n");
    fflush(fptr);

    GLuint program = glCreateProgram();

    ShaderInfo *entry = shaders;

    while(entry->type != GL_NONE) {
        GLuint shader = glCreateShader(entry->type);

        entry->shader = shader;

        const GLchar* source = ReadShader(entry->filename, fptr);

        if(source == NULL) {
            for ( entry = shaders; entry->type != GL_NONE; ++entry ) {
                glDeleteShader( entry->shader );
                entry->shader = 0;
            }

            return 0;
        }

        glShaderSource(shader, 1, &source, NULL);
        delete [] source;

        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if(!compiled) {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            GLchar *log = new GLchar[len+1];
            glGetShaderInfoLog(shader, len, &len, log);
            fprintf(fptr, "Shader compilation failed: ( %s ) -> %s \n", entry->filename, log);
            fflush(fptr);
            delete [] log;
            return 0;
        } else {
            fprintf(fptr, "Shader compilation successful: ( %s )\n", entry->filename);
            fflush(fptr);
        }

        glAttachShader(program, shader);
        ++entry;
    }

    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    for ( entry = shaders; entry->type != GL_NONE; ++entry ) {
        glDeleteShader( entry->shader );
        entry->shader = 0;
    }

    if(!linked) {
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);

        GLchar *log = new GLchar[len + 1];
        glGetProgramInfoLog(program, len, &len, log);
        fprintf(fptr, "Program linkage failed: %s \n", log);
        fflush(fptr);
        delete [] log;
        return 0;
    } else {
        fprintf(fptr, "Program linkage successful\n");
        fflush(fptr);
    }

    return program;
}

GLuint DetachShaders(GLuint program, FILE *fptr) {
    if(!program) {
        return -1;
    }

    fprintf(fptr, "\n");
    fflush(fptr);

    GLsizei iShaderCnt = 0;
    GLsizei iShaderNo = 0;

    glUseProgram(program);

    glGetProgramiv(
        program,
        GL_ATTACHED_SHADERS,
        &iShaderCnt
    );

    GLuint *pShaders = (GLuint*)malloc(iShaderCnt * sizeof(GLuint));

    if(pShaders) {
        glGetAttachedShaders(
            program,
            iShaderCnt, 
            &iShaderCnt, 
            pShaders
        );

        for(iShaderNo = 0; iShaderNo < iShaderCnt; iShaderNo++) {
            glDetachShader(program, pShaders[iShaderNo]);
            fprintf(fptr, "Detached Shader: %ld\n", pShaders[iShaderNo]);
            fflush(fptr);
            pShaders[iShaderNo] = 0;
        }
        free(pShaders);
    }

    glDeleteProgram(program);
    glUseProgram(0);

    return 0;
}

#ifdef __cplusplus
};
#endif

