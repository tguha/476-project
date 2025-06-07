#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

#include "Program.h"
#include "Texture.h"

GLuint fullscreenQuadVAO = 0;
GLuint fullscreenQuadVBO = 0;
std::shared_ptr<Texture> titleScreenTexture;
std::shared_ptr<Program> titleShader;

void initfullscreenQuad() {
    // Initialize fullscreen quad VAO and VBO
   glGenVertexArrays(1, &fullscreenQuadVAO);
   glGenBuffers(1, &fullscreenQuadVBO);

   glBindVertexArray(fullscreenQuadVAO);
   glBindBuffer(GL_ARRAY_BUFFER, fullscreenQuadVBO);

    float quadVertices[] = {
        // x, y     u, v
        -1.0f, -1.0f,  0.0f, 0.0f, // bottom-left
        1.0f, -1.0f,  1.0f, 0.0f, // bottom-right
        -1.0f,  1.0f,  0.0f, 1.0f, // top-left
        1.0f,  1.0f,  1.0f, 1.0f  // top-right
    };


    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initTitleScreen(const std::string& resourceDirectory) {
    // Load the title screen texture


    // Create and bind the shader program for the title screen
    titleShader = std::make_shared<Program>();
    titleShader->setShaderNames(resourceDirectory + "/fullscreen.vert", resourceDirectory + "/fullscreen.frag");

    if (!titleShader->init()) {
        std::cerr << "Failed to initialize title screen shader." << std::endl;
        return;
    }

    titleShader->addAttribute("position");
    titleShader->addAttribute("texCoord");

    // titleShader->addUniform("projection");
    // titleShader->addUniform("view");
    titleShader->addUniform("rotation");
    titleShader->addUniform("screenTexture");

    titleScreenTexture = make_shared<Texture>();
    titleScreenTexture->setFilename(resourceDirectory + "/titlescreen1.png");
    titleScreenTexture->init();
    titleScreenTexture->setUnit(0);
    titleScreenTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);


    initfullscreenQuad();

}

void drawTitleScreen(std::shared_ptr<Program> shader, int width, int height) {
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, titleScreenTexture->getID());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(shader->getUniform("screenTexture"), 0);

    float angle = glm::radians(90.0f); // or 180, etc.
    glUniform1f(titleShader->getUniform("rotation"), angle);


    glBindVertexArray(fullscreenQuadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    titleScreenTexture->unbind();

    shader->unbind();
}