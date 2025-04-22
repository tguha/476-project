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
#include "Enemy.h"

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

#define NUM_LIGHTS 4
#define MAX_BONES 200

float randFloat(float l, float h) {
	float r = rand() / (float)RAND_MAX;
	return (1.0f - r) * l + r * h;
}

// Enum for book states
enum class BookState {
	ON_SHELF,
	FALLING,
	LANDED,
	OPENING,
	OPENED
};

enum class OrbState {
	SPAWNING,  // Initial state right after creation
	LEVITATING,// Moving upwards
	IDLE,      // Stationary, ready for collection
	COLLECTED // Visually attached to player (handled by drawing logic)
};

class Book {
public:
	vec3 initialPosition; // Where the book starts
	vec3 position;        // Current position (updated by spline or stays initial)
	vec3 scale;           // Base scale for the book
	quat orientation;     // Initial orientation (using quaternion is often easier for complex rotations)
	// Alternatively, use glm::vec3 for Euler angles if you prefer

	BookState state = BookState::ON_SHELF;
	Spline* fallSpline = nullptr; // Pointer to the spline for falling animation
	float fallStartTime = 0.0f;   // Time the fall started

	float openAngle = 0.0f;       // Current angle for opening animation (radians)
	float maxOpenAngle = glm::radians(80.0f); // How far the book opens
	float openSpeed = glm::radians(120.0f); // Speed of opening in radians per second

	AssimpModel* bookModel; // Pointer to the cube model
	AssimpModel* orbModel;  // Pointer to the sphere model

	vec3 orbColor;
	float orbScale = 0.1f; // Scale of the spell orb
	bool orbSpawned = false;

	// Constructor
	Book(AssimpModel* bookMdl, AssimpModel* orbMdl, const glm::vec3& pos, const glm::vec3& scl, const glm::quat& orient, const glm::vec3& orbClr)
		: initialPosition(pos), position(pos), scale(scl), orientation(orient),
		bookModel(bookMdl), orbModel(orbMdl), orbColor(orbClr) {
	}

	// Destructor to clean up spline if needed
	~Book() {
		delete fallSpline;
	}

	// Method to start the fall
	void startFalling(float groundY) {
		if (state == BookState::ON_SHELF) {
			state = BookState::FALLING;
			fallStartTime = glfwGetTime();

			glm::vec3 endPosition = position;
			endPosition.y = groundY + (scale.y * 0.5f); // Land flat on the ground based on scale

			// Add some randomness to landing spot and arc
			float offsetX = randFloat(-0.5f, 0.5f);
			float offsetZ = randFloat(-0.5f, 0.5f);
			endPosition.x += offsetX;
			endPosition.z += offsetZ;

			// Simple control point for a basic arc
			glm::vec3 controlPoint = (initialPosition + endPosition) * 0.5f; // Midpoint
			controlPoint.y += 2.0f; // Arc upwards
			controlPoint.x += randFloat(-1.0f, 1.0f); // Random horizontal arc deviation

			// Create spline (quadratic example, cubic needs another control point)
			delete fallSpline; // Delete old one if any (shouldn't happen in this flow)
			fallSpline = new Spline(initialPosition, controlPoint, endPosition, 0.25f); // 0.25 second fall duration
		}
	}

	// Method to update the book's state and position
	void update(float deltaTime, float groundY) {
		switch (state) {
		case BookState::FALLING:
			if (fallSpline) {
				fallSpline->update(deltaTime);
				position = fallSpline->getPosition();
				// Optional: Add rotation during fall here

				if (fallSpline->isDone()) {
					state = BookState::LANDED; // Transition to landed state
					position.y = groundY + (scale.y * 0.5f); // Ensure it's exactly on the ground
					delete fallSpline;
					fallSpline = nullptr;
					// Set orientation flat on the ground if needed
					orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Reset orientation or calculate landing angle
				}
			}
			break;

		case BookState::LANDED:
			// Optional delay before opening?
			state = BookState::OPENING; // Immediately start opening after landing
			break;

		case BookState::OPENING:
			openAngle += openSpeed * deltaTime;
			if (openAngle >= maxOpenAngle) {
				openAngle = maxOpenAngle;
				state = BookState::OPENED;
			}
			break;

		case BookState::OPENED:
			// Book remains open, orb is visible
			break;

		case BookState::ON_SHELF:
		default:
			// Do nothing
			break;
		}
	}
};

class Collectible {
public:
	AssimpModel* model; // Will always be the sphere model
	glm::vec3 position; // CURRENT position (animated during levitation)
	float scale;
	glm::vec3 AABBmin;
	glm::vec3 AABBmax;
	bool collected;
	glm::vec3 color;

	// --- Levitation Members ---
	OrbState state = OrbState::SPAWNING; // Start in SPAWNING state
	glm::vec3 spawnPosition;             // Where the orb initially appears
	glm::vec3 idlePosition;              // Target position after levitating
	float levitationHeight = 0.6f;       // How far up it moves
	float levitationStartTime = 0.0f;
	float levitationDuration = 0.75f;     // Duration of the levitation animation (seconds)


	// Updated constructor for Orbs
	Collectible(AssimpModel* model, const glm::vec3& spawnPos, const float scale, const glm::vec3& clr)
		: model(model),
		position(spawnPos), // Initial position is spawn position
		scale(scale),
		collected(false),
		color(clr),
		state(OrbState::LEVITATING), // Immediately start levitating after spawn
		spawnPosition(spawnPos)
	{
		// Calculate target idle position
		idlePosition = spawnPosition + glm::vec3(0.0f, levitationHeight, 0.0f);

		// Record start time for animation
		levitationStartTime = glfwGetTime();

		// Initial AABB calculation (based on spawn position initially)
		updateAABB(); // Use a helper function for AABB updates
	}

	// Helper function to update AABB based on current position
	void updateAABB() {
		glm::vec3 localMin = model->getBoundingBoxMin();
		glm::vec3 localMax = model->getBoundingBoxMax();
		localMin *= scale;
		localMax *= scale;
		AABBmin = localMin + position; // Use current position
		AABBmax = localMax + position; // Use current position
	}

	// Function to update levitation animation
	void updateLevitation(float currentTime) {
		if (state == OrbState::LEVITATING) {
			float elapsedTime = currentTime - levitationStartTime;
			float t = glm::clamp(elapsedTime / levitationDuration, 0.0f, 1.0f);

			// Apply an easing function for smoother start/end (optional)
			// t = glm::sineEaseInOut(t); // Example using easing functions (requires #include <glm/gtx/easing.hpp>)
			t = t * t * (3.0f - 2.0f * t); // Manual smoothstep calculation

			// Interpolate position
			position = glm::mix(spawnPosition, idlePosition, t);

			// Update AABB as the orb moves
			updateAABB();

			// Check if animation finished
			if (t >= 1.0f) {
				state = OrbState::IDLE;
				position = idlePosition; // Ensure it's exactly at the target
				updateAABB();           // Final AABB update
				// std::cout << "Orb reached IDLE state." << std::endl; // Debug output
			}
		}
	}
};

class Application : public EventCallbacks {

public:
	WindowManager * windowManager = nullptr;
	
	bool windowMaximized = false;
	int window_width = 640;
	int window_height = 480;

	// Our shader programs
	std::shared_ptr<Program> texProg, prog2, assimptexProg;

	// ground data
	GLuint GrndBuffObj, GrndNorBuffObj, GIndxBuffObj;
	int g_GiboLen;
	GLuint GroundVertexArrayID;
	float groundSize = 20.0f;

	// setup collectibles vector
	std::vector<Collectible> orbCollectibles;
	int orbsCollectedCount = 0;
	std::vector<Enemy*> enemies;

	// character bounding box
	glm::vec3 manAABBmin, manAABBmax;

	AssimpModel *cube, *barrel, *creeper, *alien, *wizard_hat, *fish, *cylinder, *sphere;

	//  vector of books
	vector<Book> books;

	AssimpModel *stickfigure_running, *stickfigure_standing;
	Animation *stickfigure_anim, *stickfigure_idle;
	Animator *stickfigure_animator;

	float AnimDeltaTime = 0.0f;
	float AnimLastFrame = 0.0f;

	int change_mat = 0;

	vec3 characterMovement = vec3(0, 0, 0);
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
	vec3 lookAt = characterMovement;
	vec3 up = vec3(0, 1, 0);

	vec3 right = normalize(cross(manMoveDir, up));

	bool mouseIntialized = false;
	double lastX, lastY;


	int debug = 0;
	int debug_pos = 0;

	bool cursor_visable = true;

	enum Man_State {
		WALKING,
		STANDING,
	};

	//Movement Variables (Maybe move?)
	bool movingForward = false;
	bool movingBackward = false;
	bool movingLeft = false;
	bool movingRight = false;
	
	float characterRotation = 0.0f;

	Man_State manState = STANDING;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
		{
			//Fullscreen Mode
			if (!windowMaximized) {
				glfwMaximizeWindow(window);
				windowMaximized = !windowMaximized;
			}
			else {
				glfwRestoreWindow(window);
				windowMaximized = !windowMaximized;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
			manState = WALKING;

			//Movement Variable
			movingForward = true;
			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			manState = STANDING;
			//Movement Variable
			movingForward = false;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
			manState = WALKING;

			//Movement Variable
			movingBackward = true;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			manState = STANDING;
			//Movement Variable
			movingBackward = false;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
			manState = WALKING;

			//Movement Variable
			movingLeft = true;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}

		} else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			manState = STANDING;
			//Movement Variable
			movingLeft = false;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
			manState = WALKING;
			
			//Movement Variable
			movingRight = true;

			if (debug_pos) {
				cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
				cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
			}
		} else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
			manState = STANDING;
			//Movement Variable
			movingRight = false;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) { // Interaction Key
			interactWithBooks();
    }
		if (key == GLFW_KEY_L && action == GLFW_PRESS){
			cursor_visable = !cursor_visable;
			if (cursor_visable) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
	}

	void scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
	{

			float sensitivity = 0.7f;

			theta = theta + deltaX * sensitivity;

			phi = phi - deltaY * sensitivity;

			if (phi > radians(-10.0f))
			{
				phi = radians(-10.0f);
			}
			// if (phi > radians(80.0f))
			// {
			// 	phi = radians(80.0f);
			// }
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

		eye = characterMovement - front;
		lookAt = characterMovement;

		manRot.y = theta + radians(-90.0f);
		manRot.y = - manRot.y;
		manRot.x = phi;

		// cout << "Theta: " << theta << " Phi: " << phi << endl;
		manMoveDir = vec3(sin(manRot.y), 0, cos(manRot.y));
		right = normalize(cross(manMoveDir, up));
		
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

		// load the cube (books)
		cube = new AssimpModel(resourceDirectory + "/cube.obj");

		// load the sphere (spell)
		sphere = new AssimpModel(resourceDirectory + "/SmoothSphere.obj");

		books.emplace_back(cube, sphere,
			glm::vec3(5.0f, 2.0f, 0.0f),  // Initial Position (on a shelf)
			glm::vec3(0.8f, 1.0f, 0.2f),  // Scale (Width, Height, Thickness)
			glm::angleAxis(glm::radians(0.0f), glm::vec3(0, 1, 0)), // Orientation (upright)
			glm::vec3(1.0f, 0.0f, 0.0f)); // Orb Color (Red)

		books.emplace_back(cube, sphere,
			glm::vec3(5.0f, 2.0f, 0.5f),  // Position
			glm::vec3(0.8f, 1.0f, 0.2f),  // Scale
			glm::angleAxis(glm::radians(10.0f), glm::vec3(0, 1, 0)), // Slightly rotated
			glm::vec3(0.0f, 0.0f, 1.0f)); // Orb Color (Blue)

		books.emplace_back(cube, sphere,
			glm::vec3(5.0f, 1.0f, -0.5f), // Lower shelf
			glm::vec3(0.6f, 0.8f, 0.15f), // Smaller book
			glm::angleAxis(glm::radians(-5.0f), glm::vec3(0, 1, 0)),
			glm::vec3(0.0f, 1.0f, 0.0f)); // Orb Color (Green)

		// --- Initialize Enemies ---
		// Create one enemy instance at position (e.g., 5, 1, 5) with 100 HP and 0 move speed (static for now)
		// The vertical pill shape means the base sphere should be scaled more in Y.
		enemies.push_back(new Enemy(glm::vec3(5.0f, 1.0f, 5.0f), 100.0f, 0.0f)); // Pos, HP, Speed
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

	void drawPlayer(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, float animTime) {
		curS->bind();
		
		// select animation for vanguard model
		stickfigure_animator->UpdateAnimation(1.5 * animTime);
		if (manState == WALKING) {
			stickfigure_animator->SetCurrentAnimation(stickfigure_anim);
		}
		else if (manState == STANDING) {
			stickfigure_animator->SetCurrentAnimation(stickfigure_idle);
		}

		// update the bone matrices according to selected animation
		vector<glm::mat4> transforms = stickfigure_animator->GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i) {
			glUniformMatrix4fv(curS->getUniform("finalBonesMatrices[" + std::to_string(i) + "]"), 1, GL_FALSE, value_ptr(transforms[i]));
		}

		// set the model matrix and draw the walking character model
		Model->pushMatrix();
		Model->loadIdentity();
    
    //Character Movement - REFACTOR
		charMove();
		Model->translate(characterMovement);
		Model->scale(0.01f);
		Model->rotate(characterRotation, vec3(0, 1, 0));

		// update the bounding box for collision detection
		glm::mat4 manTransform = glm::translate(glm::mat4(1.0f), charMove())
			* glm::rotate(glm::mat4(1.0f), manRot.x, glm::vec3(1, 0, 0))
			* glm::rotate(glm::mat4(1.0f), manRot.y, glm::vec3(0, 1, 0))
			* glm::scale(glm::mat4(1.0f), manScale);
		updateBoundingBox(stickfigure_running->getBoundingBoxMin(),
			stickfigure_running->getBoundingBoxMax(),
			manTransform,
			manAABBmin,
			manAABBmax);

		glUniform1i(curS->getUniform("hasTexture"), 1);
		SetMaterialMan(curS, 0);
		setModel(curS, Model);
		stickfigure_running->Draw(curS);
		Model->popMatrix();

		curS->unbind();
	}

	void drawBooks(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model) {
		shader->bind();

		for (const auto& book : books) {
			// Common values for book halves
			float halfThickness = book.scale.z * 0.5f;
			glm::vec3 halfScaleVec = glm::vec3(book.scale.x, book.scale.y, halfThickness);

			// Set Material (e.g., brown for cover) - Apply once if same for both halves
			glUniform3f(shader->getUniform("MatAmb"), 0.15f, 0.08f, 0.03f);
			glUniform3f(shader->getUniform("MatDif"), 0.6f, 0.3f, 0.1f);
			glUniform3f(shader->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
			glUniform1f(shader->getUniform("MatShine"), 4.0f);
			glUniform1i(shader->getUniform("hasEmittance"), 0);

			// --- Draw Left Cover/Pages ---
			Model->pushMatrix(); // SAVE current stack state
			{
				// 1. Apply base world transformation
				Model->translate(book.position);
				// Apply orientation - Use multMatrix if orientation is a quat
				Model->multMatrix(glm::mat4_cast(book.orientation));
				// If using Euler angles (vec3 rot), apply multiple Model->rotate(...) here

				// 2. Apply opening rotation (relative to book's local Y)
				if (book.state == BookState::OPENING || book.state == BookState::OPENED) {
					Model->rotate(-book.openAngle * 0.5f, glm::vec3(0, 1, 0));
				}

				// 3. Apply offset for this half (relative to spine)
				Model->translate(glm::vec3(0, 0, -halfThickness * 0.5f));

				// 4. Apply scale for this half
				Model->scale(halfScaleVec);

				// 5. Set the uniform with the final matrix from the stack top
				setModel(shader, Model); // Assumes setModel uses Model->topMatrix()

				// 6. Draw
				book.bookModel->Draw(shader);
			}
			Model->popMatrix(); // RESTORE saved stack state

			// --- Draw Right Cover/Back ---
			Model->pushMatrix(); // SAVE current stack state
			{
				// 1. Apply base world transformation
				Model->translate(book.position);
				Model->multMatrix(glm::mat4_cast(book.orientation));

				// 2. Apply opening rotation
				if (book.state == BookState::OPENING || book.state == BookState::OPENED) {
					Model->rotate(book.openAngle * 0.5f, glm::vec3(0, 1, 0));
				}

				// 3. Apply offset for this half
				Model->translate(glm::vec3(0, 0, halfThickness * 0.5f));

				// 4. Apply scale for this half
				Model->scale(halfScaleVec);

				// 5. Set the uniform
				setModel(shader, Model);

				// 6. Draw
				book.bookModel->Draw(shader);
			}
			Model->popMatrix(); // RESTORE saved stack state
		}

		shader->unbind();
	}

	void Application::drawOrbs(shared_ptr<Program> simpleShader, shared_ptr<MatrixStack> Model) {

		// --- Collision Check Logic ---
		for (auto& orb : orbCollectibles) {
			// Perform collision check ONLY if not collected AND in the IDLE state
			if (!orb.collected && orb.state == OrbState::IDLE && // <<<--- ADD STATE CHECK
				checkAABBCollision(manAABBmin, manAABBmax, orb.AABBmin, orb.AABBmax)) {
				orb.collected = true;
				// orb.state = OrbState::COLLECTED; // Optionally set state
				orbsCollectedCount++;
				std::cout << "Collected a Spell Orb! (" << orbsCollectedCount << ")\n";
			}
		}

		// --- Drawing Logic ---
		simpleShader->bind();

		int collectedOrbDrawIndex = 0;

		for (auto& orb : orbCollectibles) {

			glm::vec3 currentDrawPosition;
			float currentDrawScale = orb.scale; // Use base scale

			if (orb.collected) {
				// Calculate position behind the player (same logic as before)
				float backOffset = 0.4f;
				float upOffsetBase = 0.6f;
				float stackOffset = orb.scale * 2.5f;
				float sideOffset = 0.15f;
				glm::vec3 playerForward = normalize(manMoveDir);
				glm::vec3 playerUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 playerRight = normalize(cross(playerForward, playerUp));
				float currentUpOffset = upOffsetBase + (collectedOrbDrawIndex * stackOffset);
				float currentSideOffset = (collectedOrbDrawIndex % 2 == 0 ? -sideOffset : sideOffset);
				currentDrawPosition = charMove() - playerForward * backOffset
					+ playerUp * currentUpOffset
					+ playerRight * currentSideOffset;
				collectedOrbDrawIndex++;
				// currentDrawScale = orb.scale * 0.8f; // Optional: shrink collected orbs
			}
			else {
				// Use the orb's current position (potentially animated by updateOrbs)
				currentDrawPosition = orb.position;
			}

			// --- Set up transformations ---
			Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(currentDrawPosition);
			Model->scale(currentDrawScale); // Use current scale

			// --- Set Material & Draw ---
			// (Material setting code remains the same)
			glUniform3f(simpleShader->getUniform("MatAmb"), orb.color.r * 0.2f, orb.color.g * 0.2f, orb.color.b * 0.2f);
			glUniform3f(simpleShader->getUniform("MatDif"), orb.color.r * 0.8f, orb.color.g * 0.8f, orb.color.b * 0.8f);
			glUniform3f(simpleShader->getUniform("MatSpec"), 0.8f, 0.8f, 0.8f);
			glUniform1f(simpleShader->getUniform("MatShine"), 32.0f);
			glUniform1i(simpleShader->getUniform("hasEmittance"), 0);

			setModel(simpleShader, Model);
			orb.model->Draw(simpleShader);

			Model->popMatrix();
		} // End drawing loop

		simpleShader->unbind();
	}

	void drawEnemies(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model) {
		if (!sphere) return; // Need the sphere model

		shader->bind(); // Use prog2 for simple colored shapes

		// --- Material Settings ---
		glm::vec3 bodyColor = glm::vec3(0.6f, 0.2f, 0.8f); // Purple-ish body
		glm::vec3 eyeWhiteColor = glm::vec3(1.0f, 1.0f, 1.0f);
		glm::vec3 eyePupilColor = glm::vec3(0.1f, 0.1f, 0.1f);

		// --- Common Eye Parameters ---
		float bodyBaseScaleY = 0.8f; // Base height factor before pill stretch
		glm::vec3 eyeOffsetBase = glm::vec3(0.0f, bodyBaseScaleY * 0.4f, 0.45f); // Y up, Z forward from body center
		float eyeSeparation = 0.25f; // Distance between eye centers
		float whiteScale = 0.18f;
		float pupilScale = 0.1f;
		float pupilOffsetForward = 0.02f; // Push pupil slightly in front of white


		for (const auto* enemy : enemies) {
			if (!enemy || !enemy->isAlive()) continue; // Skip null or dead enemies

			glm::vec3 enemyPos = enemy->getPosition();

			// --- Draw Main Body (Pill Shape) ---
			Model->pushMatrix();
			{
				Model->translate(enemyPos);
				// Scale for pill shape ( taller in Y, squished in X/Z )
				Model->scale(glm::vec3(0.5f, bodyBaseScaleY * 1.6f, 0.5f)); // Adjust scale factors as needed

				// Set body material
				glUniform3f(shader->getUniform("MatAmb"), bodyColor.r * 0.3f, bodyColor.g * 0.3f, bodyColor.b * 0.3f);
				glUniform3f(shader->getUniform("MatDif"), bodyColor.r, bodyColor.g, bodyColor.b);
				glUniform3f(shader->getUniform("MatSpec"), 0.3f, 0.3f, 0.3f);
				glUniform1f(shader->getUniform("MatShine"), 8.0f);

				setModel(shader, Model);
				sphere->Draw(shader); // Draw the scaled sphere as the body
			}
			Model->popMatrix();


			// --- Draw Eyes (Relative to Enemy Center) ---

			// Set Eye Materials Once
			// White Material Setup (done inside loop per part for clarity now)
			// Black Material Setup (done inside loop per part for clarity now)

			// Left Eye
			Model->pushMatrix();
			{
				// Go to enemy center, then offset to eye position
				Model->translate(enemyPos);
				Model->translate(eyeOffsetBase + glm::vec3(-eyeSeparation, 0, 0));

				// White Part
				Model->pushMatrix();
				{
					Model->scale(glm::vec3(whiteScale));
					// Set white material
					glUniform3f(shader->getUniform("MatAmb"), eyeWhiteColor.r * 0.3f, eyeWhiteColor.g * 0.3f, eyeWhiteColor.b * 0.3f);
					glUniform3f(shader->getUniform("MatDif"), eyeWhiteColor.r, eyeWhiteColor.g, eyeWhiteColor.b);
					glUniform3f(shader->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
					glUniform1f(shader->getUniform("MatShine"), 4.0f);
					setModel(shader, Model);
					sphere->Draw(shader);
				}
				Model->popMatrix(); // Pop white scale

				// Pupil Part
				Model->pushMatrix();
				{
					// Move slightly forward from white surface and scale down
					Model->translate(glm::vec3(0, 0, whiteScale * 0.5f + pupilOffsetForward)); // Offset relative to white scale
					Model->scale(glm::vec3(pupilScale));
					// Set black material
					glUniform3f(shader->getUniform("MatAmb"), eyePupilColor.r * 0.3f, eyePupilColor.g * 0.3f, eyePupilColor.b * 0.3f);
					glUniform3f(shader->getUniform("MatDif"), eyePupilColor.r, eyePupilColor.g, eyePupilColor.b);
					glUniform3f(shader->getUniform("MatSpec"), 0.5f, 0.5f, 0.5f); // Some specular highlight
					glUniform1f(shader->getUniform("MatShine"), 32.0f);
					setModel(shader, Model);
					sphere->Draw(shader);
				}
				Model->popMatrix(); // Pop pupil transform
			}
			Model->popMatrix(); // Pop left eye transform


			// Right Eye (Similar to Left)
			Model->pushMatrix();
			{
				Model->translate(enemyPos);
				Model->translate(eyeOffsetBase + glm::vec3(+eyeSeparation, 0, 0)); // Offset to the right

				// White Part
				Model->pushMatrix();
				{
					Model->scale(glm::vec3(whiteScale));
					// Set white material
					glUniform3f(shader->getUniform("MatAmb"), eyeWhiteColor.r * 0.3f, eyeWhiteColor.g * 0.3f, eyeWhiteColor.b * 0.3f);
					glUniform3f(shader->getUniform("MatDif"), eyeWhiteColor.r, eyeWhiteColor.g, eyeWhiteColor.b);
					glUniform3f(shader->getUniform("MatSpec"), 0.1f, 0.1f, 0.1f);
					glUniform1f(shader->getUniform("MatShine"), 4.0f);
					setModel(shader, Model);
					sphere->Draw(shader);
				}
				Model->popMatrix();

				// Pupil Part
				Model->pushMatrix();
				{
					Model->translate(glm::vec3(0, 0, whiteScale * 0.5f + pupilOffsetForward));
					Model->scale(glm::vec3(pupilScale));
					// Set black material
					glUniform3f(shader->getUniform("MatAmb"), eyePupilColor.r * 0.3f, eyePupilColor.g * 0.3f, eyePupilColor.b * 0.3f);
					glUniform3f(shader->getUniform("MatDif"), eyePupilColor.r, eyePupilColor.g, eyePupilColor.b);
					glUniform3f(shader->getUniform("MatSpec"), 0.5f, 0.5f, 0.5f);
					glUniform1f(shader->getUniform("MatShine"), 32.0f);
					setModel(shader, Model);
					sphere->Draw(shader);
				}
				Model->popMatrix();
			}
			Model->popMatrix(); // Pop right eye transform

		} // End loop through enemies

		shader->unbind();
	}

	bool checkAABBCollision(const glm::vec3& minA, const glm::vec3& maxA,
		const glm::vec3& minB, const glm::vec3& maxB)
	{
		return (minA.x <= maxB.x && maxA.x >= minB.x) &&
			(minA.y <= maxB.y && maxA.y >= minB.y) &&
			(minA.z <= maxB.z && maxA.z >= minB.z);
	}

	void updateBooks(float deltaTime) { // deltaTime might not be needed if using glfwGetTime()
		for (auto& book : books) {
			book.update(deltaTime, 0.0f);

			if (book.state == BookState::OPENED && !book.orbSpawned) {
				glm::mat4 baseRotation = glm::mat4_cast(book.orientation);
				// Spawn slightly above the book center to avoid immediate ground collision?
				glm::vec3 orbOffset = glm::vec3(0.0f, book.scale.y * 0.1f + 0.05f, 0.0f); // Small initial Y offset
				glm::vec3 orbSpawnPos = book.position + glm::vec3(baseRotation * glm::vec4(orbOffset, 0.0f));

				// Constructor handles setting state to LEVITATING and calculating idlePosition
				orbCollectibles.emplace_back(sphere, orbSpawnPos, book.orbScale, book.orbColor);

				book.orbSpawned = true;
				cout << "Orb Spawned! State: LEVITATING" << endl;
			}
		}
	}

	void interactWithBooks() {

		// Player's AABB (manAABBmin, manAABBmax) is assumed to be updated from drawPlayer
		for (auto& book : books) {
			// Only check for interaction if the book is on the shelf
			if (book.state == BookState::ON_SHELF) {

				// 1. Define the Book's Local AABB (Approximation when closed)
				// We approximate the closed book as a single box with dimensions book.scale
				// centered at the origin.
				glm::vec3 bookLocalMin = -book.scale * 0.5f;
				glm::vec3 bookLocalMax = book.scale * 0.5f;

				// 2. Calculate the Book's World Transformation Matrix
				glm::mat4 bookWorldTransform = glm::translate(glm::mat4(1.0f), book.position) *
					glm::mat4_cast(book.orientation);

				// 3. Calculate the Book's World AABB
				glm::vec3 bookWorldMin, bookWorldMax;
				updateBoundingBox(bookLocalMin, bookLocalMax, bookWorldTransform, bookWorldMin, bookWorldMax);

				// 4. Perform AABB Collision Check
				if (checkAABBCollision(manAABBmin, manAABBmax, bookWorldMin, bookWorldMax)) {
					// Collision detected! Trigger the fall.
					book.startFalling(0);
					break; // break here so we only trigger one book
				}
			}
		}
	}

	void updateOrbs(float currentTime) {
		for (auto& orb : orbCollectibles) {
			// Update levitation only if not already collected
			if (!orb.collected) {
				orb.updateLevitation(currentTime);
			}
		}
	}

	void updateEnemies(float deltaTime) {
		// TODO: Add enemy movement, AI, attack logic later
		for (auto* enemy : enemies) {
			if (!enemy || !enemy->isAlive()) continue;
			// Example: Simple bobbing motion
			// float bobSpeed = 2.0f;
			// float bobHeight = 0.05f;
			// glm::vec3 currentPos = enemy->getPosition();
			// enemy->setPosition(glm::vec3(currentPos.x, 0.8f + sin(glfwGetTime() * bobSpeed) * bobHeight, currentPos.z));

			 // IMPORTANT: Update enemy AABB if it moves
			 // enemy->updateAABB(); // Need to add AABB members and update method to Enemy/Entity class
		}
	}

	vec3 charMove() {
		float moveSpeed = 0.045;
		vec3 moveDir = vec3(0.0f, 0.0f, 0.0f);

		if (movingForward) {
			characterMovement += manMoveDir * moveSpeed;
			eye += manMoveDir * moveSpeed;
			lookAt = characterMovement;

			characterRotation = manRot.y + 0.0f;
			
		}
		else if (movingBackward) {
			characterMovement -= manMoveDir * moveSpeed;
			eye -= manMoveDir * moveSpeed;
			lookAt = characterMovement;

			characterRotation = manRot.y + 3.14f;
		}
		if (movingRight) {
			characterMovement += right * moveSpeed;
			eye += right * moveSpeed;
			lookAt = characterMovement;
			
			characterRotation = manRot.y + 4.71;
		}
		else if (movingLeft) {
			characterMovement -= right * moveSpeed;
			eye -= right * moveSpeed;
			lookAt = characterMovement;

			characterRotation = manRot.y + 1.57;
		}
		normalize(characterMovement);
		return characterMovement;
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

		updateBooks(frametime);
		updateOrbs(glfwGetTime());
		updateEnemies(frametime);

		// Apply perspective projection
		Projection->pushMatrix();

		// Projection->perspective(45.0f, aspect, 0.01f, 200.0f);
		Projection->perspective(45.0f, aspect, 0.01f, 400.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
		View->loadIdentity();
		View->lookAt(eye, lookAt, vec3(0, 1, 0));

		// Setup Shaders
		prog2->bind();
		glUniformMatrix4fv(prog2->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(prog2->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(prog2->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
		glUniform1f(prog2->getUniform("lightIntensity[0]"), 1.0); // light intensity
		glUniform3f(prog2->getUniform("lightPos[0]"), 0, 2, 0); // light position at the computer screen
		glUniform1i(prog2->getUniform("numLights"), 1); // light position at the computer screen
		prog2->unbind();

		assimptexProg->bind();
		glUniformMatrix4fv(assimptexProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(assimptexProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(assimptexProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // white light
		glUniform1f(assimptexProg->getUniform("lightIntensity[0]"), 0.0); // light intensity
		glUniform3f(assimptexProg->getUniform("lightPos[0]"), 0, 10, 0); // light position at the computer screen
		glUniform1i(assimptexProg->getUniform("numLights"), 1); // light position at the computer screen
		assimptexProg->unbind();

		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		glUniform3f(texProg->getUniform("lightColor[0]"), 1.0, 1.0, 1.0); // White light
		glUniform1f(texProg->getUniform("lightIntensity[0]"), 5.0); // High intensity for visibility
		glUniform3f(texProg->getUniform("lightPos[0]"), 0, 2, 0);
		glUniform1i(texProg->getUniform("numLights"), 1);
		glUniform3f(texProg->getUniform("MatAmb"), 0.5, 0.5, 0.5); // Bright ambient
		glUniform3f(texProg->getUniform("MatSpec"), 0.8, 0.8, 0.8); // Strong specular
		glUniform1f(texProg->getUniform("MatShine"), 32.0f); // High shininess
		texProg->unbind();

		drawGround(prog2, Model);

		drawEnemies(prog2, Model);

		drawPlayer(assimptexProg, Model, animTime);
		
		drawOrbs(prog2, Model);

		drawBooks(prog2, Model);

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

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(windowManager->getHandle(), application);
	glfwSetCursorPosCallback(windowManager->getHandle(), mouseMoveCallbackWrapper);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initGeom(resourceDir);
	application->initGround();

	auto lastTime = chrono::high_resolution_clock::now();

	glfwSetInputMode(windowManager->getHandle(), GLFW_STICKY_KEYS, GLFW_TRUE);

	cout << "Controls: " << endl << "WASD: Move" << endl << "Mouse: Look around" << endl
		<< "'F': Interact with book" << "F11 Fullscreen" << endl << "'L': Toggle cursor mode";

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
