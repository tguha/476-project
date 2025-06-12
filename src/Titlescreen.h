#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

#include "Program.h"
#include "Texture.h"

std::shared_ptr<Texture> titleScreenTexture;
std::shared_ptr<Program> titleShader;
std::shared_ptr<Program> keyHUDshader;
std::shared_ptr<Texture> keyScreenTexture;
std::shared_ptr<Texture> catSadScreenTexture;

GLuint screenVAO = 0;
GLuint screenVBO = 0;

GLuint keyHUDVAO = 0;
GLuint keyHUDVBO = 0;

void initScreenQuad() {
    glGenVertexArrays(1, &screenVAO);
    glBindVertexArray(screenVAO);

    static const GLfloat g_quad_vertex_buffer_data[] = {
			-1.0f, -1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			-1.0f,  1.0f, 0.0f,
			1.0f, -1.0f, 0.0f,
			1.0f,  1.0f, 0.0f,
    };

    glGenBuffers(1, &screenVBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initTitleScreen(const std::string& resourceDirectory) {
    // Load the title screen texture


    // Create and bind the shader program for the title screen
    titleShader = std::make_shared<Program>();
    titleShader->setShaderNames(resourceDirectory + "/fullscreen_vert.glsl", resourceDirectory + "/fullscreen_frag.glsl");

    if (!titleShader->init()) {
        std::cerr << "Failed to initialize title screen shader." << std::endl;
        return;
    }

    titleShader->addAttribute("position");
    titleShader->addUniform("screenTexture");

    titleScreenTexture = make_shared<Texture>();
    titleScreenTexture->setFilename(resourceDirectory + "/titlescreen1.png");
    titleScreenTexture->init();
    titleScreenTexture->setUnit(0);
    titleScreenTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);


    // initfullscreenQuad();
    initScreenQuad();

}

void drawTitleScreen(std::shared_ptr<Program> shader, int width, int height) {
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, titleScreenTexture->getID());
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glUniform1i(shader->getUniform("screenTexture"), 0);
    // titleScreenTexture->bind(shader->getUniform("screenTexture"));

    // float angle = glm::radians(0.0f); // or 180, etc.
    // glUniform1f(titleShader->getUniform("rotation"), angle);

    glBindVertexArray(screenVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    titleScreenTexture->unbind();

    shader->unbind();
}

void initKeyQuad() {
    glGenVertexArrays(1, &keyHUDVAO);
    glBindVertexArray(keyHUDVAO);

    glGenBuffers(1, &keyHUDVBO);
    glBindBuffer(GL_ARRAY_BUFFER, keyHUDVBO);

    float quadVerts[] = {
    // offsetX, offsetY,   u, v
    -0.5f, -0.5f,         0.0f, 0.0f,
     0.5f, -0.5f,         1.0f, 0.0f,
    -0.5f,  0.5f,         0.0f, 1.0f,
    -0.5f,  0.5f,         0.0f, 1.0f,
     0.5f, -0.5f,         1.0f, 0.0f,
     0.5f,  0.5f,         1.0f, 1.0f,
    };


    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    // Attribute 0: offset from center (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(0);

    // Attribute 1: texCoord (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void initKeyFBO(const std::string& resourceDirectory, int width, int height) {
    keyHUDshader = std::make_shared<Program>();
    keyHUDshader->setShaderNames(resourceDirectory + "/keyHUD_vert.glsl", resourceDirectory + "/keyHUD_frag.glsl");
    if (!keyHUDshader->init()) {
        std::cerr << "Failed to initialize title screen shader." << std::endl;
        return;
    }
    keyHUDshader->addAttribute("position");
    keyHUDshader->addAttribute("texCoord");

    keyHUDshader->addUniform("screenTexture");
    keyHUDshader->addUniform("center_px");
    keyHUDshader->addUniform("size_px");
    keyHUDshader->addUniform("screenSize");


    keyScreenTexture = make_shared<Texture>();
    keyScreenTexture->setFilename(resourceDirectory + "/Key_and_Lock/key.jpg");
    // keyScreenTexture->setFilename(resourceDirectory + "/titlescreen1.png"); // Use the same texture for testing
    keyScreenTexture->init();
    keyScreenTexture->setUnit(0);
    keyScreenTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    catSadScreenTexture = make_shared<Texture>();
    catSadScreenTexture->setFilename(resourceDirectory + "/sadcat.png");
    catSadScreenTexture->init();
    catSadScreenTexture->setUnit(0);
    catSadScreenTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    initKeyQuad();
}

void DrawKeyHUD(std::shared_ptr<Program> shader, GLuint tex, int width, int height, int keysCount, glm::vec2 keypos, glm::vec2 keysize) {
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(shader->getUniform("screenTexture"), 0);
    glUniform2f(shader->getUniform("screenSize"), width, height);

    for (int i = 0; i < keysCount; ++i) {
        glm::vec2 centerPx = keypos + glm::vec2(i * keysize.x - 3.0f, 0.0f); // Calculate the center position for each key
        glm::vec2 sizePx = keysize; // Use the same size for each key

        glUniform2f(shader->getUniform("center_px"), centerPx.x, centerPx.y);
        glUniform2f(shader->getUniform("size_px"), sizePx.x, sizePx.y);

        glBindVertexArray(keyHUDVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    shader->unbind();
}

void DrawTextoScreen(std::shared_ptr<Program> shader, GLuint tex, int width, int height, glm::vec2 pos, glm::vec2 size) {
    shader->bind();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glUniform1i(shader->getUniform("screenTexture"), 0);
    glUniform2f(shader->getUniform("screenSize"), width, height);

    glm::vec2 centerPx = pos; // Center position for the text
    glm::vec2 sizePx = size; // Size for the text

    glUniform2f(shader->getUniform("center_px"), centerPx.x, centerPx.y);
    glUniform2f(shader->getUniform("size_px"), sizePx.x, sizePx.y);

    glBindVertexArray(keyHUDVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    shader->unbind();
}