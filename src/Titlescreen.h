#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

#include "Program.h"
#include "Texture.h"

std::shared_ptr<Texture> titleScreenTexture;
std::shared_ptr<Program> titleShader;

GLuint screenVAO = 0;
GLuint screenVBO = 0;
GLuint titlescreenframebuf[2];
GLuint titlescreentexture[2];
GLuint titlescreendepthbuffer;

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
    titleShader->setShaderNames(resourceDirectory + "/fullscreen.vert", resourceDirectory + "/fullscreen.frag");

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

void createScreenFBO(GLuint& fb, GLuint& tex, int width, int height) {
    // set up framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    // set up texture
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        exit(0);
    }
}

void initScreenFBO(const std::string& resourceDirectory, int width, int height) {
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

    glGenFramebuffers(2, titlescreenframebuf);
    glGenTextures(2, titlescreentexture);
    glGenRenderbuffers(1, &titlescreendepthbuffer);

    createScreenFBO(titlescreenframebuf[0], titlescreentexture[0], width, height);

    // set up depth necessary since we are rendering a mesh that needs depth testing
    glBindRenderbuffer(GL_RENDERBUFFER, titlescreendepthbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, titlescreendepthbuffer);

    // more FBO set up
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers); // Set the draw buffer to the color attachment

    // create another FBO so we can swap back and forth
    createScreenFBO(titlescreenframebuf[1], titlescreentexture[1], width, height);
    // this one doesnt need a depth buffer since we are not rendering a mesh to it
}

void ProcessDrawTitleScreen(std::shared_ptr<Program> shader, GLuint inTex) {
    //set up inTex as my input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inTex);
    //example applying of 'drawing' the FBO texture
    //this shader just draws right now
    shader->bind();
      glUniform1i(shader->getUniform("screenTexture"), 0);
        glBindVertexArray(screenVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shader->unbind();
}