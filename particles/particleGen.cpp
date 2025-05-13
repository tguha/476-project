#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include "particleGen.h"
#include "../src/GLSL.h"
#include "../src/Program.h"
#include "../src/MatrixStack.h"
#include "../src/Texture.h"
#include "../src/Entity.h"

using namespace std;

particleGen::particleGen(vec3 source, float r_l, float r_h, float g_l, float g_h, float b_l, float b_h, float scale_l, float scale_h)
{
	t = 0.0f;
	h = 0.01f;
	g = vec3(0.0f, -0.098, 0.0f);
	start = source;
	theCamera = glm::mat4(1.0);

	r_low = r_l;
	r_high = r_h;
	g_low = g_l;
	g_high = g_h;
	b_low = b_l;
	b_high = b_h;
	scale_low = scale_l;
	scale_high = scale_h;
}

void particleGen::gpuSetup() {
	glEnable(GL_PROGRAM_POINT_SIZE);
	//cout << numP << endl;
 	for (int i=0; i < numP; i++) {
		points[i * 3 + 0] = start.x;
		points[i * 3 + 1] = start.y;
		points[i * 3 + 2] = start.z;
		
		// Initialization of pointColors DOES NOTHING, but good practice
		pointColors[i * 4 + 0] = 1.0;
		pointColors[i * 4 + 1] = 1.0;  
		pointColors[i * 4 + 2] = 1.0;
		pointColors[i * 4 + 3] = 1.0;

		auto particle = make_shared<Particle>(start);
		particles.push_back(particle);
		unusedParticles.push_back(particle);
		particle->load(start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
	}

	//generate the VAO
   glGenVertexArrays(1, &vertArrObj);
   glBindVertexArray(vertArrObj);

   //generate vertex buffer to hand off to OGL - using instancing
   glGenBuffers(1, &vertBuffObj);
   //set the current state to focus on our vertex buffer
   glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
   //actually memcopy the data - only do this once
   glBufferData(GL_ARRAY_BUFFER, sizeof(points), &points[0], GL_STREAM_DRAW);


   // USE SAME VAO
   //generate vertex buffer to hand off to OGL - using instancing
   glGenBuffers(1, &colorBuffObj);
   //set the current state to focus on our vertex buffer
   glBindBuffer(GL_ARRAY_BUFFER, colorBuffObj);
   //actually memcopy the data - only do this once
   glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), &pointColors[0], GL_STREAM_DRAW);
   
   assert(glGetError() == GL_NO_ERROR);
	
}

void particleGen::reSet() {
	for (int i=0; i < numP; i++) {
		particles[i]->load(start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
	}
}

void particleGen::drawMe(std::shared_ptr<Program> prog) {
    // Enable blending for transparent particles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable depth testing but don't write to depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glBindVertexArray(vertArrObj);

    // COLOR BUF
    int h_col = prog->getAttribute("vertColor");
    GLSL::enableVertexAttribArray(h_col);
    glBindBuffer(GL_ARRAY_BUFFER, colorBuffObj);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // POS BUF
    int h_pos = prog->getAttribute("vertPos");
    GLSL::enableVertexAttribArray(h_pos);
    glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);

    // Draw the points!
    glDrawArraysInstanced(GL_POINTS, 0, 1, numP);
    
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Reset state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void particleGen::update(float frameTime) {
  vec3 pos;
  vec4 col;

  //update the particles (called in render)
  for(auto particle : particles) {
      particle->update(t, h, g, start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high, frameTime);
  }
  t += h;

  // Calculate camera space positions for all particles
  for (int i = 0; i < numP; i++) {
      particles[i]->calcCamDist(theCamera);
      particles[i]->resize();
  }

  // Sort particles by depth (farther particles first)
  sorter.C = theCamera;
  sort(particles.begin(), particles.end(), sorter);

  //go through all the particles and update the CPU buffer
  for (int i = 0; i < numP; i++) {
      pos = particles[i]->getPosition();
      col = particles[i]->getColor();
      points[i*3+0] = pos.x; 
      points[i*3+1] = pos.y; 
      points[i*3+2] = pos.z; 

      // Set color array based on particle.update()
      pointColors[i*4+0] = col.r; 
      pointColors[i*4+1] = col.g; 
      pointColors[i*4+2] = col.b;
      pointColors[i*4+3] = col.a;
  }

  //update the GPU data
  glBindBuffer(GL_ARRAY_BUFFER, vertBuffObj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*numP*3, points);

  glBindBuffer(GL_ARRAY_BUFFER, colorBuffObj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*numP*4, pointColors);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void particleGen::initParticleGroup(int PARTICLES_PER_SPRAY, vec3 playerPos, Entity* Ent) {
	for (int i = 0; i < PARTICLES_PER_SPRAY; i++) {
		std::shared_ptr<Particle> particleToAdd = unusedParticles.back();
		unusedParticles.pop_back();
		particleToAdd->assignGroup(playerPos, Ent, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
		particleQueue.push(particleToAdd);
	}
}
void particleGen::deleteOldestParticleGroup(const int PARTICLES_PER_SPRAY, Entity* Ent) {
	for (int i = 0; i < PARTICLES_PER_SPRAY; i++) {
		std::shared_ptr<Particle> deletedParticle = particleQueue.front();
		particleQueue.pop();
		deletedParticle->assignGroup(start, Ent, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
		unusedParticles.push_back(deletedParticle);
	}
}

