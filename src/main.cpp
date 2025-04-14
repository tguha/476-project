/*
 * The start of our wizarding adventure
 */

#include <iostream>
#include <glad/glad.h>
#include <chrono>
#include <thread>
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "Spline.h"
#include "stb_image.h"
#include "AssimpModel.h"
#include "Animator.h"
#include "LightTrail.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define NUM_LIGHTS 4
#define MAX_BONES 200

class Collectible {
public:
	AssimpModel* model;
	glm::vec3 position;
	float scale;
	glm::vec3 AABBmin;
	glm::vec3 AABBmax;
	bool collected;

	Collectible(AssimpModel* model, const glm::vec3& position, const float scale)
		: model(model), position(position), scale(scale), collected(false)
	{
		// Get local bounding box from model
		glm::vec3 localMin = model->getBoundingBoxMin();
		glm::vec3 localMax = model->getBoundingBoxMax();

		// Apply scale
		localMin *= scale;
		localMax *= scale;

		// Offset by world position
		AABBmin = localMin + position;
		AABBmax = localMax + position;
	}
};

class Application : public EventCallbacks {

public:
	WindowManager * windowManager = nullptr;

	// Our shader programs
	std::shared_ptr<Program> texProg, prog2, assimptexProg;

	// ground data
	GLuint GrndBuffObj, GrndNorBuffObj, GIndxBuffObj;
	int g_GiboLen;
	GLuint GroundVertexArrayID;
	float groundSize = 20.0f;

	// setup collectivles vector
	std::vector<Collectible> collectibles;
	int collectedCount = 0;
	int totalCollectibles = 0;
	float finishTime;
	bool reset = false;
	// character bounding box
	glm::vec3 manAABBmin, manAABBmax;

	AssimpModel *cube, *barrel, *alien;

	AssimpModel *stickfigure_running, *stickfigure_standing;
	Animation *stickfigure_anim, *stickfigure_idle;
	Animator *stickfigure_animator;

	float AnimDeltaTime = 0.0f;
	float AnimLastFrame = 0.0f;

	vec3 gMin;

	float lightTrans = -2;
	int change_mat = 0;

	//animation data
	float sTransx = -2.5;
	float sTransy = -1.3;
	float vTransz = 0;
	float vThetax = 0.0f;
	float vThetay = 0.0f;
	float vTransx = 2;
	float vTransy = 0;

	// vec3 manTrans = vec3(-2.5, -1.3, 0);
	vec3 manTrans = vec3(0, 0, 0);
	vec3 manScale = vec3(0.01, 0.01, 0.01);
	vec3 manRot = vec3(radians(0.0f), radians(0.0f), radians(0.0f));

	vec3 manMoveDir = vec3(sin(manRot.y), 0, cos(manRot.y));

	// initial position of light cycles
	vec3 start_lightcycle1_pos = vec3(-384, -11, 31);
	vec3 start_lightcycle2_pos = vec3(-365, -11, 9.1);


	float theta = 0.0f; // controls yaw
	// float theta = radians(90.0f); // controls yaw
	// float phi = 0.0f; // controls pitch
	float phi = radians(-30.0f); // controls pitch

	float radius = 5.0f;

	float wasd_sens = 0.5f;

	vec3 eye = vec3(-6, 1.03, 0);
	// vec3 lookAt = vec3(-1.58614, -0.9738, 0.0436656);
	vec3 lookAt = manTrans;
	vec3 up = vec3(0, 1, 0);

	bool mouseIntialized = false;
	double lastX, lastY;


	int debug = 0;
	int debug_pos = 0;

	enum Man_State {
		WALKING,
		STANDING,
	};

	Man_State manState = STANDING;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
			// eye = eye + wasd_sens * normalize(lookAt - eye); // w vector
			// lookAt = lookAt + wasd_sens * normalize(lookAt - eye); // w vector
			// manTrans.x += 0.2f;
			// eye.x += 0.2f;
			manTrans += manMoveDir * 0.2f;
			eye += manMoveDir * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			// cout << "pressing W" << endl;


			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			manState = STANDING;

			// cout << "releasing W" << endl;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
			// eye = eye - wasd_sens * normalize(lookAt - eye); // w vector
			// lookAt = lookAt - wasd_sens * normalize(lookAt - eye); // w vector
			// manTrans.x -= 0.2f;
			// eye.x -= 0.2f;
			manTrans -= manMoveDir * 0.2f;
			eye -= manMoveDir * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
			// eye = eye - wasd_sens * normalize(cross(lookAt - eye, up)); // u vector
			// lookAt = lookAt - wasd_sens * normalize(cross(lookAt - eye, up)); // u vector

			// manTrans.z -= 0.2f;
			// eye.z -= 0.2f;
			vec3 right = normalize(cross(manMoveDir, up));
			manTrans -= right * 0.2f;
			eye -= right * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
			// eye = eye + wasd_sens * normalize(cross(lookAt - eye, up)); // u vector
			// lookAt = lookAt + wasd_sens * normalize(cross(lookAt - eye, up)); // u vector

			// manTrans.z += 0.2f;
			// eye.z += 0.2f;
			vec3 right = normalize(cross(manMoveDir, up));
			manTrans += right * 0.2f;
			eye += right * 0.2f;
			lookAt = manTrans;


			manState = WALKING;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			manState = STANDING;
		}
		if (glfwGetKey(window, GLFW_KEY_Q)){
			lightTrans += 1.0;
		}
		if (glfwGetKey(window, GLFW_KEY_E)){
			lightTrans -= 1.0;
		}

		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
	{

			float sensitivity = 0.7f;

			theta = theta + deltaX * sensitivity;

			phi = phi - deltaY * sensitivity;

			// if (phi > radians(-10.0f))
			// {
			// 	phi = radians(-10.0f);
			// }
			if (phi > radians(80.0f))
			{
				phi = radians(80.0f);
			}
			if (phi < radians(-80.0f))
			{
				phi = radians(-80.0f);
			}

			updateCameraVectors();
	}

	void mouseMoveCallback(GLFWwindow* window, double xpos, double ypos) {
		if (!mouseIntialized) {
			lastX = xpos;
			lastY = ypos;
			mouseIntialized = true;
			return;
		}

		float deltaX = xpos - lastX;
		float deltaY = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float mouseSensitivity = 0.005f;

		theta = theta + deltaX * mouseSensitivity;
		phi = phi + deltaY * mouseSensitivity;
		if (phi > radians(-10.0f))
		{
			phi = radians(-10.0f);
		}
		if (phi < radians(-80.0f))
		{
			phi = radians(-80.0f);
		}

		updateCameraVectors();
	}

	void updateCameraVectors() {
		vec3 front;
		front.x = radius * cos(phi) * cos(theta);
		front.y = radius * sin(phi);
		front.z = radius * cos(phi) * cos((pi<float>()/2) - theta);

		eye = manTrans - front;
		lookAt = manTrans;

		manRot.y = theta + radians(-90.0f);
		manRot.y = - manRot.y;
		manRot.x = phi;

		// cout << "Theta: " << theta << " Phi: " << phi << endl;

		manMoveDir = vec3(sin(manRot.y), 0, cos(manRot.y));

		// lookAt = eye + front;


	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &posX, &posY);
			 cout << "Pos X " << posX << " Pos Y " << posY << endl;
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}


	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();
		
		// Set background color and enable z-buffer test
		glClearColor(.12f, .34f, .56f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatAmb");
		texProg->addUniform("MatSpec");
		texProg->addUniform("MatShine");
		texProg->addUniform("numLights");
		for (int i = 0; i < NUM_LIGHTS; i++) {
			texProg->addUniform("lightPos[" + to_string(i) + "]");
			texProg->addUniform("lightColor[" + to_string(i) + "]");
			texProg->addUniform("lightIntensity[" + to_string(i) + "]");
		}
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		// Initialize the GLSL program that we will use for rendering
		prog2 = make_shared<Program>();
		prog2->setVerbose(true);
		prog2->setShaderNames(resourceDirectory + "/simple_light_vert.glsl", resourceDirectory + "/simple_light_frag.glsl");
		prog2->init();
		prog2->addUniform("P");
		prog2->addUniform("V");
		prog2->addUniform("M");
		prog2->addUniform("MatAmb");
		prog2->addAttribute("vertPos");
		prog2->addAttribute("vertNor");
		prog2->addUniform("MatDif");
		prog2->addUniform("MatSpec");
		prog2->addUniform("MatShine");
		for (int i = 0; i < NUM_LIGHTS; i++) {
			prog2->addUniform("lightPos[" + to_string(i) + "]");
			prog2->addUniform("lightColor[" + to_string(i) + "]");
			prog2->addUniform("lightIntensity[" + to_string(i) + "]");
		}
		prog2->addUniform("numLights");
		prog2->addUniform("hasEmittance");
		prog2->addUniform("MatEmitt");
		prog2->addUniform("MatEmittIntensity");
		prog2->addUniform("discardCounter");
		prog2->addUniform("activateDiscard");
		prog2->addUniform("randFloat1");
		prog2->addUniform("randFloat2");
		prog2->addUniform("randFloat3");
		prog2->addUniform("randFloat4");

		// Initialize the GLSL program that we will use for assimp models
		assimptexProg = make_shared<Program>();
		assimptexProg->setVerbose(true);
		assimptexProg->setShaderNames(resourceDirectory + "/assimp_tex_vert.glsl", resourceDirectory + "/assimp_tex_frag.glsl");
		assimptexProg->init();
		assimptexProg->addUniform("P");
		assimptexProg->addUniform("V");
		assimptexProg->addUniform("M");
		assimptexProg->addUniform("texture_diffuse1");
		assimptexProg->addUniform("texture_specular1");
		assimptexProg->addUniform("texture_roughness1");
		assimptexProg->addUniform("texture_metalness1");
		assimptexProg->addUniform("texture_emission1");
		assimptexProg->addAttribute("vertPos");
		assimptexProg->addAttribute("vertNor");
		assimptexProg->addAttribute("vertTex");
		assimptexProg->addAttribute("boneIds");
		assimptexProg->addAttribute("weights");
		for (int i = 0; i < MAX_BONES; i++) {
			assimptexProg->addUniform("finalBonesMatrices[" + to_string(i) + "]");
		}
		assimptexProg->addUniform("MatAmb");
		assimptexProg->addUniform("MatDif");
		assimptexProg->addUniform("MatSpec");
		assimptexProg->addUniform("MatShine");
		for (int i = 0; i < NUM_LIGHTS; i++) {
			assimptexProg->addUniform("lightPos[" + to_string(i) + "]");
			assimptexProg->addUniform("lightColor[" + to_string(i) + "]");
			assimptexProg->addUniform("lightIntensity[" + to_string(i) + "]");
		}
		assimptexProg->addUniform("numLights");
		assimptexProg->addUniform("hasTexture");
		updateCameraVectors();
	}

	void initGeom(const std::string& resourceDirectory)
	{
 		string errStr;

		// load the walking character model
		stickfigure_running = new AssimpModel(resourceDirectory + "/Vanguard/Vanguard.fbx");
		stickfigure_anim = new Animation(resourceDirectory + "/Vanguard/Vanguard.fbx", stickfigure_running, 0);
		stickfigure_idle = new Animation(resourceDirectory + "/Vanguard/Vanguard.fbx", stickfigure_running, 1);
		stickfigure_animator = new Animator(stickfigure_anim);

		// load the cube
		cube = new AssimpModel(resourceDirectory + "/cube.obj");

		// load the barrel
		barrel = new AssimpModel(resourceDirectory + "/Barrel/Barrel_OBJ.obj");
		// manually assign the barrel texture
		// this is happening because the barrel does not have any embedded textures
		// we could import to blender and then embed the textures as a remedy
		barrel->assignTexture("texture_diffuse1", resourceDirectory + "/Barrel/textures/barrel_diffuse.png");
		barrel->assignTexture("texture_roughness1", resourceDirectory + "/Barrel/textures/barrel_roughness.png");
		barrel->assignTexture("texture_metalness1", resourceDirectory + "/Barrel/textures/barrel_metallic.png");
		barrel->assignTexture("texture_normal1", resourceDirectory + "/Barrel/textures/barrel_normal.png");

		alien = new AssimpModel(resourceDirectory + "/Alien/Alien_OBJ.obj");
		alien->assignTexture("texture_diffuse1", resourceDirectory + "/Alien/textures/alien.jpg");

		// example debug for checking mesh count of a model, helps w multimesh and sanity checks
		/*std::cout << "Barrel model has " << barrel->getMeshCount() << " meshes" << std::endl;
		for (size_t i = 0; i < barrel->getMeshCount(); i++) {
			std::cout << "  Mesh " << i << " has " << barrel->getMeshSize(i) << " vertices" << std::endl;
		}*/

		// add 2 instances of the barrel to the collectibles vector Collectible(<model>, <position>, <scale>)
		collectibles.push_back(Collectible(barrel, vec3(3.0f, 0.0f, 1.0f), 1.0f));
		collectibles.push_back(Collectible(barrel, vec3(-2.0f, 0.0f, 2.0f), 1.0f));

		collectibles.push_back(Collectible(alien, vec3(-2.0f, -0.2f, -2.0f), 0.1f));

		// update total collectibles
		totalCollectibles = collectibles.size();
	}

	void SetMaterialMan(shared_ptr<Program> curS, int i) {
		switch (i) {
			case 0:
			// gold
				glUniform3f(curS->getUniform("MatAmb"), 0.24725f, 0.1995f, 0.0745f);
				glUniform3f(curS->getUniform("MatDif"), 0.75164f, 0.60648f, 0.22648f);
				glUniform3f(curS->getUniform("MatSpec"), 0.628281f, 0.555802f, 0.366065f);
				glUniform1f(curS->getUniform("MatShine"), 51.2f);
			break;
			case 1:
			// silver
				glUniform3f(curS->getUniform("MatAmb"), 0.19225f, 0.19225f, 0.19225f);
				glUniform3f(curS->getUniform("MatDif"), 0.50754f, 0.50754f, 0.50754f);
				glUniform3f(curS->getUniform("MatSpec"), 0.508273f, 0.508273f, 0.508273f);
				glUniform1f(curS->getUniform("MatShine"), 51.2f);
			break;
			case 2:
			// bronze
				glUniform3f(curS->getUniform("MatAmb"), 0.2125f, 0.1275f, 0.054f);
				glUniform3f(curS->getUniform("MatDif"), 0.714f, 0.4284f, 0.18144f);
				glUniform3f(curS->getUniform("MatSpec"), 0.393548f, 0.271906f, 0.166721f);
				glUniform1f(curS->getUniform("MatShine"), 25.6f);
			break;
			case 3:
			// black
				glUniform3f(curS->getUniform("MatAmb"), 0.01f, 0.01f, 0.01f);
				glUniform3f(curS->getUniform("MatDif"), 0.07f, 0.07f, 0.07f);
				glUniform3f(curS->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
			break;
			case 4:
			// dark white
				glUniform3f(curS->getUniform("MatAmb"), 0.05f, 0.05f, 0.05f);
				glUniform3f(curS->getUniform("MatDif"), 0.5f, 0.5f, 0.5f);
				glUniform3f(curS->getUniform("MatSpec"), 0.7f, 0.7f, 0.7f);
				glUniform1f(curS->getUniform("MatShine"), 10.0f);
			break;
		}
	}

	/* helper for sending top of the matrix strack to GPU */
	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
    }

	/* helper function to set model trasnforms */
  	void setModel(shared_ptr<Program> curS, vec3 trans, float rotY, float rotX, float sc) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void updateBoundingBox(const glm::vec3& localMin, const glm::vec3& localMax, const glm::mat4& transform, glm::vec3& outWorldMin, glm::vec3& outWorldMax) {
		// Initialize with extreme values
		outWorldMin = glm::vec3(std::numeric_limits<float>::max());
		outWorldMax = glm::vec3(-std::numeric_limits<float>::max());

		// Get the 8 corners of the bounding box
		glm::vec3 corners[8] = {
			{localMin.x, localMin.y, localMin.z},
			{localMax.x, localMin.y, localMin.z},
			{localMin.x, localMax.y, localMin.z},
			{localMax.x, localMax.y, localMin.z},
			{localMin.x, localMin.y, localMax.z},
			{localMax.x, localMin.y, localMax.z},
			{localMin.x, localMax.y, localMax.z},
			{localMax.x, localMax.y, localMax.z}
		};

		// Transform corners and update min/max
		for (int i = 0; i < 8; ++i) {
			glm::vec4 transformed = transform * glm::vec4(corners[i], 1.0f);
			outWorldMin = glm::min(outWorldMin, glm::vec3(transformed));
			outWorldMax = glm::max(outWorldMax, glm::vec3(transformed));
		}
	}

	//directly pass quad for the ground to the GPU
	void initGround() {
		float g_groundSize = groundSize;
		float g_groundY = 0.0f;
		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};
		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};
		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };

		// Generate the ground VAO
		glGenVertexArrays(1, &GroundVertexArrayID);
		glBindVertexArray(GroundVertexArrayID);

		g_GiboLen = 6;
		glGenBuffers(1, &GrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

		glGenBuffers(1, &GrndNorBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

		glGenBuffers(1, &GIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
	}

    //code to draw the ground plane
	void drawGround(shared_ptr<Program> curS, std::shared_ptr<MatrixStack> Model) {
		curS->bind();
		glBindVertexArray(GroundVertexArrayID);

		// Set material for ground
		SetMaterialMan(curS, 1);

		// Use the matrix stack for the model matrix
		Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(vec3(0, 0, 0));
		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Draw
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		Model->popMatrix();
		curS->unbind();
	}

	float randFloat(float l, float h) {
		float r = rand() / (float) RAND_MAX;
		return (1.0f - r) * l + r * h;
	}

	bool checkAABBCollision(const glm::vec3& minA, const glm::vec3& maxA,
		const glm::vec3& minB, const glm::vec3& maxB)
	{
		return (minA.x <= maxB.x && maxA.x >= minB.x) &&
			(minA.y <= maxB.y && maxA.y >= minB.y) &&
			(minA.z <= maxB.z && maxA.z >= minB.z);
	}

	void resetCollectibles() {
		for (auto& collectible : collectibles) {
			collectible.collected = false;
		}
		collectedCount = 0;
		std::cout << "Game reset. Find all the collectibles again!" << std::endl;
	}

	void render(float frametime, float animTime) {
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float aspect = width/(float)height;

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		// Apply perspective projection
		Projection->pushMatrix();

		// Projection->perspective(45.0f, aspect, 0.01f, 200.0f);
		Projection->perspective(45.0f, aspect, 0.01f, 400.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
		View->loadIdentity();
		View->lookAt(eye, lookAt, vec3(0, 1, 0));

		// Draw the ground
		prog2->bind();
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog2->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
		glUniform1f(prog2->getUniform("lightIntensity[0]"), 1.0); // light intensity
		glUniform3f(prog2->getUniform("lightPos[0]"), 0, 2, 0); // light position at the computer screen
		glUniform1i(prog2->getUniform("numLights"), 1); // light position at the computer screen
		drawGround(prog2, Model);
		prog2->unbind();

		assimptexProg->bind();
		glUniformMatrix4fv(assimptexProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(assimptexProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(assimptexProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
		glUniform1f(assimptexProg->getUniform("lightIntensity[0]"), 0.0); // light intensity
		glUniform3f(assimptexProg->getUniform("lightPos[0]"), 0, 10, 0); // light position at the computer screen
		glUniform1i(assimptexProg->getUniform("numLights"), 1); // light position at the computer screen

		// select animation for vanguard model
		stickfigure_animator->UpdateAnimation(1.5 * animTime);
		if (manState == WALKING) {
			stickfigure_animator->SetCurrentAnimation(stickfigure_anim);
		} else if (manState == STANDING) {
			stickfigure_animator->SetCurrentAnimation(stickfigure_idle);
		}

		// update the bone matrices according to selected animation
		vector<glm::mat4> transforms = stickfigure_animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i) {
			glUniformMatrix4fv(assimptexProg->getUniform("finalBonesMatrices[" + std::to_string(i) + "]"), 1, GL_FALSE, value_ptr(transforms[i]));
		}

		// set the model matrix and draw the walking character model
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(manTrans);
			Model->scale(0.01f);
			Model->rotate(manRot.y, vec3(0, 1, 0));
			Model->rotate(manRot.z, vec3(0, 0, 1));

			// update the bounding box for collision detection
			glm::mat4 manTransform = glm::translate(glm::mat4(1.0f), manTrans)
				* glm::rotate(glm::mat4(1.0f), manRot.x, glm::vec3(1, 0, 0))
				* glm::rotate(glm::mat4(1.0f), manRot.y, glm::vec3(0, 1, 0))
				* glm::scale(glm::mat4(1.0f), manScale);
			updateBoundingBox(stickfigure_running->getBoundingBoxMin(),
				stickfigure_running->getBoundingBoxMax(),
				manTransform,
				manAABBmin,
				manAABBmax);

			glUniform1i(assimptexProg->getUniform("hasTexture"), 1);
			SetMaterialMan(assimptexProg, 0);
			setModel(assimptexProg, Model);
			stickfigure_running->Draw(assimptexProg);
		Model->popMatrix();

		assimptexProg->unbind();

		// Draw the collectibles with the simple texture shader
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));

		// Set lighting uniforms
		glUniform3f(texProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // White light
		glUniform1f(texProg->getUniform("lightIntensity[0]"), 5.0); // High intensity for visibility
		glUniform3f(texProg->getUniform("lightPos[0]"), 0, 2, 0);
		glUniform1i(texProg->getUniform("numLights"), 1);

		// Set material properties
		glUniform3f(texProg->getUniform("MatAmb"), 0.5, 0.5, 0.5); // Bright ambient
		glUniform3f(texProg->getUniform("MatSpec"), 0.8, 0.8, 0.8); // Strong specular
		glUniform1f(texProg->getUniform("MatShine"), 32.0f); // High shininess

		// Check for collisions with collectibles
		for (auto& collectible : collectibles) {
			if (collectedCount >= totalCollectibles && !reset) {
				std::cout << "All items collected! Resetting Game.\n";
				finishTime = glfwGetTime();
				reset = true;
			} else if (collectedCount >= totalCollectibles && (glfwGetTime() - finishTime >= 1.0f)) {
				resetCollectibles();
				reset = false;
			}
			if (!collectible.collected &&
				checkAABBCollision(manAABBmin, manAABBmax, collectible.AABBmin, collectible.AABBmax)) {
				collectible.collected = true;
				collectedCount++;
				std::cout << "Collected an item! (" << collectedCount << "/" << totalCollectibles << ")\n";
			}
		}

		// Draw collectibles
		for (auto& collectible : collectibles) {
			// Skip drawing if collected
			if (collectible.collected) continue;

			Model->pushMatrix();
				Model->loadIdentity();
				Model->translate(collectible.position);
				Model->scale(collectible.scale);
				setModel(texProg, Model);
				glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

				// Bind the diffuse texture to texture unit 0
				glActiveTexture(GL_TEXTURE0);

				// draw the collectible
				collectible.model->Draw(texProg);
			Model->popMatrix();
		}

		// example of drawing a barrel
		//Model->pushMatrix();
		//	Model->loadIdentity();

		//	// Position barrel
		//	vec3 barrelPos = glm::vec3(0, 0, -5);
		//	Model->translate(barrelPos);

		//	// Scale the barrel
		//	Model->scale(vec3(1.0f));

		//	glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));

		//	// Bind the diffuse texture to texture unit 0
		//	glActiveTexture(GL_TEXTURE0);

		//	// draw the barrel
		//	barrel->Draw(texProg);
		//Model->popMatrix();

		texProg->unbind();

		// Pop matrix stacks
		Projection->popMatrix();
		View->popMatrix();
	}
};

void mouseMoveCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	app->mouseMoveCallback(window, xpos, ypos);
}

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	// Your main will always include a similar set up to establish your window
	// and GL context, etc

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	glfwSetWindowUserPointer(windowManager->getHandle(), application);
	glfwSetCursorPosCallback(windowManager->getHandle(), mouseMoveCallbackWrapper);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initGround();

	auto lastTime = chrono::high_resolution_clock::now();

	glfwSetInputMode(windowManager->getHandle(), GLFW_STICKY_KEYS, GLFW_TRUE);

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		auto nextLastTIme = chrono::high_resolution_clock::now();

		float deltaTime = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - lastTime).count();

		deltaTime *= 0.000001f; // convert to seconds

		lastTime = nextLastTIme;

		float AnimcurrFrame = glfwGetTime();
		application->AnimDeltaTime = AnimcurrFrame - application->AnimLastFrame;
		application->AnimLastFrame = AnimcurrFrame;
		// Render scene.
		application->render(deltaTime, application->AnimDeltaTime);

		// Swap front and back buffers
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events
		glfwPollEvents();
	}

	// Quit program
	windowManager->shutdown();
	return 0;
}
