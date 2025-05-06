//
// sueda - geometry edits Z. Wood
// 3/16
//

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
	if(t > tEnd) {
		rebirth(t, start, r_low, r_high, g_low, g_high, b_low, b_high, scale_low, scale_high);
	}
	//x = start;

	// change velocity based on gravity
	//v = v + g * (tEnd - t);
	x += v * vec3(scale* frameTime * 9.0);

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