#pragma once
#include <GL/glew.h>

class SimpleRenderer {
public:
    bool init();
    void render(int fb_width, int fb_height);
    void shutdown();

private:
    GLuint program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    GLuint compileShader(GLenum type, const char* src);
    GLuint linkProgram(GLuint vs, GLuint fs);
};