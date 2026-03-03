#pragma once
#include <GL/glew.h>
#include "mesh_data.hpp"
#include "math.hpp"

class MeshRenderer {
public:
    bool init();
    void upload(const MeshData& mesh);
    void render(const Mat4& mvp, bool wireframe, float r, float g, float b);
    void shutdown();

private:
    GLuint program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei index_count_ = 0;
    GLint u_mvp_ = -1;
    GLint u_color_ = -1;

    GLuint compileShader(GLenum type, const char* src);
    GLuint linkProgram(GLuint vs, GLuint fs);
};