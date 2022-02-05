#ifndef _GRAPHICS_SPRITESHEET_H
#define _GRAPHICS_SPRITESHEET_H
#include "graphics.h"
#include <map>
#include <memory>

struct SpritesheetSpec {
    glm::vec2 min;
    glm::vec2 max;
};

SpritesheetSpec textureGrid(int sheetWidth, int sheetHeight, int index);

class SpritesheetRender {
public:
    SpritesheetRender();
    void render(glm::mat4 matrix, glm::vec4 color, GLint sampler, SpritesheetSpec spec);
private:
    // Shared is used so that Renders can be copied and still work just fine
    struct Shared {
        GLint program;
        GLuint vbo;
        GLuint vao;
        ~Shared();
    };
    std::shared_ptr<Shared> shared;
    GLint uniformMatrix;
    GLint uniformMin;
    GLint uniformMax;
    GLint uniformColor;
    GLint uniformSampler;
};

#endif
