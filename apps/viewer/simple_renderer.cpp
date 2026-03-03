#include "simple_renderer.hpp"
#include <cstdio>
#include <vector>

static void print_shader_log(GLuint shader) {
    GLint len = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        std::vector<char> buf(len);
        glGetShaderInfoLog(shader, len, nullptr, buf.data());
        std::fprintf(stderr, "%s\n", buf.data());
    }
}

static void print_program_log(GLuint prog) {
    GLint len = 0;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        std::vector<char> buf(len);
        glGetProgramInfoLog(prog, len, nullptr, buf.data());
        std::fprintf(stderr, "%s\n", buf.data());
    }
}

GLuint SimpleRenderer::compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);

    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        std::fprintf(stderr, "Shader compile failed:\n");
        print_shader_log(s);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint SimpleRenderer::linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        std::fprintf(stderr, "Program link failed:\n");
        print_program_log(p);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

bool SimpleRenderer::init() {
    const char* vs_src = R"GLSL(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aCol;
        out vec3 vCol;
        void main() {
            vCol = aCol;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )GLSL";

    const char* fs_src = R"GLSL(
        #version 330 core
        in vec3 vCol;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(vCol, 1.0);
        }
    )GLSL";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src);
    if (!vs) return false;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src);
    if (!fs) { glDeleteShader(vs); return false; }

    program_ = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!program_) return false;

    // Interleaved: pos(x,y), color(r,g,b)
    const float verts[] = {
        //  x,     y,      r,    g,    b
         0.0f,  0.6f,   1.0f, 0.2f, 0.2f,
        -0.6f, -0.6f,   0.2f, 1.0f, 0.2f,
         0.6f, -0.6f,   0.2f, 0.2f, 1.0f
    };

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void SimpleRenderer::render(int fb_width, int fb_height) {
    (void)fb_width; (void)fb_height;

    glUseProgram(program_);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);
}

void SimpleRenderer::shutdown() {
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (program_) glDeleteProgram(program_);
    vbo_ = vao_ = program_ = 0;
}