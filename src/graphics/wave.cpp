#include "wave.h"
#include "graphics.h"

WaveRender::WaveRender() {
    GLint vshader = readShader("res/texture_v.glsl");
    GLint fshader = readShader("res/wave_f.glsl");
    GLint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glBindAttribLocation(program, ATTRIB_POSITION, "position");
    glBindAttribLocation(program, ATTRIB_TEXTURE, "texture");
    glLinkProgram(program);
    checkProgram(program);
    uniformMatrix = glGetUniformLocation(program, "matrix");
    uniformColor = glGetUniformLocation(program, "color");
    uniformRadius = glGetUniformLocation(program, "radius");
    uniformThickness = glGetUniformLocation(program, "thickness");

    glDeleteShader(vshader);
    glDeleteShader(fshader);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLfloat data[] = {
        -0.5f, -0.5f, 0.0f, 0.0f,
        -0.5f, +0.5f, 0.0f, 1.0f,
        +0.5f, +0.5f, 1.0f, 1.0f,
        +0.5f, +0.5f, 1.0f, 1.0f,
        +0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f,
    };
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(ATTRIB_POSITION);
    glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*) 0);
    glEnableVertexAttribArray(ATTRIB_TEXTURE);
    glVertexAttribPointer(ATTRIB_TEXTURE, 2, GL_FLOAT, false, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

    shared = std::shared_ptr<Shared>(new Shared());
    shared->program = program;
    shared->vbo = vbo;
    shared->vao = vao;
}

void WaveRender::render(glm::mat4 matrix, glm::vec4 color, float radius, float thickness) {
    glUseProgram(shared->program);
    glUniformMatrix4fv(uniformMatrix, 1, false, glm::value_ptr(matrix));
    glUniform4f(uniformColor, color.x, color.y, color.z, color.w);
    glUniform1f(uniformRadius, radius);
    glUniform1f(uniformThickness, thickness);
    glBindVertexArray(shared->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

WaveRender::Shared::~Shared() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}
