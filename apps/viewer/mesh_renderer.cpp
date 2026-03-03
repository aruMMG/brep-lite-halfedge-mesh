#include "mesh_renderer.hpp"
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

GLuint MeshRenderer::compileShader(GLenum type, const char* src) {
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

GLuint MeshRenderer::linkProgram(GLuint vs, GLuint fs) {
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

bool MeshRenderer::init() {
    const char* vs_src = R"GLSL(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 uMVP;
        void main() {
            gl_Position = uMVP * vec4(aPos, 1.0);
        }
    )GLSL";

    const char* fs_src = R"GLSL(
        #version 330 core
        uniform vec3 uColor;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(uColor, 1.0);
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

    u_mvp_ = glGetUniformLocation(program_, "uMVP");
    u_color_ = glGetUniformLocation(program_, "uColor");

    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    return true;
}

void MeshRenderer::upload(const MeshData& mesh) {
    index_count_ = (GLsizei)mesh.indices.size();

    glBindVertexArray(vao_);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(mesh.positions.size() * sizeof(float)),
                 mesh.positions.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(mesh.indices.size() * sizeof(uint32_t)),
                 mesh.indices.data(),
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void MeshRenderer::render(const Mat4& mvp, bool wireframe, float r, float g, float b) {
    if (!program_ || !vao_ || index_count_ == 0) return;

    glUseProgram(program_);
    glUniformMatrix4fv(u_mvp_, 1, GL_FALSE, mvp.m);
    glUniform3f(u_color_, r, g, b);

    glBindVertexArray(vao_);

    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    glDrawElements(GL_TRIANGLES, index_count_, GL_UNSIGNED_INT, (void*)0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glUseProgram(0);
}

void MeshRenderer::shutdown() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (program_) glDeleteProgram(program_);
    ebo_ = vbo_ = vao_ = program_ = 0;
    index_count_ = 0;
}