#include "spritesheet.h"
#include "graphics.h"

SpritesheetSpec textureGrid(int sheetWidth, int sheetHeight, int index) {
    float tileWidth = 1.0f / sheetWidth;
    float tileHeight = 1.0f / sheetHeight;
    float x = tileWidth * (index % sheetWidth);
    float y = tileHeight * (int) (index / sheetHeight);
    return {
        glm::vec2(x, y),
        glm::vec2(x + tileWidth, y + tileHeight)
    };
}

SpritesheetRender::SpritesheetRender() {
    GLint vshader = readShader("res/spritesheet_v.glsl");
    GLint fshader = readShader("res/texture_f.glsl");
    GLint program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glBindAttribLocation(program, ATTRIB_POSITION, "position");
    glBindAttribLocation(program, ATTRIB_TEXTURE, "texture");
    glLinkProgram(program);
    checkProgram(program);
    uniformMatrix = glGetUniformLocation(program, "matrix");
    uniformSampler = glGetUniformLocation(program, "sampler");
    uniformColor = glGetUniformLocation(program, "color");
    uniformMin = glGetUniformLocation(program, "min");
    uniformMax = glGetUniformLocation(program, "max");

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

void SpritesheetRender::render(glm::mat4 matrix, glm::vec4 color, GLint sampler, SpritesheetSpec spec) {
    glUseProgram(shared->program);
    glUniformMatrix4fv(uniformMatrix, 1, false, glm::value_ptr(matrix));
    glUniform4f(uniformColor, color.x, color.y, color.z, color.w);
    glUniform1i(uniformSampler, sampler);
    glUniform2f(uniformMin, spec.min.x, spec.min.y);
    glUniform2f(uniformMax, spec.max.x, spec.max.y);
    glBindVertexArray(shared->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

SpritesheetRender::Shared::~Shared() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

