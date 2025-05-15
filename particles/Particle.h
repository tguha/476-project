//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../src/Entity.h"

using namespace glm;

class MatrixStack;
class Program;
class Texture;
class Entity;

// External declaration of randFloat
extern float randFloat(float l, float h);

class Particle
{
public:
	Particle(vec3 pos);
	virtual ~Particle();
	void load(vec3 start, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high);
	void rebirth(float t, vec3 start, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high);
	void update(float t, float h, const glm::vec3 &g, const vec3 start,
				float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high, float frameTime);
	const vec3 &getPosition() const { return x; };
	const vec3 &getVelocity() const { return v; };
	const vec4 &getColor() const { return color; };
	float getTEnd() const { return tEnd; };
	void setTEnd(float time) { tEnd = time; }
	float getScale() const { return scale; }
	void launch(float current_t, const glm::vec3& start_pos, const glm::vec3& initial_vel, float particle_lifespan, const glm::vec4& particle_color, float particle_scale);
	void assignGroup(vec3 start, Entity* entity, float r_low, float r_high, float g_low, float g_high, float b_low, float b_high, float scale_low, float scale_high);
	void calcCamDist(mat4 view);
	void resize();
private:
	float charge; // +1 or -1
	float m; // mass
	float d; // viscous damping
	vec3 x; // position
	vec3 v; // velocity
	float lifespan; // how long this particle lives
	float tEnd;     // time this particle dies
	float scale;
	vec4 color;
	vec3 startPos;
	Entity* attachedEntity;
	float camDist;
};

#endif // LAB471_PARTICLE_H_INCLUDED