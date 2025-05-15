//
// sueda - geometry edits Z. Wood
// 3/16
//TODO: Add death effect to enemy
//TODO: Add effects to orbs

#include <iostream>
#include "Particle.h"
#include "../src/GLSL.h"
#include "../src/MatrixStack.h"
#include "../src/Program.h"
#include "../src/Texture.h"
#include "../src/Entity.h"
#include "../src/Config.h"

/**float randFloat(float l, float h)
{
	float r = rand() / (float) RAND_MAX;
	return (1.0f - r) * l + r * h;
}**/

Particle::Particle(vec3 start) :
	// initialization.. doesnt really matter
	charge(1.0f),
	m(1.0f),
	d(0.0f),
	x(start),
	v(0.0f, 0.0f, 0.0f),
	lifespan(10.0f),
	tEnd(0.0f),
	scale(1.0f),
	color(1.0f, 1.0f, 1.0f, 1.0f),
	startPos(start),
	attachedEntity(nullptr),
	camDist(1.0f)
{
}

Particle::~Particle()
{
}

void Particle::load(vec3 start, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high)
{
	// Random initialization
	rebirth(0.0f, start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
    // Make particle immediately available/"dead" after initial load by particleGen::gpuSetup()
    this->tEnd = 0.0f; 
}

/* all particles born at the origin */
void Particle::rebirth(float t, vec3 start, float r_low, float r_high, float g_low, 
	float g_high, float b_low, float b_high, float scale_low, float scale_high)
{
	charge = Config::randFloat(0.0f, 1.0f) < 0.5 ? -1.0f : 1.0f;
	m = 1.0f;
  	d = Config::randFloat(0.0f, 0.02f);
	x = startPos;
	v.x = Config::randFloat(-0.1f, 0.1f);
	v.y = Config::randFloat(-0.1f, 0.1f);
	v.z = Config::randFloat(-0.1f, 0.1f);
	lifespan = Config::randFloat(1.0f, 2.7f);
	tEnd = t + lifespan;
	scale = Config::randFloat(scale_low, scale_high);

	// Set color on rebirth
	color.r = Config::randFloat(r_low, r_high);
	color.g = Config::randFloat(g_low, g_high);
	color.b = Config::randFloat(b_low, b_high);
	color.a = 1.0f;
}

void Particle::update(float t, float h, const vec3 &g, const vec3 start, float r_low, float r_high, float g_low, 
	float g_high, float b_low, float b_high, float scale_low, float scale_high, float frameTime)
{
	// if(t > tEnd) { // Removed automatic rebirth
	// 	rebirth(t, start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
	// }
	//x = start;

	// Only update if particle is alive (current time t is less than its end time tEnd)
    if (t <= tEnd && lifespan > 0.0f) { // Check if particle is alive and lifespan is valid
        x += v * frameTime; // Update position based on current velocity and frameTime

        // Fade out alpha over lifetime
        float lifeProgress = (tEnd - t) / lifespan; // 얼마나 살았는지 (1.0 -> 0.0)
        color.a = glm::clamp(lifeProgress, 0.0f, 1.0f); // Alpha fades from 1 (full life) to 0 (dead)
    } else {
        color.a = 0.0f; // Particle is dead or has invalid lifespan, ensure its alpha is 0
    }

	// change velocity based on gravity
	//v = v + g * (tEnd - t);
	// x += v * vec3(scale* frameTime * 9.0); // This line seems to conflict with simple v * frameTime above. Consolidating.

}

void Particle::assignGroup(vec3 start, Entity* entity, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high)
{
	startPos = start;
	attachedEntity = entity;

	rebirth(0.0f, start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
}

void Particle::calcCamDist(mat4 view) {
	// Particle positions in camera space
	vec4 camSpaceVec = view * vec4(x.x, x.y, x.z, 1.0f);
	camDist = camSpaceVec.z;
}

void Particle::resize() {
}

// Implementation for the new launch method
void Particle::launch(float current_t, const glm::vec3& start_pos, const glm::vec3& initial_vel, float particle_lifespan, const glm::vec4& particle_color, float particle_scale)
{
    this->x = start_pos;        // Set position
    this->v = initial_vel;      // Set velocity
    this->lifespan = particle_lifespan;
    this->tEnd = current_t + particle_lifespan; // Set expiration time
    this->scale = particle_scale;   // Set scale
    this->color = particle_color;   // Set color

    // Default other properties if necessary, or ensure they don't interfere
    this->charge = 0.0f; // Fireballs are not charged
    this->m = 0.1f;      // Lighter mass for particles
    this->d = 0.01f;     // Minimal damping
    this->startPos = start_pos; // Update startPos as well, though less critical for launched particles not using rebirth immediately
}