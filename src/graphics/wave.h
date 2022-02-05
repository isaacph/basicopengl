#ifndef _GRAPHICS_WAVE_H
#define _GRAPHICS_WAVE_H
#include "graphics.h"
#include <map>
#include <memory>

class WaveRender {
public:
    WaveRender();
    void render(glm::mat4 matrix, glm::vec4 color, float radius, float thickness);
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
    GLint uniformColor;
    GLint uniformRadius;
    GLint uniformThickness;
};

#endif
