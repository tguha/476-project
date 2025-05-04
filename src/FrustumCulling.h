#ifndef FRUSTUM_CULLING_H
#define FRUSTUM_CULLING_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Function to extract the view frustum planes from the combined projection and view matrix

void ExtractVFPlanes(glm::mat4 P, glm::mat4 V, glm::vec4 planes[6]) {
    glm::mat4 comp = P * V;
    glm::vec3 n;
    float l;
    glm::vec4 Left, Right, Bottom, Top, Near, Far;

    Left.x = comp[0][3] + comp[0][0];
    Left.y = comp[1][3] + comp[1][0];
    Left.z = comp[2][3] + comp[2][0];
    Left.w = comp[3][3] + comp[3][0];
    n = glm::vec3(Left.x, Left.y, Left.z);
    l = glm::length(n);
    planes[0] = Left = Left/l; // normalize the plane


    Right.x = comp[0][3] - comp[0][0]; // see handout to fill in with values from comp
    Right.y = comp[1][3] - comp[1][0]; // see handout to fill in with values from comp
    Right.z = comp[2][3] - comp[2][0]; // see handout to fill in with values from comp
    Right.w = comp[3][3] - comp[3][0]; // see handout to fill in with values from comp
    n = glm::vec3(Right.x, Right.y, Right.z);
    l = glm::length(n);
    planes[1] = Right = Right/l; // normalize the plane

    Bottom.x = comp[0][3] + comp[0][1]; // see handout to fill in with values from comp
    Bottom.y = comp[1][3] + comp[1][1]; // see handout to fill in with values from comp
    Bottom.z = comp[2][3] + comp[2][1]; // see handout to fill in with values from comp
    Bottom.w = comp[3][3] + comp[3][1]; // see handout to fill in with values from comp
    n = glm::vec3(Bottom.x, Bottom.y, Bottom.z);
    l = glm::length(n);
    planes[2] = Bottom = Bottom/l;

    Top.x = comp[0][3] - comp[0][1]; // see handout to fill in with values from comp
    Top.y = comp[1][3] - comp[1][1]; // see handout to fill in with values from comp
    Top.z = comp[2][3] - comp[2][1]; // see handout to fill in with values from comp
    Top.w = comp[3][3] - comp[3][1]; // see handout to fill in with values from comp
    n = glm::vec3(Top.x, Top.y, Top.z);
    l = glm::length(n);
    planes[3] = Top = Top/l;


    Near.x = comp[0][3] + comp[0][2]; // see handout to fill in with values from comp
    Near.y = comp[1][3] + comp[1][2]; // see handout to fill in with values from comp
    Near.z = comp[2][3] + comp[2][2]; // see handout to fill in with values from comp
    Near.w = comp[3][3] + comp[3][2]; // see handout to fill in with values from comp
    n = glm::vec3(Near.x, Near.y, Near.z);
    l = glm::length(n);
    planes[4] = Near = Near/l;

    Far.x = comp[0][3] - comp[0][2]; // see handout to fill in with values from comp
    Far.y = comp[1][3] - comp[1][2]; // see handout to fill in with values from comp
    Far.z = comp[2][3] - comp[2][2]; // see handout to fill in with values from comp
    Far.w = comp[3][3] - comp[3][2]; // see handout to fill in with values from comp
    n = glm::vec3(Far.x, Far.y, Far.z);
    l = glm::length(n);
    planes[5] = Far = Far/l;

}

float DistToPlane(float A, float B, float C, float D, glm::vec3 point) {
    return (A*point.x + B*point.y + C*point.z + D) / sqrt(A*A + B*B + C*C);
}

bool ViewFrustCull(glm::vec3 center, float radius, glm::vec4 planes[6]) {
    float dist;

    for (int i = 0; i < 6; i++) {
        dist = DistToPlane(planes[i].x, planes[i].y, planes[i].z, planes[i].w, center);
        if (dist < -radius) {
            return true; // Outside the frustum
        }
    }
    return false; // Inside the frustum
}

#endif // FRUSTUM_CULLING_H