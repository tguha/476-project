//
// sueda
// November, 2014/ wood 16
//

#pragma once

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