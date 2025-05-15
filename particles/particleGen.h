#pragma once
#ifndef __particleS__
#define __particleS__

#include <glm/glm.hpp>
#include <vector>
#include "Particle.h"
#include "../src/Program.h"
#include "../src/Entity.h"
#include <queue>

using namespace glm;
using namespace std;
const int MAX_PARTICLES = 1000;

class ParticleSorter {
public:
   bool operator()(const shared_ptr<Particle> p0, const shared_ptr<Particle> p1) const
   {
      // Particle positions in world space
      const vec3 &x0 = p0->getPosition();
      const vec3 &x1 = p1->getPosition();
      // Particle positions in camera space
      vec4 x0w = C * vec4(x0.x, x0.y, x0.z, 1.0f);
      vec4 x1w = C * vec4(x1.x, x1.y, x1.z, 1.0f);
      // Sort by camera space Z (farther particles first)
      return x0w.z > x1w.z;
   }
  
   mat4 C; // current camera matrix
};

class particleGen {
private:
	vector<shared_ptr<Particle>> particles;
	vector<shared_ptr<Particle>> unusedParticles;
	queue<shared_ptr<Particle>> particleQueue;
	float t, h; //?
	vec3 g; //gravity
	vec3 start;
	ParticleSorter sorter;
	//int numP;
	//GLfloat *points;
	//GLfloat *pointColors;
	int numP = MAX_PARTICLES;
	GLfloat points[MAX_PARTICLES*3];
	GLfloat pointColors[MAX_PARTICLES * 4];
	GLfloat pointScales[MAX_PARTICLES];
	
	mat4 theCamera;
	unsigned vertArrObj;		// VAO: contains both buffer
	unsigned vertBuffObj;
	unsigned colorBuffObj;
	unsigned scaleBuffObj;

	float r_low;
	float r_high;
	float g_low;
	float g_high;
	float b_low;
	float b_high;
	float scale_low;
	float scale_high;
	
public:
	particleGen(vec3 source, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high);
	void drawMe(std::shared_ptr<Program> prog);
	void gpuSetup();
	void update(float frameTime);
	void reSet();
	void setCamera(mat4 inC) {theCamera = inC;}
	void setStart(vec3 setStart) { start = setStart; }
	void setnumP(int numParticles) 
	{ 
		numP = numParticles;
	}
	float getCurrentTime() const { return t; }
	void initParticleGroup(int PARTICLES_PER_SPRAY, vec3 playerPos, Entity* Ent);
	void deleteOldestParticleGroup(const int PARTICLES_PER_SPRAY, Entity* Ent);

	// Method to spawn a burst of particles for effects like fireballs
    void spawnParticleBurst(const glm::vec3& position, 
                            const glm::vec3& base_direction, 
                            int count, 
                            float current_time, 
                            float speed_min, 
                            float speed_max, 
                            float spread, 
                            float p_lifespan_min, float p_lifespan_max,
                            const glm::vec4& p_color_start, 
                            const glm::vec4& p_color_end, 
                            float p_scale_min, float p_scale_max);
};


#endif