#ifndef LIGHTTRAIL_H
#define LIGHTTRAIL_H

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <vector>
#include <deque>
#include "Program.h"

using namespace glm;
using namespace std;

struct TrailSegment {
    vec3 startPos;
    vec3 endPos;
    vec3 direction;
    float length;
};

class LightTrail
{
private:
    GLuint VBO, EBO, VAO;
    vector<vec3> vertices;
    vector<unsigned int> indices;

    // Trail properties
    deque<TrailSegment> segments;
    vec3 startPos;
    vec3 currPos;
    vec3 lastDirection;
    float trailWidth;
    float trailHeight;
    vec4 trailColor;
    float minSegmentLength; // Minimum length before creating a new segment
    size_t maxSegments;     // Maximum number of segments to store

    // Shader program
    shared_ptr<Program> shaderProg;

public:
    LightTrail(shared_ptr<Program> prog, float width = 0.1f, float height = 0.05f) {
        shaderProg = prog;
        trailWidth = width;
        trailHeight = height;
        trailColor = vec4(0.0f, 0.8f, 1.0f, 1.0f);
        minSegmentLength = 0.1f;
        maxSegments = 100; // Adjust based on your needs
        lastDirection = vec3(0.0f, 0.0f, -1.0f);

        initBuffers();
    }

    ~LightTrail() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteVertexArrays(1, &VAO);
    }

    void initBuffers() {
        // Initialize empty buffers
        vertices.clear();
        indices.clear();

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
    }

    void setStartPos(const vec3& pos) {
        startPos = pos;
        currPos = pos;
        segments.clear();

        // Create the first segment
        TrailSegment firstSegment;
        firstSegment.startPos = startPos;
        firstSegment.endPos = startPos;
        firstSegment.direction = lastDirection;
        firstSegment.length = 0.0f;
        segments.push_back(firstSegment);

        updateGeometry();
    }

    void setTrailColor(const vec4& color) {
        trailColor = color;
    }

    void updatePosition(const vec3& newPos) {
        vec3 prevPos = currPos;
        currPos = newPos;

        // Calculate new direction
        vec3 newDirection = normalize(currPos - prevPos);

        // If direction has changed significantly or this is the first segment, create a new segment
        float directionDot = dot(newDirection, lastDirection);
        bool directionChanged = directionDot < 0.99f; // About 8 degrees threshold

        if (directionChanged || segments.empty()) {
            // If there's a previous segment, use its end position as the start of the new segment
            vec3 startPos = segments.empty() ? prevPos : segments.back().endPos;

            // Create new segment
            TrailSegment newSegment;
            newSegment.startPos = startPos;
            newSegment.endPos = currPos;
            newSegment.direction = newDirection;
            newSegment.length = length(currPos - startPos);

            lastDirection = newDirection;
            segments.push_back(newSegment);

            // Limit the number of segments
            if (segments.size() > maxSegments) {
                segments.pop_front();
            }
        } else {
            // Update the last segment
            segments.back().endPos = currPos;
            segments.back().length = length(currPos - segments.back().startPos);
        }

        updateGeometry();
    }


    void updateGeometry() {
        vertices.clear();
        indices.clear();

        if (segments.empty()) {
            return;
        }

        int vertexOffset = 0;

        // Create geometry for each segment
        for (const auto& segment : segments) {
            vec3 direction = normalize(segment.endPos - segment.startPos);

            // Create a coordinate system for this segment
            vec3 forward = direction;
            vec3 up = vec3(0.0f, 1.0f, 0.0f);

            // Handle case where direction is parallel to up vector
            if (abs(dot(forward, up)) > 0.999f) {
                up = vec3(1.0f, 0.0f, 0.0f);
            }

            vec3 right = normalize(cross(forward, up));
            up = normalize(cross(right, forward));

            // Calculate the 8 vertices of this segment
            vec3 bottomLeftStart = segment.startPos - (right * (trailWidth/2)) - (up * (trailHeight/2));
            vec3 bottomRightStart = segment.startPos + (right * (trailWidth/2)) - (up * (trailHeight/2));
            vec3 topRightStart = segment.startPos + (right * (trailWidth/2)) + (up * (trailHeight/2));
            vec3 topLeftStart = segment.startPos - (right * (trailWidth/2)) + (up * (trailHeight/2));

            vec3 bottomLeftEnd = segment.endPos - (right * (trailWidth/2)) - (up * (trailHeight/2));
            vec3 bottomRightEnd = segment.endPos + (right * (trailWidth/2)) - (up * (trailHeight/2));
            vec3 topRightEnd = segment.endPos + (right * (trailWidth/2)) + (up * (trailHeight/2));
            vec3 topLeftEnd = segment.endPos - (right * (trailWidth/2)) + (up * (trailHeight/2));

            // Add vertices
            vertices.push_back(bottomLeftStart);  // 0
            vertices.push_back(bottomRightStart); // 1
            vertices.push_back(topRightStart);    // 2
            vertices.push_back(topLeftStart);     // 3
            vertices.push_back(bottomLeftEnd);    // 4
            vertices.push_back(bottomRightEnd);   // 5
            vertices.push_back(topRightEnd);      // 6
            vertices.push_back(topLeftEnd);       // 7

            // Calculate bounding box for this segment


            // Add indices for this segment
            // Back face
            indices.push_back(vertexOffset + 0);
            indices.push_back(vertexOffset + 1);
            indices.push_back(vertexOffset + 2);
            indices.push_back(vertexOffset + 2);
            indices.push_back(vertexOffset + 3);
            indices.push_back(vertexOffset + 0);

            // Front face
            indices.push_back(vertexOffset + 4);
            indices.push_back(vertexOffset + 5);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 7);
            indices.push_back(vertexOffset + 4);

            // Left face
            indices.push_back(vertexOffset + 0);
            indices.push_back(vertexOffset + 3);
            indices.push_back(vertexOffset + 7);
            indices.push_back(vertexOffset + 7);
            indices.push_back(vertexOffset + 4);
            indices.push_back(vertexOffset + 0);

            // Right face
            indices.push_back(vertexOffset + 1);
            indices.push_back(vertexOffset + 2);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 5);
            indices.push_back(vertexOffset + 1);

            // Bottom face
            indices.push_back(vertexOffset + 0);
            indices.push_back(vertexOffset + 1);
            indices.push_back(vertexOffset + 5);
            indices.push_back(vertexOffset + 5);
            indices.push_back(vertexOffset + 4);
            indices.push_back(vertexOffset + 0);

            // Top face
            indices.push_back(vertexOffset + 3);
            indices.push_back(vertexOffset + 2);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 6);
            indices.push_back(vertexOffset + 7);
            indices.push_back(vertexOffset + 3);

            vertexOffset += 8;
        }

        // Update the GPU buffers
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), vertices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
    }

    void draw() {
        if (vertices.empty() || indices.empty()) {
            return;
        }

        // shaderProg->bind();

        // glUniformMatrix4fv(shaderProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection));
        // glUniformMatrix4fv(shaderProg->getUniform("V"), 1, GL_FALSE, value_ptr(View));
        // glUniformMatrix4fv(shaderProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model));

        // // Pass color if shader supports it
        // if (shaderProg->getUniform("trailColor") != -1) {
        //     glUniform4fv(shaderProg->getUniform("trailColor"), 1, value_ptr(trailColor));
        // }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // shaderProg->unbind();
    }

    void clearTrail() {
        segments.clear();
        updateGeometry();
    }

    void setMaxSegments(size_t max) {
        maxSegments = max;

        // Remove excess segments if needed
        while (segments.size() > maxSegments) {
            segments.pop_front();
        }

        updateGeometry();
    }

    void setDirectionChangeThreshold(float degrees) {
        // Convert degrees to dot product threshold
        float radians = degrees * 3.14159f / 180.0f;
        float threshold = cos(radians);
        // Store threshold for use in updatePosition
        // (This would need a class member to store)
    }

    vec3 getCurrentPosition() {
        return currPos;
    }

    vec3 getDirection() {
        return lastDirection;
    }
};

#endif