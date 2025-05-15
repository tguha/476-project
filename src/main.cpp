//======================================
// The start of our wizarding adventure
//======================================

#include <iostream>
#include <glad/glad.h>
#include <chrono>
#include <thread>
#include <set>
#pragma comment(lib, "winmm.lib")

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
#include "LibraryGen.h"
// #include "Grid.h"
#include "Enemy.h"
#include "IceElemental.h"
#include "Player.h"
#include "BossRoomGen.h"
#include "FrustumCulling.h"
#include "BossEnemy.h"
#include "Config.h"
#include "GameObjectTypes.h"
#include "../particles/particleGen.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <algorithm>
#include <limits>

using namespace std;
using namespace glm;

class Application : public EventCallbacks {
public:
	// Setup window and context
	WindowManager * windowManager = nullptr;
	bool windowMaximized = false;
	int window_width = Config::DEFAULT_WINDOW_WIDTH;
	int window_height = Config::DEFAULT_WINDOW_HEIGHT;

	// Our shader programs
	shared_ptr<Program> particleProg;
	shared_ptr<Program> DepthProg;
	shared_ptr<Program> DepthProgDebug;
	shared_ptr<Program> ShadowProg;
	shared_ptr<Program> DebugProg;
	shared_ptr<Program> hudProg;
	shared_ptr<Program> prog2_enemy;
	shared_ptr<Program> redFlashProg;

	// Shadows
	GLuint depthMapFBO;
	const GLuint S_WIDTH = 2048, S_HEIGHT = 2048;
	GLuint depthMap;

	// PLayer
	std::shared_ptr<Player> player;

	// ground data - Reused for all flat ground planes
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	// Geometry for texture render
	GLuint quad_VertexArrayID;
	GLuint quad_vertexbuffer;

	// Textures
	shared_ptr<Texture> borderWallTex;
	shared_ptr<Texture> libraryGroundTex;

	shared_ptr<Texture> carpetTex;
	shared_ptr<Texture> particleAlphaTex;

	vector<WallObject> borderWalls;
	vector<LibGrndObject> libraryGrounds;
	
	std::set<WallObjKey> borderWallKeys; // Set to track unique keys
	std::set<LibGrndObjKey> libraryGroundKeys; // Set to track unique keys
	
	//std::unordered_set<int> borderWallIDs; // Set to track unique IDs
	//std::unordered_set<int> libraryGroundIDs; // Set to track unique IDs

	// Scene layout parameters
	vec3 libraryCenter = vec3(0.0f, Config::GROUND_HEIGHT, 0.0f);
	vec3 bossAreaCenter = vec3(0.0f, Config::GROUND_HEIGHT, 60.0f); // Further away
	vec3 doorPosition = vec3(0.0f, 1.5f, Config::GROUND_SIZE); // Center of door at library edge
	vec3 doorScale = vec3(1.5f, 3.0f, 0.2f); // Width, Height, Thickness
	float pathWidth = 4.0f; // Width of the path connecting areas

	// setup collectibles vector
	std::vector<Collectible> orbCollectibles;
	int orbsCollectedCount = 0;
	std::vector<Enemy*> enemies;

	// --- Spell Projectiles ---
	std::vector<SpellProjectile> activeSpells;
	std::shared_ptr<particleGen> particleSystem; // Add particle system
	glm::vec3 baseSphereLocalAABBMin; // Store base sphere AABB once
	glm::vec3 baseSphereLocalAABBMax;
	bool sphereAABBCalculated = false;

	// -- Boss Enemy Spell Projectiles --
	std::vector<SpellProjectile> bossActiveSpells;

	AssimpModel *book_shelf1, *book_shelf2;
	AssimpModel *candelabra, *chest, *library_bench, *low_poly_bookshelf, *table_chairs1, *table_chairs2, *grandfather_clock, *bookstand, *door;

	AssimpModel *healthBar;

	AssimpModel *cube, *sphere;

	AssimpModel *sky_sphere;

	AssimpModel *border, *lock, *lockHandle, *key;

	//key collectibles
	std::vector<Collectible> keyCollectibles;
	int keysCollectedCount = 0;

	//  vector of books
	vector<Book> books;

	AssimpModel *stickfigure_running, *stickfigure_standing;
	Animation *stickfigure_anim, *stickfigure_idle;
	Animator *stickfigure_animator;

	AssimpModel *CatWizard;

	AssimpModel *iceElemental;

	BossEnemy *bossEnemy;

	float AnimDeltaTime = 0.0f;
	float AnimLastFrame = 0.0f;

	int change_mat = 0;

	// vec3 characterMovement = vec3(0, 0, 0);
	glm::vec3 manScale = glm::vec3(0.01, 0.01, 0.01);
	glm::vec3 manMoveDir = glm::vec3(sin(radians(0.0f)), 0, cos(radians(0.0f)));
	AABB playerAABB; // Contains <vec3>min and <vec3>max

	float theta = glm::radians(Config::CAMERA_DEFAULT_THETA_DEGREES); // controls yaw
	float phi = glm::radians(Config::CAMERA_DEFAULT_PHI_DEGREES); // controls pitch
	float radius = Config::CAMERA_DEFAULT_RADIUS;

	float wasd_sens = 0.5f;

	glm::vec3 eye = glm::vec3(-6, 1.03, 0); /*MINI MAP*/
	glm::vec3 lookAt = glm::vec3(0, 0, 0); /*MINI MAP*/
	glm::vec3 up = glm::vec3(0, 1, 0);
	bool CULL = false;

	vec3 right = normalize(cross(manMoveDir, up));

	bool mouseIntialized = false;
	double lastX, lastY;

	int debug = 0;
	int debug_pos = 0;

	bool debug_shelf = false;

	bool cursor_visable = true;

	//Movement Variables (Maybe move?)
	bool movingForward = false;
	bool movingBackward = false;
	bool movingLeft = false;
	bool movingRight = false;

	//unlock bool
	bool unlocked = false;
	float lTheta = 0;

	float characterRotation = 0.0f;

	//Debug Camera
	bool debugCamera = false;
	vec3 debugEye = vec3(0.0f, 0.0f, 0.0f);
	float debugMovementSpeed = 0.2f;

	Man_State manState = Man_State::STANDING;

	LibraryGen *library = new LibraryGen();
	Grid<LibraryGen::Cell> grid;
	ivec2 gridSize = glm::ivec2(30, 30); // Size of the grid (number of cells in each dimension)

	BossRoomGen *bossRoom = new BossRoomGen();
	Grid<BossRoomGen::Cell> bossGrid;
	ivec2 bossGridSize = glm::ivec2(30, 30); // Size of the grid (number of cells in each dimension)

	ivec2 bossEntranceDir = glm::ivec2(0, 1); // Direction of the boss entrance (relative to the library grid)

	glm::vec4 planes[6]; // Frustum planes

	// Flags for game state
	bool canFightboss = false; // Flag to check if the player can fight the boss
	bool allEnemiesDead = false; // Flag to check if all enemies are dead
	bool restartGen = false;
	bool bossfightstarted = false;
	bool bossfightended = false;

	// -- Camera Occlusion Query --
	GLuint occlusionQueryID = 0; // Occlusion query ID
	GLuint visible = 0;
	GLuint occlusionBoxVAO = 0;
	GLuint occlusionBoxVBO = 0;

	float cameraVisibleCooldown = 0.0f; // Cooldown for camera visibility check
	bool wasVisibleLastFrame = true;

	// Set up the FBO for storing the light's depth map
	void initShadow() {
		glGenFramebuffers(1, &depthMapFBO); // Generate FBO for shadow depth
		glGenTextures(1, &depthMap); // Generate texture for shadow depth
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, S_WIDTH, S_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // bind with framebuffer's depth buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0); // attach the texture to the framebuffer
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind the framebuffer
	}

	void updateCameraVectors() {
		if (!debugCamera) {
			// 1. Compute the desired front vector
			glm::vec3 front;
			front.x = cos(phi) * cos(theta);
			front.y = sin(phi);
			front.z = cos(phi) * cos((pi<float>() / 2) - theta);

			// 2. Store current desired eye (before occlusion check)
			glm::vec3 playerPos = glm::vec3(player->getPosition().x, player->getPosition().y + 1.0f, player->getPosition().z);
			glm::vec3 desiredEye = playerPos - front * radius;

			// 3. Adjust radius if needed
			float desiredRadius = Config::CAMERA_DEFAULT_RADIUS;
			float minRadius = 0.5f;
			float step = 0.45f;
			float testRadius = desiredRadius;
			float finalRadius = radius;
			const float cooldownTime = 0.45f; // Cooldown time in seconds
			cameraVisibleCooldown -= AnimDeltaTime;

			// if (visible == 0) {
			// 	// std::cout << "Camera Occluded" << std::endl;
			// 	radius = glm::max(minRadius, radius - step);
			// } else {
			// 	radius = glm::min(desiredRadius, radius + step);
			// }

			if (visible == 0) {
				radius = glm::max(minRadius, radius - step);
				cameraVisibleCooldown = cooldownTime; // Reset cooldown
				wasVisibleLastFrame = false; // Mark as not visible
			} else if (cameraVisibleCooldown <= 0.0f) {
				// Only expand if cooldown is over
				radius = glm::min(desiredRadius, radius + step);
				wasVisibleLastFrame = true; // Mark as visible
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
				manState = Man_State::WALKING;

			radius = glm::mix(radius, finalRadius, 0.5f); // Smoothly interpolate radius

			// 4. Recalculate final eye based on adjusted radius
			eye = playerPos - front * radius;
			lookAt = playerPos;

			// 5. Update player rotation
			player->setRotY(-(theta + radians(-90.0f)));
			player->setRotX(phi);

			manMoveDir = vec3(sin(player->getRotY()), 0, cos(player->getRotY()));
			right = normalize(cross(manMoveDir, up));

			// lookAt = eye + front;
		}
		else {
			//Activate Debug Camera
			float radius = 1.0;
			float x = radius * cos(phi) * cos(theta);
			float y = radius * sin(phi);
			float z = radius * cos(phi) * sin(theta);
			// Defined above Globally- eyePos = vec3(0.0, 0.0, 0.0);
			vec3 targetPos = vec3(x, y, z);
			vec3 viewVec = normalize(targetPos);

			if (movingForward) {
				debugEye += debugMovementSpeed * viewVec;
			}
			if (movingBackward) {
				debugEye -= debugMovementSpeed * viewVec;
			}
			if (movingLeft) {
				debugEye -= debugMovementSpeed * normalize(cross(targetPos, up));
			}
			if (movingRight) {
				debugEye += debugMovementSpeed * normalize(cross(targetPos, up));
			}

			eye = debugEye;
			lookAt = debugEye + targetPos;
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
		if (action == GLFW_PRESS) {
			shootSpell();
		}
	}

	void resizeCallback(GLFWwindow *window, int width, int height) {
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory) {
		GLSL::checkVersion();

		// Set background color and enable z-buffer test
		glClearColor(.12f, .34f, .56f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		// Initialize GLSL programs for shadow mapping
		DepthProg = make_shared<Program>();
		DepthProg->setVerbose(Config::DEBUG_SHADER);
		DepthProg->setShaderNames(resourceDirectory + "/depth_vert.glsl", resourceDirectory + "/depth_frag.glsl");
		DepthProg->init();

		DepthProgDebug = make_shared<Program>();
		DepthProgDebug->setVerbose(Config::DEBUG_SHADER);
		DepthProgDebug->setShaderNames(resourceDirectory + "/depth_vertDebug.glsl", resourceDirectory + "/depth_fragDebug.glsl");
		DepthProgDebug->init();

		ShadowProg = make_shared<Program>();
		ShadowProg->setVerbose(Config::DEBUG_SHADER);
		ShadowProg->setShaderNames(resourceDirectory + "/shadow_vert.glsl", resourceDirectory + "/shadow_frag.glsl");
		ShadowProg->init();

		DebugProg = make_shared<Program>();
		DebugProg->setVerbose(Config::DEBUG_SHADER);
		DebugProg->setShaderNames(resourceDirectory + "/pass_vert.glsl", resourceDirectory + "/pass_texfrag.glsl");
		DebugProg->init();

		// Add unfigorm and attrubutes to each of the programs
		DepthProg->addUniform("LP");
		DepthProg->addUniform("LV");
		DepthProg->addUniform("M");
		DebugProg->addUniform("texBuf");
		DepthProg->addAttribute("vertPos");

		DepthProgDebug->addUniform("LP");
		DepthProgDebug->addUniform("LV");
		DepthProgDebug->addUniform("M");
		DepthProgDebug->addAttribute("vertPos");
		DepthProgDebug->addAttribute("vertNor");
		DepthProgDebug->addAttribute("vertTex");

		ShadowProg->addUniform("P");
		ShadowProg->addUniform("V");
		ShadowProg->addUniform("M");
		ShadowProg->addUniform("LV");
		ShadowProg->addUniform("lightDir");
		ShadowProg->addUniform("lightColor");
		ShadowProg->addUniform("lightIntensity");
		ShadowProg->addUniform("cameraPos");
		ShadowProg->addAttribute("vertPos");
		ShadowProg->addAttribute("vertNor");
		ShadowProg->addAttribute("vertTex");
		ShadowProg->addUniform("shadowDepth");

		ShadowProg->addUniform("hasTexAlb");
		ShadowProg->addUniform("hasTexSpec");
		ShadowProg->addUniform("hasTexRough");
		ShadowProg->addUniform("hasTexMet");
		ShadowProg->addUniform("hasTexNor");
		ShadowProg->addUniform("hasTexEmit");

		ShadowProg->addUniform("hasMaterial");
		ShadowProg->addUniform("hasBones");

		ShadowProg->addUniform("TexAlb");
		ShadowProg->addUniform("TexSpec");
		ShadowProg->addUniform("TexRough");
		ShadowProg->addUniform("TexMet");
		ShadowProg->addUniform("TexNor");
		ShadowProg->addUniform("TexEmit");

		ShadowProg->addUniform("MatAlbedo");
		ShadowProg->addUniform("MatRough");
		ShadowProg->addUniform("MatMetal");
		ShadowProg->addUniform("MatEmit");

		ShadowProg->addUniform("enemyAlpha");

		for (int i = 0; i < Config::MAX_BONES; i++) {
			ShadowProg->addUniform("finalBoneMatrices[" + to_string(i) + "]");
		}
		ShadowProg->addAttribute("boneIds");
		ShadowProg->addAttribute("weights");

		initShadow();

		hudProg = make_shared<Program>();
		hudProg->setVerbose(Config::DEBUG_SHADER);
		hudProg->setShaderNames(resourceDirectory + "/hud_vert.glsl", resourceDirectory + "/hud_frag.glsl");
		hudProg->init();
		hudProg->addUniform("projection");
		hudProg->addUniform("model");
		hudProg->addUniform("healthPercent");
		hudProg->addUniform("BarStartX");
		hudProg->addUniform("BarWidth");

		// Initialize the particle program
		particleProg = make_shared<Program>();
		particleProg->setVerbose(Config::DEBUG_SHADER);
		particleProg->setShaderNames(resourceDirectory + "/particle_vert.glsl", resourceDirectory + "/particle_frag.glsl");
		particleProg->init();
		particleProg->addUniform("P");
		particleProg->addUniform("V");
		particleProg->addUniform("M");
		particleProg->addUniform("alphaTexture");
		particleProg->addAttribute("vertPos");
		particleProg->addAttribute("vertColor");

		redFlashProg = make_shared<Program>();
		redFlashProg->setVerbose(Config::DEBUG_SHADER);
		redFlashProg->setShaderNames(resourceDirectory + "/red_flash_vert.glsl", resourceDirectory + "/red_flash_frag.glsl");
		redFlashProg->init();
		redFlashProg->addUniform("projection");
		redFlashProg->addUniform("model");
		redFlashProg->addUniform("color");
		redFlashProg->addUniform("alpha");

		updateCameraVectors();

		borderWallTex = make_shared<Texture>();
		// borderWallTex->setFilename(resourceDirectory + "/sky_sphere/sky_sphere.fbm/infinite_lib2.png");
		borderWallTex->setFilename(resourceDirectory + "/Wall/textures/mossCastle.png");
		borderWallTex->init();
		borderWallTex->setUnit(0);
		borderWallTex->setWrapModes(GL_REPEAT, GL_REPEAT);

		libraryGroundTex = make_shared<Texture>();
		libraryGroundTex->setFilename(resourceDirectory + "/book_shelf/textures/wood_texture.png");
		libraryGroundTex->init();
		libraryGroundTex->setUnit(0);
		libraryGroundTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		carpetTex = make_shared<Texture>();
		carpetTex->setFilename(resourceDirectory + "/cluster_assets/carpet_texture1.png");
		carpetTex->init();
		carpetTex->setUnit(0);
		carpetTex->setWrapModes(GL_REPEAT, GL_REPEAT);

		// Initialize particle alpha texture
		particleAlphaTex = make_shared<Texture>();
		particleAlphaTex->setFilename(resourceDirectory + "/alpha.bmp");
		particleAlphaTex->init();
		particleAlphaTex->setUnit(0);
		particleAlphaTex->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Initialize particle system
		particleSystem = make_shared<particleGen>(vec3(0.0f), 0.0f, 0.2f, 0.6f, 0.8f, 0.8f, 1.0f, 0.1f, 0.2f);
		particleSystem->gpuSetup();
	}

	void initMapGen()
	{
		library->generate(gridSize, glm::vec3(0, 0, 0), player->getPosition(), bossEntranceDir);
		grid = library->getGrid();

		if (bossEntranceDir.y > 0) {
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(0) + 2), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x - 3, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x - 3, vec3(library->mapGridXtoWorldX((gridSize.x - 1) / 2), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(0) + 2, 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
		} else if (bossEntranceDir.y < 0) {
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x - 3, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(0) + 2), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x - 3, vec3(library->mapGridXtoWorldX((gridSize.x - 1) / 2), 0, library->mapGridYtoWorldZ(0) + 2), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(0) + 2, 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
		} else if (bossEntranceDir.x > 0) {
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(0) + 2), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.y - 3, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y - 3, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ((gridSize.y - 1) / 2)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(0) + 2, 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
		} else if (bossEntranceDir.x < 0) {
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1) + 2), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.x * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(0)), vec3(-1, 0, 0), 10.0f, borderWallTex);
			addWall(gridSize.y - 3, vec3(library->mapGridXtoWorldX(0) + 2, 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y - 3, vec3(library->mapGridXtoWorldX(0) + 2, 0, library->mapGridYtoWorldZ((gridSize.y - 1) / 2)), vec3(0, 0, -1), 10.0f, borderWallTex);
			addWall(gridSize.y * 2, vec3(library->mapGridXtoWorldX(gridSize.x - 1), 0, library->mapGridYtoWorldZ(gridSize.y - 1)), vec3(0, 0, -1), 10.0f, borderWallTex);
		}

		addLibGrnd(gridSize.x * 2, gridSize.y * 2, 0.0f, vec3(0, 0, 0), libraryGroundTex);

		bossRoom->generate(bossGridSize, gridSize, glm::vec3(0, 0, 0), bossEntranceDir);
		bossGrid = bossRoom->getGrid();
		addLibGrnd(bossGridSize.x * 2, bossGridSize.y * 2, 0.0f, bossRoom->getWorldOrigin(), libraryGroundTex);
	}

	void initGeom(const std::string& resourceDirectory) {
 		string errStr;

		// load the walking character model
		stickfigure_running = new AssimpModel(resourceDirectory + "/CatWizard/CatWizardNoTex.fbx");
		stickfigure_running->assignTexture("texture_diffuse", resourceDirectory + "/CatWizard/textures/ImphenziaPalette02-Albedo.png");
		//stickfigure_anim = new Animation(resourceDirectory + "/CatWizard/untitled.fbx", stickfigure_running, 0);
		//stickfigure_idle = new Animation(resourceDirectory + "/Vanguard/Vanguard.fbx", stickfigure_running, 1);

		//TEST Load the cat
		//CatWizard = new AssimpModel(resourceDirectory + "/CatWizard/CatWizardOrange.fbx");

		// --- Calculate Player Collision Box NOW that model is loaded ---
		calculatePlayerLocalAABB();

		stickfigure_animator = new Animator(stickfigure_anim);

		// load the cube (books)
		cube = new AssimpModel(resourceDirectory + "/cube.obj");

		book_shelf1 = new AssimpModel(resourceDirectory + "/cluster_assets/bookshelf_texture2.obj");

		book_shelf1->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/darker_bookshelf_diffuse.png");

		book_shelf2 = new AssimpModel(resourceDirectory + "/cluster_assets/bookshelf_texture2.obj");

		book_shelf2->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/glowing_bookshelf_bake_diffuse.png");

		candelabra = new AssimpModel(resourceDirectory + "/cluster_assets/candelabrum/Candelabrum.obj");

		candelabra->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/candelabrum/textures/defaultobject_gloss.png");
		candelabra->assignTexture("texture_specular", resourceDirectory + "/cluster_assets/candelabrum/textures/defaultobject_specular.png");
		candelabra->assignTexture("texture_normal", resourceDirectory + "/cluster_assets/candelabrum/textures/defaultobject_normal.png");

		chest = new AssimpModel(resourceDirectory + "/cluster_assets/chest/Chest.obj");

		chest->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/chest/textures/TreasureChestDiffuse_2.png");
		chest->assignTexture("texture_roughness", resourceDirectory + "/cluster_assets/chest/textures/TreasureChestRoughness_2.png");
		chest->assignTexture("texture_metalness", resourceDirectory + "/cluster_assets/chest/textures/TreasureChestMetal_2.png");
		chest->assignTexture("texture_normal", resourceDirectory + "/cluster_assets/chest/textures/TreasureChestNormal_2.png");

		library_bench = new AssimpModel(resourceDirectory + "/cluster_assets/library_bench/library_bench.obj");

		library_bench->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/library_bench/textures/bench_diffuse.png");

		table_chairs1 = new AssimpModel(resourceDirectory + "/cluster_assets/table_chairs/table_chairs_3.obj");

		table_chairs1->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/table_chairs/textures/table_chairs_3_diffuse.png");

		table_chairs2 = new AssimpModel(resourceDirectory + "/cluster_assets/table_chairs/table_chairs_4.obj");

		table_chairs2->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/table_chairs/textures/table_chairs_4_diffuse.png");

		grandfather_clock = new AssimpModel(resourceDirectory + "/cluster_assets/grandfather_clock/grandfather_clock.obj");

		grandfather_clock->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/grandfather_clock/textures/Clock_L_lambert1_BaseColor.tga.png");
		grandfather_clock->assignTexture("texture_metalness", resourceDirectory + "/cluster_assets/grandfather_clock/textures/Clock_L_lambert1_Metallic.tga.png");
		grandfather_clock->assignTexture("texture_roughness", resourceDirectory + "/cluster_assets/grandfather_clock/textures/Clock_L_lambert1_Roughness.tga.png");
		grandfather_clock->assignTexture("texture_normal", resourceDirectory + "/cluster_assets/grandfather_clock/textures/Clock_L_lambert1_Normal.tga.jpg");

		bookstand = new AssimpModel(resourceDirectory + "/cluster_assets/bookstand/bookstand.obj");
		bookstand->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/bookstand/textures/bookstand_diffuse.png");

		door = new AssimpModel(resourceDirectory + "/cluster_assets/door/door.obj");
		door->assignTexture("texture_diffuse", resourceDirectory + "/cluster_assets/door/Door_diffuse.png");

		sky_sphere = new AssimpModel(resourceDirectory + "/sky_sphere/skybox_sphere.obj");
		sky_sphere->assignTexture("texture_diffuse", resourceDirectory + "/sky_sphere/sky_sphere.fbm/infinite_lib2.png");

		// load the sphere (spell)
		sphere = new AssimpModel(resourceDirectory + "/SmoothSphere.obj");

		// load enemies
		iceElemental = new AssimpModel(resourceDirectory + "/IceElemental/IceElem.fbx");

		// health bar
		healthBar = new AssimpModel(resourceDirectory + "/Quad/hud_quad.obj");
		healthBar->assignTexture("texture_diffuse", resourceDirectory + "/healthbar.bmp");

		//key
		key = new AssimpModel(resourceDirectory + "/Key_and_Lock/key.obj");

		//lock
		lock = new AssimpModel(resourceDirectory + "/Key_and_Lock/lockCopy.obj");
		lockHandle = new AssimpModel(resourceDirectory + "/Key_and_Lock/lockHandle.obj");

		//key
		key = new AssimpModel(resourceDirectory + "/Key_and_Lock/key.obj");

		//lock
		lock = new AssimpModel(resourceDirectory + "/Key_and_Lock/lockCopy.obj");
		lockHandle = new AssimpModel(resourceDirectory + "/Key_and_Lock/lockHandle.obj");

		baseSphereLocalAABBMin = sphere->getBoundingBoxMin();
		baseSphereLocalAABBMax = sphere->getBoundingBoxMax();
		sphereAABBCalculated = true;
		cout << "[DEBUG] Stored Base Sphere Local AABB." << endl;

		// --- Initialize Enemy(s) ---
		cout << "Initializing enemies..." << endl;
		// Use the scale factor used in drawEnemies
		// Body scale was (0.5f, bodyBaseScaleY * 1.6f, 0.5f) where bodyBaseScaleY = 0.8f => (0.5, 1.28, 0.5)
		// glm::vec3 enemyCollisionScale = glm::vec3(0.5f, 1.28f, 0.5f); // Define the scale
		// glm::vec3 enemyCollisionScale = glm::vec3(1.0f, 1.0f, 1.0f);
		vec3 bossSpawnPos = bossRoom->getWorldOrigin();

		// Check if sphere model is loaded before creating enemies that use it
		if (sphere) {
			// enemies.push_back(new Enemy(bossSpawnPos, 200.0f, 0.0f, sphere, enemyCollisionScale, vec3(0.0f))); // <<-- Pass sphere and scale
			// cout << " Enemy placed at boss area: (" << bossSpawnPos.x << ", " << bossSpawnPos.y << ", " << bossSpawnPos.z << ")" << endl;
			// enemies.push_back(new Enemy(libraryCenter + vec3(-5.0f, 0.8f, 8.0f), ENEMY_HP_MAX / 2, 2.0f, sphere, enemyCollisionScale, vec3(0.0f))); // <<-- Pass sphere and scale

			// std::vector<vec3> enemySpawnPositions = library->getEnemySpawnPositions();
			// for (const auto& spawnPos : enemySpawnPositions) {
			// 	enemies.push_back(new IceElemental(vec3(spawnPos.x, Config::ICE_ELEMENTAL_TRANS_Y, spawnPos.z), ENEMY_HP_MAX, 2.0f, iceElemental, enemyCollisionScale, vec3(0.0f)));
			// 	// cout << " Enemy placed at: (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << endl;
			// }
			initEnemies();

			bossEnemy = new BossEnemy(bossSpawnPos, BOSS_HP_MAX, sphere, vec3(1.0f), vec3(0, 1, 0), BOSS_SPECIAL_ATTACK_COOLDOWN);
		}
		else {
			cerr << "ERROR: Sphere model not loaded, cannot create enemies." << endl;
		}
	}
	
	void SetMaterial(shared_ptr<Program> shader, Material color) {
		/* Some important notes about PBR materials:

		Albedo (Base Color):
		Never use pure black (0,0,0) or pure white (1,1,1)
		Realistic materials range from about 0.04 to 0.95
		For metals, this is the actual metal color

		Roughness:
		0.0 = perfectly smooth (mirror-like)
		1.0 = completely rough (diffuse)
		Most materials fall between 0.2-0.8

		Metalness:
		0.0 = non-metallic (dielectric)
		1.0 = metallic
		Should almost always be 0 or 1, rarely in between

		Emission:
		Only for materials that emit light
		Values can exceed 1.0 for strong emission
		Most materials have (0,0,0) emission

		Good reference values can be found at physicallybased.info. */

		switch (color) {
		case Material::purple:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.3f, 0.1f, 0.4f);
				glUniform1f(shader->getUniform("MatRough"), 0.7f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::black:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.04f, 0.04f, 0.04f);
				glUniform1f(shader->getUniform("MatRough"), 0.8f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::eye_white:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.95f, 0.95f, 0.95f);
				glUniform1f(shader->getUniform("MatRough"), 0.2f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::pupil_white:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.85f, 0.85f, 0.9f);
				glUniform1f(shader->getUniform("MatRough"), 0.1f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::bronze:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.714f, 0.4284f, 0.181f);
				glUniform1f(shader->getUniform("MatRough"), 0.4f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::silver:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.972f, 0.960f, 0.915f);
				glUniform1f(shader->getUniform("MatRough"), 0.2f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::brown:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.25f, 0.15f, 0.08f);
				glUniform1f(shader->getUniform("MatRough"), 0.7f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::orb_glowing_blue:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.1f, 0.2f, 0.5f);
				glUniform1f(shader->getUniform("MatRough"), 1.0f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::orb_glowing_red:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.5f, 0.1f, 0.1f);
				glUniform1f(shader->getUniform("MatRough"), 1.0f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.9f, 0.3f, 0.2f);
				break;
			case Material::orb_glowing_yellow:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.5f, 0.4f, 0.1f);
				glUniform1f(shader->getUniform("MatRough"), 1.0f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.9f, 0.8f, 0.2f);
				break;
			case Material::grey:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.8f, 0.8f, 0.8f);
				glUniform1f(shader->getUniform("MatRough"), 0.6f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::wood:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.65f, 0.45f, 0.25f);
				glUniform1f(shader->getUniform("MatRough"), 0.8f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::mini_map:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.65f, 0.45f, 0.25f);
				glUniform1f(shader->getUniform("MatRough"), 0.0f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 1.0f, 1.0f, 1.0f);
				break;
			case Material::defaultMaterial:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.5f, 0.5f, 0.5f);
				glUniform1f(shader->getUniform("MatRough"), 0.0f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::blue_body:
				glUniform3f(shader->getUniform("MatAlbedo"), 0.35f, 0.4f, 0.914f);
				glUniform1f(shader->getUniform("MatRough"), 0.8f);
				glUniform1f(shader->getUniform("MatMetal"), 0.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
			case Material::gold:
				glUniform3f(shader->getUniform("MatAlbedo"), 1.0f, 0.766f, 0.336f);
				glUniform1f(shader->getUniform("MatRough"), 0.2f);
				glUniform1f(shader->getUniform("MatMetal"), 1.0f);
				glUniform3f(shader->getUniform("MatEmit"), 0.0f, 0.0f, 0.0f);
				break;
		}
	}

	void setProgFlags(shared_ptr<Program> shader, bool hasMat, bool hasBones) {
		//shader->bind();
		if (hasMat && shader->hasUniform("hasMaterial")) {
			glUniform1i(shader->getUniform("hasMaterial"), 1);
		}
		else if (shader->hasUniform("hasMaterial")) {
			glUniform1i(shader->getUniform("hasMaterial"), 0);
		}

		if (hasBones && shader->hasUniform("hasBones")) {
			glUniform1i(shader->getUniform("hasBones"), 1);
		}
		else if (shader->hasUniform("hasBones")) {
			glUniform1i(shader->getUniform("hasBones"), 0);
		}
	}

	void clearProgFlags(shared_ptr<Program> shader) {
		//shader->bind();
		if (shader->hasUniform("hasTexAlb")) glUniform1i(shader->getUniform("hasTexAlb"), 0);
		if (shader->hasUniform("hasTexSpec")) glUniform1i(shader->getUniform("hasTexSpec"), 0);
		if (shader->hasUniform("hasTexRough")) glUniform1i(shader->getUniform("hasTexRough"), 0);
		if (shader->hasUniform("hasTexMet")) glUniform1i(shader->getUniform("hasTexMet"), 0);
		if (shader->hasUniform("hasTexNor")) glUniform1i(shader->getUniform("hasTexNor"), 0);
		if (shader->hasUniform("hasTexEmit")) glUniform1i(shader->getUniform("hasTexEmit"), 0);
		if (shader->hasUniform("hasMaterial")) glUniform1i(shader->getUniform("hasMaterial"), 0);
		if (shader->hasUniform("hasBones")) glUniform1i(shader->getUniform("hasBones"), 0);
		//shader->unbind();
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

	void initGround() {
		// Check if already initialized
		if (quad_VertexArrayID != 0) {
			cout << "Warning: initGround() called more than once." << endl;
			return;
		}
		// Ground plane from -Config::GROUND_SIZE to +Config::GROUND_SIZE in X and Z at Config::GROUND_HEIGHT
		float GrndPos[] = {
			-Config::GROUND_SIZE, Config::GROUND_HEIGHT, -Config::GROUND_SIZE, // top-left
			-Config::GROUND_SIZE, Config::GROUND_HEIGHT,  Config::GROUND_SIZE, // bottom-left
			 Config::GROUND_SIZE, Config::GROUND_HEIGHT,  Config::GROUND_SIZE, // bottom-right
			 Config::GROUND_SIZE, Config::GROUND_HEIGHT, -Config::GROUND_SIZE  // top-right
		};
		// Normals point straight up
		float GrndNorm[] = {
			0, 1, 0,   0, 1, 0,   0, 1, 0,   0, 1, 0
		};
		// Indices for two triangles covering the quad
		unsigned short idx[] = { 0, 1, 2,   0, 2, 3 };
		g_GiboLen = 6; // Number of indices

		// Generate VAO
		glGenVertexArrays(1, &quad_VertexArrayID);
		glBindVertexArray(quad_VertexArrayID);

		// Position buffer (Attribute 0)
		glGenBuffers(1, &GrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Normal buffer (Attribute 1)
		glGenBuffers(1, &GrndNorBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glGenBuffers(1, &GIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

		// Unbind VAO and buffers (good practice)
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		cout << "Ground Initialized: VAO ID " << quad_VertexArrayID << endl;
	}

	// Draw the ground sections (library, boss area, path)
	void drawGroundSections(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model) {
		if (!shader || !Model || quad_VertexArrayID == 0) return;

		shader->bind();

		bool isShadowShader = (shader == ShadowProg);

		if (isShadowShader) setProgFlags(shader, true, false); // materials, no bones

		glBindVertexArray(quad_VertexArrayID); // Bind ground VAO

		// Draw Boss Area Ground
		Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(bossAreaCenter); // Position the boss ground plane
		setModel(shader, Model);
		if (isShadowShader) SetMaterial(shader, Material::bronze);
		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);
		Model->popMatrix();

		// Unbind VAO after drawing all ground parts
		glBindVertexArray(0);

		if (isShadowShader) clearProgFlags(shader); // Clear shader flags

		shader->unbind(); // Unbind the simple shader
	}

	void initLibGrnd(float length, float width, float height, vec3 center_pos,
		GLuint &LibGrndVertexArrayID, GLuint &LibGrndBuffObj, GLuint &LibGrndNormBuffObj, GLuint &LibGrndIndxBuffObj, GLuint &LibGrndTexBuffObj, int &g_GiboLen) {
		// Define vertices for the library ground
		float LibGrndPos[] = {
			center_pos.x - length / 2, center_pos.y, center_pos.z - width / 2,
			center_pos.x - length / 2, center_pos.y, center_pos.z + width / 2,
			center_pos.x + length / 2, center_pos.y, center_pos.z + width / 2,
			center_pos.x + length / 2, center_pos.y, center_pos.z - width / 2
		};

		// Normals point straight up
		float LibGrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
		};

		float LibGrndTex[] = {
			0, 0,
			0, 1,
			1, 1,
			1, 0,
		};

		// Indices for two triangles covering the quad
		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };
		g_GiboLen = 6; // Number of indices

		// Generate VAO
		glGenVertexArrays(1, &LibGrndVertexArrayID);
		glBindVertexArray(LibGrndVertexArrayID);

		// Position buffer (Attribute 0)
		glGenBuffers(1, &LibGrndBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, LibGrndBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(LibGrndPos), LibGrndPos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Normal buffer (Attribute 1)
		glGenBuffers(1, &LibGrndNormBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, LibGrndNormBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(LibGrndNorm), LibGrndNorm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Texture buffer (Attribute 2)
		glGenBuffers(1, &LibGrndTexBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, LibGrndTexBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(LibGrndTex), LibGrndTex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glGenBuffers(1, &LibGrndIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LibGrndIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

		// Unbind VAO and buffers (good practice)
		glBindVertexArray(0);
	}

	void addLibGrnd(float length, float width, float height, vec3 center_pos, shared_ptr<Texture> tex) {
		LibGrndObjKey key{ center_pos, height };

		// Check if already initialized
		if (libraryGroundKeys.count(key)) {
			return;
		}

		LibGrndObject newLibGrnd;
		newLibGrnd.length = length;
		newLibGrnd.width = width;
		newLibGrnd.height = height;
		newLibGrnd.center_pos = center_pos;
		newLibGrnd.texture = tex;

		initLibGrnd(length, width, height, center_pos,
			newLibGrnd.VAO, newLibGrnd.BuffObj, newLibGrnd.NorBuffObj,
			newLibGrnd.IndxBuffObj, newLibGrnd.TexBuffObj, newLibGrnd.GiboLen);

		libraryGrounds.push_back(newLibGrnd);
		libraryGroundKeys.insert(key); // Add key to set to avoid duplicates
	}

	void drawLibGrnd(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model) return; // safety check

		shader->bind(); // Bind the simple shader

		if (secondPass) setProgFlags(shader, true, false); // materials, no bones

		for (const auto& libGrnd : libraryGrounds) {
			glBindVertexArray(libGrnd.VAO); // Bind each library ground VAO

			if (secondPass) {
				libGrnd.texture->bind(shader->getUniform("TexAlb")); // Bind the texture
				glUniform1i(shader->getUniform("hasTexAlb"), 0); // Set texture uniform
			}

			Model->pushMatrix();
			Model->loadIdentity();
			setModel(shader, Model);

			if (secondPass) SetMaterial(shader, Material::silver); // set the material

			glDrawElements(GL_TRIANGLES, libGrnd.GiboLen, GL_UNSIGNED_SHORT, 0);
			Model->popMatrix();

			if (secondPass) libGrnd.texture->unbind(); // Unbind the texture after drawing each library ground
		}

		glBindVertexArray(0); // Unbind VAO after drawing all library grounds

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind(); // Unbind the simple shader
	}

	void initWall(float length, vec3 pos, vec3 dir, float height,
	GLuint &WallVertexArrayID, GLuint &WallBuffObj, GLuint &WallNormBuffObj, GLuint &WIndxBuffObj, GLuint &WallTexBuffObj, int &w_GiboLen) {
		vec3 dirNorm = normalize(dir);

		// Define border vertices
		// positioned relative to the bottom-left corner of the border
		float WallPos[] = {
			pos.x, pos.y, pos.z, // bottom-left
			pos.x + dirNorm.x * length, pos.y, pos.z + dirNorm.z * length, // bottom-right
			pos.x + dirNorm.x * length, pos.y + height, pos.z + dirNorm.z * length, // top-right
			pos.x, pos.y + height, pos.z // top-left
		};

		// Normals face outward
		float WallNorm[] = {
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1
		};

		// float WallTex[] = {
		// 	0, 0,
		// 	1, 0,
		// 	1, 1,
		// 	0, 1
		// };

		// Repeating wall texture
		float texRepeatPerUnit = 0.2f;

		// Compute number of times to repeat the texture
		float texRepeatX = length * texRepeatPerUnit;
		float texRepeatY = height * texRepeatPerUnit;

		float WallTex[] = {
			0.0f,         0.0f,
			texRepeatX,   0.0f,
			texRepeatX,   texRepeatY,
			0.0f,         texRepeatY
		};

		unsigned short idx[] = { 0, 1, 2, 0, 2, 3 };
		w_GiboLen = 6; // Number of indices

		// Generate VAO
		glGenVertexArrays(1, &WallVertexArrayID);
		glBindVertexArray(WallVertexArrayID);

		// Position buffer (Attribute 0)
		glGenBuffers(1, &WallBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, WallBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(WallPos), WallPos, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Normal buffer (Attribute 1)
		glGenBuffers(1, &WallNormBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, WallNormBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(WallNorm), WallNorm, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Texture buffer (Attribute 2)
		glGenBuffers(1, &WallTexBuffObj);
		glBindBuffer(GL_ARRAY_BUFFER, WallTexBuffObj);
		glBufferData(GL_ARRAY_BUFFER, sizeof(WallTex), WallTex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glGenBuffers(1, &WIndxBuffObj);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, WIndxBuffObj);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

		// Unbind VAO and buffers (good practice)
		glBindVertexArray(0);
	}

	void addWall(float length, vec3 pos, vec3 dir, float height, shared_ptr<Texture> tex) {
		WallObjKey posKey{ pos, dir, height };
		if (borderWallKeys.count(posKey)) {
			return;
		}


		WallObject newBorder;
		newBorder.length = length;
		newBorder.position = pos;
		newBorder.direction = dir;
		newBorder.height = height;
		newBorder.texture = tex;

		initWall(length, pos, dir, height,
			newBorder.WallVAID, newBorder.BuffObj, newBorder.NorBuffObj,
			newBorder.IndxBuffObj, newBorder.TexBuffObj, newBorder.GiboLen);

		borderWalls.push_back(newBorder);
		borderWallKeys.insert(posKey); // Add key to set to avoid duplicates
	}

	void initEnemies() {
		if (enemies.size() == 0) {
			std::vector<vec3> enemySpawnPositions = library->getEnemySpawnPositions();

			for (auto e = enemies.begin(); e != enemies.end(); ++e) {
				enemies.erase(e);
			}

			for (const auto& spawnPos : enemySpawnPositions) {
				enemies.push_back(new IceElemental(vec3(spawnPos.x, Config::ICE_ELEMENTAL_TRANS_Y, spawnPos.z), ENEMY_HP_MAX, 2.0f, iceElemental, vec3(1.0f), vec3(0.0f)));
				// cout << " Enemy placed at: (" << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << ")" << endl;
			}
		}
	}

	void drawBorderWalls(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model) return; // safety checks

		shader->bind(); // Bind the shader

		if (secondPass) setProgFlags(shader, false, false); // no material, no bones

		for (const auto& border : borderWalls) {
			glBindVertexArray(border.WallVAID); // Bind each border VAO
			
			if (secondPass) { // For shadow shader, explicitly set texture unit
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, border.texture->getID());
				glUniform1i(shader->getUniform("TexAlb"), 0);
				glUniform1i(shader->getUniform("hasTexAlb"), 1); // Set texture uniform

			}

			Model->pushMatrix();
			Model->loadIdentity();
			setModel(shader, Model);
			// SetMaterial(shader, Material::black); // set the material
			glDrawElements(GL_TRIANGLES, border.GiboLen, GL_UNSIGNED_SHORT, 0);
			Model->popMatrix();

			if (secondPass) border.texture->unbind(); // Unbind the texture after drawing each border
		}

		glBindVertexArray(0); // Unbind VAO after drawing all borders

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind(); // Unbind the simple shader
	}

	void drawPlayer(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, float animTime, bool secondPass) {
		if (!curS || !Model || !stickfigure_running || !stickfigure_animator) return;

		curS->bind();

		if (secondPass) setProgFlags(curS, false, true); // no material, bones for animation

		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(player->getPosition());
			Model->rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
			Model->rotate(player->getRotY() + 3.14f, vec3(0, 0, 1));
			Model->scale(1.0f);

			setModel(curS, Model);
			stickfigure_running->Draw(curS);

			if (secondPass) clearProgFlags(curS); // Clear shader flags

			curS->unbind();

			if (secondPass) drawParticles(particleSystem, particleProg, Model); // draw particles if full scene render
			
		Model->popMatrix();
	}

	void drawBooks(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model) return; // Check if shader and model stack are valid

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		for (const auto& book : books) {
			// Common values for book halves
			float halfThickness = book.scale.z * 0.5f;
			glm::vec3 halfScaleVec = glm::vec3(book.scale.x, book.scale.y, halfThickness);

			// Set Material properties - check for uniform existence first
			if (secondPass) SetMaterial(shader, Material::brown);

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

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	//TODO: Add particle effects to orbs
	void drawOrbs(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		// --- Collision Check Logic ---
		for (auto& orb : orbCollectibles) {
			// Perform collision check ONLY if not collected AND in the IDLE state
			if (!orb.collected && orb.state == OrbState::IDLE) {//&& // <<<--- ADD STATE CHECK
				if (Config::DEBUG_ORB_PICKUP) {
					cout << "player position min: " << (playerAABB.min + player->getPosition()).x << " " << (playerAABB.min + player->getPosition()).y << " " << (playerAABB.min + player->getPosition()).z << endl;
					cout << "player position max: " << (playerAABB.max + player->getPosition()).x << " " << (playerAABB.max + player->getPosition()).y << " " << (playerAABB.max + player->getPosition()).z << endl;
					cout << "orb position min: " << orb.AABBmin.x << " " << orb.AABBmin.y << " " << orb.AABBmin.z << endl;
					cout << "orb position max: " << orb.AABBmax.x << " " << orb.AABBmax.y << " " << orb.AABBmax.z << endl;
				}
				if (checkAABBCollision(playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition(), orb.AABBmin, orb.AABBmax)) {
					orb.collected = true;
					orb.state = OrbState::COLLECTED;
					orbsCollectedCount++;
					std::cout << "Collected a Spell Orb! (" << orbsCollectedCount << ")\n";
				}
			}
		}

		// --- Drawing Logic ---
		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		int collectedOrbDrawIndex = 0;

		for (auto& orb : orbCollectibles) {

			glm::vec3 currentDrawPosition;
			float currentDrawScale = orb.scale; // Use base scale

			if (orb.collected) {
				// Calculate position behind the player
				float backOffset = 0.4f;
				float upOffsetBase = 0.6f;
				float stackOffset = orb.scale * 2.5f;
				float sideOffset = 0.15f;
				glm::vec3 playerForward = normalize(manMoveDir);
				glm::vec3 playerUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 playerRight = normalize(cross(playerForward, playerUp));
				float currentUpOffset = upOffsetBase + (collectedOrbDrawIndex * stackOffset);
				float currentSideOffset = (collectedOrbDrawIndex % 2 == 0 ? -sideOffset : sideOffset);
				currentDrawPosition = player->getPosition() - playerForward * backOffset
					+ playerUp * currentUpOffset
					+ playerRight * currentSideOffset;
				collectedOrbDrawIndex++;
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
				if (secondPass) SetMaterial(shader, orb.material); // Set material for shadow shader
				setModel(shader, Model);
				orb.model->Draw(shader);
			Model->popMatrix();
		} // End drawing loop

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	void drawCat(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!CatWizard) return; //Need Cat Model

		shader->bind(); // Texture

		if (secondPass) setProgFlags(shader, false, false); // no material, no bones

		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 0.0f, 0.0f)); // Position at origin
			//Model->scale(vec3(0.25f));
			Model->rotate(glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
			setModel(shader, Model);
			CatWizard->Draw(shader);
		Model->popMatrix();
		if (secondPass) clearProgFlags(shader); // Clear shader flags
		shader->unbind();

	}

	void checkAllEnemies() {
		if (canFightboss) return; // Already set to true
		allEnemiesDead = true; // Assume all are dead unless we find one alive
		for (const auto* enemy : enemies) {
			if (enemy && enemy->isAlive()) {
				allEnemiesDead = false; // Found at least one alive enemy
				break;
			}
		}
		if (allEnemiesDead) {
			canFightboss = true; // All enemies are dead, boss can be fought
		}
	}

	void checkBossfight() {
		if (canFightboss && !bossfightended && bossEnemy->isAlive()) {
			int i = bossRoom->mapXtoGridX(player->getPosition().x);
			int j = bossRoom->mapZtoGridY(player->getPosition().z);
			if (bossGrid.inBounds(glm::ivec2(i, j))) {

				if (bossRoom->isInsideBossArea(glm::ivec2(i, j))) {
					bossfightstarted = true; // Player is in the boss area
				}
			}
		} else if (canFightboss && bossfightstarted && !bossEnemy->isAlive()) {
			bossfightstarted = false; // Player is no longer in the boss area
			bossfightended = true; // Boss fight ended
			bossActiveSpells.clear(); // Clear active spells
			canFightboss = false; // Reset boss fight flag
		}
	}

	void restartGeneration() {
		if (restartGen) {
			restartGen = false; // Reset flag after reinitialization
			canFightboss = false; // Reset boss fight flag
			allEnemiesDead = false; // Reset enemy status
			player->setPosition(vec3(0.0f, 0.0f, 0.0f)); // Reset player position
			books.clear();
			libraryGrounds.clear();
			libraryGroundKeys.clear();
			borderWalls.clear();
			borderWallKeys.clear();
			orbCollectibles.clear();
			keyCollectibles.clear();
			if (!player->isAlive()) {
				player->resetHitpoints();
				player->setAlive(); // Reset player status to alive if had died
				canFightboss = false; // Flag to check if the player can fight the boss
				allEnemiesDead = false; // Flag to check if all enemies are dead
				restartGen = false;
				bossfightstarted = false;
				bossfightended = false;
			}

			bossEnemy->setAlive(); // Reset boss status to alive
			initMapGen();
			initEnemies(); // Reinitialize enemies
			bossActiveSpells.clear();
			// enemies.push_back(new Enemy(libraryCenter + vec3(-5.0f, 0.8f, 8.0f), 50.0f, 2.0f, sphere, glm::vec3(0.5f, 1.28f, 0.5f), vec3(0.0f))); // <<-- Pass sphere and scale
			activeSpells.clear(); // Clear active spells
		}
	}

	void drawEnemies(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model || !sphere) return; // safety checks

		shader->bind(); // bind shader

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		for (const auto* enemy : enemies) {
			if (!enemy || !enemy->isAlive()) {
				keyCollectibles.emplace_back(key, enemy->getPosition(), 0.1f, Material::gold);
				drawKey(shader, Model, secondPass);
				continue; // Skip null or dead enemies
			}

			glm::vec3 enemyPos = enemy->getPosition();

			// --- Draw Main Body (Pill Shape) ---
			Model->pushMatrix();
			{
				Model->translate(enemyPos);
				// Scale for pill shape ( taller in Y, squished in X/Z )
				Model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
				Model->rotate(enemy->getRotY(), glm::vec3(0, 1, 0));
				// rotate 90 around z
				Model->rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0));

				if (secondPass) SetMaterial(shader, Material::blue_body);

				if (secondPass) glUniform1f(shader->getUniform("enemyAlpha"), enemy->getDamageTimer() / Config::ENEMY_HIT_DURATION);

				setModel(shader, Model);
				iceElemental->Draw(shader); // Draw the scaled sphere as the body
			}
			Model->popMatrix();
		} // End loop through enemies

		if (secondPass) {
			glUniform1f(shader->getUniform("enemyAlpha"), 1.0f); // reset enemyAlpha
			clearProgFlags(shader); // Clear shader flags
		}

		shader->unbind();
	}

	void drawLibrary(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool cullFlag, bool secondPass) {
		if (!shader || !Model || !book_shelf1 || grid.getSize().x == 0 || grid.getSize().y == 0) return; // Safety checks

		shader->bind();
		
		if (secondPass) {
			setProgFlags(shader, true, false); // material, no bones
			SetMaterial(shader, Material::grey);
		}

		float gridWorldWidth = Config::GROUND_SIZE * 2.0f; // The world space the grid should occupy (library floor width)
		float gridWorldDepth = Config::GROUND_SIZE * 2.0f; // The world space the grid should occupy (library floor depth)
		float cellWidth = gridWorldWidth / (float)grid.getSize().x;
		float cellDepth = gridWorldDepth / (float)grid.getSize().y;
		float shelfScaleFactor = 1.8f; // Adjust scale of the bookshelf model itself

		for (int z = 0; z < grid.getSize().y; ++z) {
			for (int x = 0; x < grid.getSize().x; ++x) {
				glm::ivec2 gridPos(x, z);
				float i = library->mapGridXtoWorldX(x); // Center the shelf in the cell
				float j = library->mapGridYtoWorldZ(z); // Center the shelf in the cell
				if (!cullFlag || !ViewFrustCull(glm::vec3(i, 0, j), 2.0f, planes)) {
					if (grid[gridPos].type == LibraryGen::CellType::CLUSTER) {
						if (grid[gridPos].clusterType == LibraryGen::ClusterType::SHELF1) {
							Model->pushMatrix();
							Model->loadIdentity();
							// Model->translate(vec3(worldX, libraryCenter.y, worldZ)); // Position shelf at cell center on ground
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(2.0f));
							setModel(shader, Model);
							book_shelf1->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::SHELF2) {
							// Calculate world position based on grid cell, centering the grid on libraryCenter

							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(1.0f));
							setModel(shader, Model);
							book_shelf1->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::SHELF3) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->rotate(glm::radians(90.0f), vec3(0, 1, 0)); // Rotate for left/right walls
							Model->scale(vec3(2.0f));
							setModel(shader, Model);
							book_shelf1->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::ONLY_CANDELABRA) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(0.5f));
							setModel(shader, Model);
							candelabra->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::ONLY_CHEST) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(0.25f));
							setModel(shader, Model);
							chest->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::ONLY_TABLE) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(0.35f));
							setModel(shader, Model);
							table_chairs1->Draw(shader);
							Model->popMatrix();

							addLibGrnd(5.0f, 5.0f, 1.0f, vec3(i, libraryCenter.y + 0.1f, j), carpetTex);
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::ONLY_CLOCK) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position wall at cell center on ground
							Model->scale(vec3(0.5f));
							setModel(shader, Model);
							grandfather_clock->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::LAYOUT1) {
							if (grid[gridPos].objectType == LibraryGen::CellObjType::BOOKSHELF) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(2.0f)); // Scale set in grid members
								setModel(shader, Model);
								book_shelf1->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::ROTATED_BOOKSHELF) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->rotate(glm::radians(90.0f), vec3(0, 1, 0)); // Rotate for left/right walls
								Model->scale(vec3(2.0f)); // Scale set in class members
								setModel(shader, Model);
								book_shelf1->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::TABLE_AND_CHAIR2) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(0.35f)); // Scale set in class members
								setModel(shader, Model);
								table_chairs1->Draw(shader);
								Model->popMatrix();

								addLibGrnd(5.0f, 5.0f, 1.0f, vec3(i, libraryCenter.y + 0.1f, j), carpetTex);

							}else if (grid[gridPos].objectType == LibraryGen::CellObjType::TABLE_AND_CHAIR1) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(0.35f)); // Scale set in class members
								setModel(shader, Model);
								table_chairs1->Draw(shader);
								Model->popMatrix();

								addLibGrnd(5.0f, 5.0f, 1.0f, vec3(i, libraryCenter.y + 0.1f, j), carpetTex);
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::CANDELABRA) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(0.5f)); // Scale set in class members
								setModel(shader, Model);
								candelabra->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::GRANDFATHER_CLOCK) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(0.5f)); // Scale set in class members
								setModel(shader, Model);
								grandfather_clock->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::CHEST) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(0.25f)); // Scale set in class members
								setModel(shader, Model);
								chest->Draw(shader);
								Model->popMatrix();
							}
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::ONLY_BOOKSTAND) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
							Model->scale(vec3(0.75f)); // Scale set in class members
							setModel(shader, Model);
							bookstand->Draw(shader);
							Model->popMatrix();
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::GLOWING_SHELF1) {
							if (grid[gridPos].objectType == LibraryGen::CellObjType::SHELF_WITH_ABILITY) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(2.0f)); // Scale set in class members
								setModel(shader, Model);
								book_shelf2->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::BOOKSHELF) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->scale(vec3(2.0f)); // Scale set in class members
								setModel(shader, Model);
								book_shelf1->Draw(shader);
								Model->popMatrix();
							}
						} else if (grid[gridPos].clusterType == LibraryGen::ClusterType::GLOWING_SHELF2) {
							if (grid[gridPos].objectType == LibraryGen::CellObjType::SHELF_WITH_ABILITY_ROTATED) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->rotate(glm::radians(90.0f), vec3(0, 1, 0)); // Rotate for left/right walls
								Model->scale(vec3(2.0f)); // Scale set in class members
								setModel(shader, Model);
								book_shelf2->Draw(shader);
								Model->popMatrix();
							} else if (grid[gridPos].objectType == LibraryGen::CellObjType::ROTATED_BOOKSHELF) {
								Model->pushMatrix();
								Model->loadIdentity();
								Model->translate(vec3(i, libraryCenter.y, j)); // Position shelf at cell center on ground
								Model->rotate(glm::radians(90.0f), vec3(0, 1, 0)); // Rotate for left/right walls
								Model->scale(vec3(2.0f)); // Scale set in class members
								setModel(shader, Model);
								book_shelf1->Draw(shader);
								Model->popMatrix();
							}
						}
					}
				}
			}
		}

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	void drawBossRoom(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool cullFlag, bool secondPass) {
		if (!shader || !Model) return;
		shader->bind();
		
		if (secondPass) setProgFlags(shader, false, false); // no material, no bones

		for (int z = 0; z < bossGrid.getSize().y; ++z) {
			for (int x = 0; x < bossGrid.getSize().x; ++x) {
				glm::ivec2 gridPos(x, z);
				float i = bossRoom->mapGridXtoWorldX(x); // Center the shelf in the cell
				float j = bossRoom->mapGridYtoWorldZ(z); // Center the shelf in the cell
				if (!cullFlag || !ViewFrustCull(glm::vec3(i, 0, j), 2.0f, planes)) {
					if (bossGrid[gridPos].type == BossRoomGen::CellType::BORDER) {
						int test = bossRoom->mapXtoGridX(i);
						int test2 = bossRoom->mapZtoGridY(j);

						if (!debug_shelf) {
							std::cout << "Shelf Position in Grid: (" << x << ", " << z << ")" << std::endl;
							std::cout << "Shelf Position in World: (" << i << ", " << libraryCenter.y << ", " << j << ")" << std::endl;
							debug_shelf = true; // Set to true to avoid spamming the console
							std::cout << "Redo Grid Position: (" << test << ", " << test2 << ")" << std::endl;
						}

						Model->pushMatrix();
						Model->loadIdentity();
						Model->translate(vec3(i, libraryCenter.y, j)); // Position set in class members
						Model->rotate(glm::radians(bossGrid[gridPos].transformData.rotation), vec3(0, 1, 0)); // Rotate for left/right walls
						Model->scale(2.5f);
						setModel(shader, Model);
						book_shelf1->Draw(shader); // Use the bookshelf model for the border
						Model->popMatrix();
					} else if (bossGrid[gridPos].type == BossRoomGen::CellType::ENTRANCE) {
						if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::ENTRANCE_MIDDLE) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, 0, j));
							Model->rotate(glm::radians(bossGrid[gridPos].transformData.rotation), vec3(0, 1, 0)); // Rotate for left/right walls
							Model->scale(1.0f);
							setModel(shader, Model);
							if(unlocked == false){
								door->Draw(shader); // Use the door model for the entrance
							}

							Model->popMatrix();
						} else if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::ENTRANCE_SIDE) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, 0, j));
							Model->rotate(glm::radians(bossGrid[gridPos].transformData.rotation), vec3(0, 1, 0)); // Rotate for left/right walls
							Model->scale(1.0f);
							setModel(shader, Model);
							book_shelf1->Draw(shader); // Use the door model for the entrance
							Model->popMatrix();
						}
					} else if (bossGrid[gridPos].type == BossRoomGen::CellType::EXIT) {
						if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::EXIT_MIDDLE) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, 0, j));
							Model->rotate(glm::radians(bossGrid[gridPos].transformData.rotation), vec3(0, 1, 0)); // Rotate for left/right walls
							Model->scale(1.0f);
							setModel(shader, Model);
							door->Draw(shader); // Use the door model for the entrance
							Model->popMatrix();
						} else if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::EXIT_SIDE) {
							Model->pushMatrix();
							Model->loadIdentity();
							Model->translate(vec3(i, 0, j));
							Model->rotate(glm::radians(bossGrid[gridPos].transformData.rotation), vec3(0, 1, 0)); // Rotate for left/right walls
							Model->scale(1.0f);
							setModel(shader, Model);
							book_shelf1->Draw(shader); // Use the door model for the entrance
							Model->popMatrix();
						}
					}
				}
			}
		}

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	void drawBossEnemy(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model || !bossEnemy) return; // Need boss enemy model (safety checks)

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		// --- Common Eye Parameters ---
		float bodyBaseScaleY = 0.8f; // Base height factor before pill stretch
		glm::vec3 eyeOffsetBase = glm::vec3(0.0f, bodyBaseScaleY * 0.4f, 0.45f); // Y up, Z forward from body center
		float eyeSeparation = 0.25f; // Distance between eye centers
		float whiteScale = 0.18f;
		float pupilScale = 0.1f;
		float pupilOffsetForward = 0.02f; // Push pupil slightly in front of white

		if (bossEnemy->isAlive()) {
			bossEnemy->lookAtPlayer(player->getPosition()); // Make the boss look at the player
			glm::vec3 bossPos = bossEnemy->getPosition() + glm::vec3(0, 0.8f, 0); // Position the boss slightly above the ground
			glm::vec3 bossRotation = bossEnemy->getRotation(); // Get rotation from the enemy object
			float bossRotY = bossEnemy->getRotY();

			Model->pushMatrix();
			{
				Model->loadIdentity(); // Reset the model matrix
				Model->translate(bossPos);
				Model->rotate(bossRotY, bossRotation); // Rotate the body to match the boss's rotation
				// --- Draw Main Body (Pill Shape) ---
				Model->pushMatrix();
				{
					// Model->translate(bossPos);
					// Scale for pill shape ( taller in Y, squished in X/Z )
					Model->scale(glm::vec3(0.5f, bodyBaseScaleY * 1.6f, 0.5f)); // Adjust scale factors as needed

					if (secondPass) SetMaterial(shader, Material::purple);

					setModel(shader, Model);
					sphere->Draw(shader); // Draw the scaled sphere as the body
				}
				Model->popMatrix();


				// --- Draw Eyes (Relative to Enemy Center) ---

				// Left Eye
				Model->pushMatrix();
				{
					// Go to enemy center, then offset to eye position
					// Model->translate(bossPos);
					Model->translate(eyeOffsetBase + glm::vec3(-eyeSeparation, 0, 0));

					// White Part
					Model->pushMatrix();
					{
						Model->scale(glm::vec3(whiteScale));

						if (secondPass) SetMaterial(shader, Material::eye_white);

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

						if (secondPass) SetMaterial(shader, Material::black);

						setModel(shader, Model);
						sphere->Draw(shader);
					}
					Model->popMatrix(); // Pop pupil transform
				}
				Model->popMatrix(); // Pop left eye transform


				// Right Eye (Similar to Left)
				Model->pushMatrix();
				{
					// Model->translate(bossPos);
					Model->translate(eyeOffsetBase + glm::vec3(+eyeSeparation, 0, 0)); // Offset to the right

					// White Part
					Model->pushMatrix();
					{
						Model->scale(glm::vec3(whiteScale));

						if (secondPass) SetMaterial(shader, Material::eye_white);

						setModel(shader, Model);
						sphere->Draw(shader);
					}
					Model->popMatrix();

					// Pupil Part
					Model->pushMatrix();
					{
						Model->translate(glm::vec3(0, 0, whiteScale * 0.5f + pupilOffsetForward));
						Model->scale(glm::vec3(pupilScale));

						if (secondPass) SetMaterial(shader, Material::black);

						setModel(shader, Model);
						sphere->Draw(shader);
					}
					Model->popMatrix();
				}
				Model->popMatrix(); // Pop right eye transform
			}
			Model->popMatrix(); // Pop boss body transform
		}
		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	void drawDoor(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model || !cube) return; // Need cube model

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		Model->pushMatrix();
		Model->loadIdentity();
		Model->translate(doorPosition); // Position set in class members
		Model->scale(doorScale);      // Scale set in class members

		if (secondPass) SetMaterial(shader, Material::wood);
		setModel(shader, Model);
		cube->Draw(shader);

		Model->popMatrix();

		if (secondPass) clearProgFlags(shader); // Clear shader flags

		shader->unbind();
	}

	bool checkAABBCollision(const glm::vec3& minA, const glm::vec3& maxA,
		const glm::vec3& minB, const glm::vec3& maxB) {
		return (minA.x <= maxB.x && maxA.x >= minB.x) &&
			(minA.y <= maxB.y && maxA.y >= minB.y) &&
			(minA.z <= maxB.z && maxA.z >= minB.z);
	}

	bool checkSphereCollision(const glm::vec3& spherePos, float sphereRadius,
		const glm::vec3& boxMin, const glm::vec3& boxMax)
	{
		glm::vec3 closestPoint = glm::clamp(spherePos, boxMin, boxMax);
		glm::vec3 distanceVec = spherePos - closestPoint;
		return glm::length(distanceVec) <= sphereRadius;
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
		float interactionRadius = 5.0f;
		float interactionRadiusSq = interactionRadius * interactionRadius;

		float gridWorldWidth = Config::GROUND_SIZE * 2.0f;
		float gridWorldDepth = Config::GROUND_SIZE * 2.0f;
		float cellWidth = gridWorldWidth / (float)grid.getSize().x;
		float cellDepth = gridWorldDepth / (float)grid.getSize().y;

		bool interacted = false;

		for (int z = 0; z < grid.getSize().y && !interacted; ++z) {
			for (int x = 0; x < grid.getSize().x && !interacted; ++x) {
				glm::ivec2 gridPos(x, z);
				if (grid[gridPos].objectType == LibraryGen::CellObjType::SHELF_WITH_ABILITY || grid[gridPos].objectType == LibraryGen::CellObjType::SHELF_WITH_ABILITY_ROTATED) {
					// float shelfWorldX = libraryCenter.x - gridWorldWidth * 0.5f + (x + 0.5f) * cellWidth;
					// float shelfWorldZ = libraryCenter.z - gridWorldDepth * 0.5f + (z + 0.5f) * cellDepth;
					float shelfWorldX = library->mapGridXtoWorldX(x); // Center the shelf in the cell
					float shelfWorldZ = library->mapGridYtoWorldZ(z); // Center the shelf in the cell
					glm::vec3 shelfCenterPos = glm::vec3(shelfWorldX, Config::GROUND_HEIGHT + 1.0f, shelfWorldZ);

					// glm::vec3 diff = shelfCenterPos - characterMovement;
					glm::vec3 diff = shelfCenterPos - player->getPosition();
					diff.y = 0.0f; // Ignore Y difference for interaction distance
					float distSq = dot(diff, diff); // Use dot product for squared distance

					if (distSq <= interactionRadiusSq) {

						// --- ADJUST Spawn Height ---
						float minSpawnHeight = 1.8f; // Minimum height above Config::GROUND_HEIGHT
						float maxSpawnHeight = 2.8f; // Maximum height above Config::GROUND_HEIGHT
						float spawnHeight = Config::GROUND_HEIGHT + Config::randFloat(minSpawnHeight, maxSpawnHeight); // <-- ADJUSTED height range

						glm::vec3 spawnPos = glm::vec3(shelfWorldX, spawnHeight, shelfWorldZ);

						glm::vec3 bookScale = glm::vec3(0.7f, 0.9f, 0.2f);
						glm::quat bookOrientation = glm::angleAxis(glm::radians(Config::randFloat(-10.f, 10.f)), glm::vec3(0, 1, 0));

						// Select one of the 3 orb materials
						Material orbColor;
						int randomChoice = Config::randInt(0, 2);
						switch (randomChoice) {
						case 0:
							orbColor = Material::orb_glowing_blue;
							break;
						case 1:
							orbColor = Material::orb_glowing_red;
							break;
						case 2:
							orbColor = Material::orb_glowing_yellow;
							break;
						default:
							orbColor = Material::orb_glowing_blue; // Fallback
						}

						books.emplace_back(cube, sphere, spawnPos, bookScale, bookOrientation, orbColor);

						Book& newBook = books.back();

						// --- PASS Player Position to startFalling ---
						// newBook.startFalling(Config::GROUND_HEIGHT, characterMovement); // <<-- MODIFIED call
						newBook.startFalling(Config::GROUND_HEIGHT, player->getPosition());

						interacted = true;
						break;
					}
				}
			}
		}

		if (interacted) {
			cout << "Book spawned and falling." << endl;
		}
		else {
			cout << "No shelf nearby to interact with." << endl;
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
		int screenWidth, screenHeight;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenWidth, &screenHeight);
		// TODO: Add enemy movement, AI, attack logic later
		for (auto* enemy : enemies) enemy->update(player.get(), deltaTime);
	}

	// Player Collision Calc
	void calculatePlayerLocalAABB() {
		// Get base AABB from player model
		glm::vec3 baseMin = stickfigure_running->getBoundingBoxMin();
		glm::vec3 baseMax = stickfigure_running->getBoundingBoxMax();

		// Apply the player's base scale
		playerAABB.min = baseMin * manScale.x;
		playerAABB.max = baseMax * manScale.x;

		// Alin box with ground
		float yOffset = 0.0f - playerAABB.min.y;
		playerAABB.min.y += yOffset;
		playerAABB.max.y = 2.0f;

		if (Config::DEBUG_PLAYER_AABB) {
			cout << "Calculated Player AABB Min: (" << playerAABB.min.x << "," << playerAABB.min.y << "," << playerAABB.min.z << ")" << endl;
			cout << "Calculated Player AABB Max: (" << playerAABB.max.x << "," << playerAABB.max.y << "," << playerAABB.max.z << ")" << endl;
		}
	}

	// Collision Checking Helper
	bool checkCollisionAt(const glm::vec3& checkPos, const glm::quat& playerOrientation) {
		if (!book_shelf1 || grid.getSize().x == 0) return false; // safety checks

		// first, collide against any border walls
		for (const auto& w : borderWalls) {
			// compute the two corners of the wall�s rectangle
			glm::vec3 p0 = w.position;
			glm::vec3 p1 = w.position + w.direction * w.length;
			glm::vec3 minCorner = glm::min(p0, p1);
			glm::vec3 maxCorner = glm::max(p0, p1) + glm::vec3(0.0f, w.height, 0.0f);
			if (checkAABBCollision(
				playerAABB.min + player->getPosition(),
				playerAABB.max + player->getPosition(),
				minCorner, maxCorner)) return true;
		}

		// Iterate through grid for shelves
		float gridWorldWidth = Config::GROUND_SIZE * 2.0f;
		float gridWorldDepth = Config::GROUND_SIZE * 2.0f;
		float cellWidth = gridWorldWidth / (float)grid.getSize().x;
		float cellDepth = gridWorldDepth / (float)grid.getSize().y;
		// Use the same scale factor as drawLibrary
		float shelfScaleFactor = 1.8f;
		glm::vec3 shelfVisScale = vec3(1.0f);
		// Get shelf model's local AABB
		glm::vec3 shelfLocalMin = book_shelf1->getBoundingBoxMin();
		glm::vec3 shelfLocalMax = book_shelf1->getBoundingBoxMax();
		// Apply visual scale to shelf local AABB for collision
		glm::vec3 collisionShelfLocalMin = shelfLocalMin * shelfVisScale;
		glm::vec3 collisionShelfLocalMax = shelfLocalMax * shelfVisScale;
		// Handle potential inversion if scale is negative (unlikely here)
		for (int i = 0; i < 3; ++i) {
			if (collisionShelfLocalMin[i] > collisionShelfLocalMax[i]) std::swap(collisionShelfLocalMin[i], collisionShelfLocalMax[i]);
		}

		// spatial detection for library grid
		int gridX = library->mapXtoGridX(checkPos.x);
		int gridZ = library->mapZtoGridY(checkPos.z);

		glm::ivec2 gridPos(gridX, gridZ);

		float gridtoworldX = library->mapGridXtoWorldX(gridPos.x); // check back against the specific world position
		float gridtoworldZ = library->mapGridYtoWorldZ(gridPos.y);

		if (grid.inBounds(glm::ivec2(gridX, gridZ))) {
			if (grid[gridPos].type == LibraryGen::CellType::CLUSTER) {
				glm::vec3 shelfPos = glm::vec3(gridtoworldX, libraryCenter.y, gridtoworldZ); // Base position on ground

				if (checkSphereCollision(shelfPos, 1.5f, playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition())) {
					std::cout << "[DEBUG] Collision DETECTED with shelf at grid (" << gridX << "," << gridZ << ")" << std::endl;
					return true; // Collision found
				}
			} else if (grid[gridPos].type == LibraryGen::CellType::BORDER) {
				return true; // Collision found
			}
		}

		// spatial detection for boss room grid
		gridX = bossRoom->mapXtoGridX(checkPos.x);
		gridZ = bossRoom->mapZtoGridY(checkPos.z);

		gridPos = glm::ivec2(gridX, gridZ);

		gridtoworldX = bossRoom->mapGridXtoWorldX(gridPos.x); // check back against the specific world position
		gridtoworldZ = bossRoom->mapGridYtoWorldZ(gridPos.y);

		if (bossGrid.inBounds(glm::ivec2(gridX, gridZ))) {

			if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::ENTRANCE_SIDE) {
				glm::vec3 pos = glm::vec3(gridtoworldX, libraryCenter.y, gridtoworldZ); // Base position on ground

				if (checkSphereCollision(pos, 2.0f, playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition())) {
					std::cout << "[DEBUG] Collision DETECTED with shelf at grid (" << gridX << "," << gridZ << ")" << std::endl;
					return true; // Collision found
				}
			}
		}

		gridX = bossRoom->mapXtoGridX(checkPos.x);
		gridZ = bossRoom->mapZtoGridY(checkPos.z);

		gridPos = glm::ivec2(gridX, gridZ);

		gridtoworldX = bossRoom->mapGridXtoWorldX(gridPos.x); // check back against the specific world position
		gridtoworldZ = bossRoom->mapGridYtoWorldZ(gridPos.y);

		if (bossGrid.inBounds(glm::ivec2(gridX, gridZ))) {
			if (bossGrid[gridPos].borderType == BossRoomGen::BorderType::ENTRANCE_SIDE) {
				glm::vec3 pos = glm::vec3(gridtoworldX, libraryCenter.y, gridtoworldZ); // Base position on ground

				if (checkSphereCollision(pos, 3.0f, playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition())) {
					std::cout << "[DEBUG] Collision DETECTED with shelf at grid (" << gridX << "," << gridZ << ")" << std::endl;
					return true; // Collision found
				}
			}
			// prevents entering the boss room
			else if ((bossGrid[gridPos].borderType == BossRoomGen::BorderType::ENTRANCE_MIDDLE && !canFightboss)) {
				glm::vec3 pos = glm::vec3(gridtoworldX, libraryCenter.y, gridtoworldZ); // Base position on ground
				if (checkSphereCollision(pos, 2.0f, playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition())) {
					std::cout << "[DEBUG] Collision DETECTED with shelf at grid (" << gridX << "," << gridZ << ")" << std::endl;
					return true; // Collision found
				}
			}
			else if (bossfightstarted && !bossRoom->isInsideBossArea(gridPos)) {
				return true;
			}
			// when boss is dead player is able to leave the boss room and will restart the generation
			else if ((bossfightended && !bossEnemy->isAlive() && bossGrid[gridPos].borderType == BossRoomGen::BorderType::EXIT_MIDDLE)) {
				bossfightended = false;
				restartGen = true;
				return true;
			}
		}
		return false; // No collision found
	}

	// handle player movement
	vec3 charMove() {
		float moveSpeed = 4.5f * AnimDeltaTime; // Use frame-rate independent speed
		vec3 desiredMoveDelta = vec3(0.0f);

		// Calculate desired movement direction based on input
		if (movingForward)  desiredMoveDelta += manMoveDir;
		if (movingBackward) desiredMoveDelta -= manMoveDir;
		if (movingLeft)     desiredMoveDelta -= right;
		if (movingRight)    desiredMoveDelta += right;

		// Normalize and scale movement delta
		float moveLength = length(desiredMoveDelta);
		if (moveLength > 0.0f) {
			desiredMoveDelta = (desiredMoveDelta / moveLength) * moveSpeed;
		}
		else {
			return player->getPosition();
		}

		vec3 currentPos = player->getPosition();
		vec3 nextPos = currentPos + desiredMoveDelta;
		nextPos.y = Config::GROUND_HEIGHT; // Keep player on the ground plane

		// Player orientation for AABB calculation
		glm::quat playerOrientation = glm::angleAxis(player->getRotY(), glm::vec3(0, 1, 0));

		// Sliding (Separate Axes)
		vec3 allowedPos = currentPos; // Start with current position

		// Try moving along X only
		vec3 nextPosX = vec3(nextPos.x, currentPos.y, currentPos.z);
		if (!checkCollisionAt(nextPosX, playerOrientation)) {
			allowedPos.x = nextPos.x; // Allow X movement
		}
		else {
			cout << "[DEBUG] X-Collision prevented." << endl;
		}


		// Try moving along Z only (starting from potentially updated X)
		vec3 nextPosZ = vec3(allowedPos.x, currentPos.y, nextPos.z); // Use allowedPos.x
		if (!checkCollisionAt(nextPosZ, playerOrientation)) {
			allowedPos.z = nextPos.z; // Allow Z movement
		}
		else {
			cout << "[DEBUG] Z-Collision prevented." << endl;
		}

		player->setPosition(vec3(allowedPos.x, Config::GROUND_HEIGHT, allowedPos.z)); // Update player position

		return player->getPosition(); // Return the final position
	}

	// Shoot a spell
	void shootSpell() {
		cout << "[DEBUG] shootSpell() called. Orbs: " << orbsCollectedCount << endl;
		if (orbsCollectedCount <= 0 || !sphere || !sphereAABBCalculated) {
			cout << "[DEBUG] Cannot shoot: No orbs or sphere model not ready." << endl;
			return; // Need orbs and the sphere model/AABB
		}

		// Consume an orb
		orbsCollectedCount--;
		// Remove visual orb logic... (find first collected orb and erase)
		for (auto it = orbCollectibles.begin(); it != orbCollectibles.end(); ++it) {
			if (it->collected) {
				orbCollectibles.erase(it);
				break;
			}
		}

		vec3 shootDir = manMoveDir; // Already normalized and horizontal

		// Calculate the horizontal right vector based on manMoveDir
		vec3 playerRight = normalize(cross(manMoveDir, vec3(0.0f, 1.0f, 0.0f)));

		// Spawn Position Calculation (relative to character's position and orientation)
		float forwardOffset = 0.5f; // How far in front of player center
		float upOffset = 0.8f;      // Height relative to player base (Config::GROUND_HEIGHT)
		float rightOffset = 0.2f;   // Offset to the side (e.g., right hand)

		vec3 spawnPos = player->getPosition()
		+ vec3(0.0f, upOffset, 0.0f) // Vertical offset from base
		+ shootDir * forwardOffset   // Forward offset along character's facing direction
		+ playerRight * rightOffset; // Sideways offset along character's right

		// Create and add projectile
		activeSpells.emplace_back(spawnPos, shootDir, (float)glfwGetTime(), sphere);
		cout << "[DEBUG] Spell Fired! Start:(" << spawnPos.x << "," << spawnPos.y << "," << spawnPos.z
			<< ") Dir: (" << shootDir.x << "," << shootDir.y << "," << shootDir.z // y should be 0
			<< "). Active spells: " << activeSpells.size() << endl;
	}

	// --- updateProjectiles ---
	void updateProjectiles(float deltaTime) {
		if (!sphereAABBCalculated) return;

		float damageAmount = Config::PROJECTILE_DAMAGE;

		// Iterate using index for potential removal
		for (int i = 0; i < activeSpells.size(); ++i) {
			if (!activeSpells[i].active) continue;

			SpellProjectile& proj = activeSpells[i]; // Use reference

			// Check lifetime
			if (glfwGetTime() - proj.spawnTime > proj.lifetime) {
				proj.active = false;
				// cout << "[DEBUG] Spell lifetime expired." << endl;
				continue;
			}

			// Update position
			proj.position += proj.direction * proj.speed * deltaTime;

			// Calculate transform HERE
			glm::quat rotation = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), proj.direction);
			proj.transform = glm::translate(glm::mat4(1.0f), proj.position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), proj.scale);

			// Update AABB using Application's function
			this->updateBoundingBox(baseSphereLocalAABBMin, baseSphereLocalAABBMax, proj.transform, proj.aabbMin, proj.aabbMax);

			// Check collision with enemies
			for (auto* enemy : enemies) {
				if (!enemy || !enemy->isAlive()) continue;

				if (checkAABBCollision(proj.aabbMin, proj.aabbMax, enemy->getAABBMin(), enemy->getAABBMax())) {
					cout << "[DEBUG] Spell HIT enemy!" << endl;

					enemy->takeDamage(damageAmount);
					proj.active = false; // Deactivate projectile
					break; // Hit one enemy
				}
			}

			// for boss enemy
			if (canFightboss && bossEnemy && bossEnemy->isAlive()) {
				if (checkAABBCollision(proj.aabbMin, proj.aabbMax, bossEnemy->getAABBMin(), bossEnemy->getAABBMax())) {
					cout << "[DEBUG] Spell HIT boss!" << endl;
					damageAmount = 500.0f; // Boss takes more damage

					bossEnemy->takeDamage(damageAmount);
					proj.active = false; // Deactivate projectile
					break; // Hit the boss
				}
			}
		} // End loop

		// Remove inactive projectiles
		activeSpells.erase(
			std::remove_if(activeSpells.begin(), activeSpells.end(),
				[](const SpellProjectile& p) { return !p.active; }),
			activeSpells.end()
		);
	}

	void drawProjectiles(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model || !sphere) return; // Need shader, stack, model

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		// Set Material properties - check for uniform existence first
		if (secondPass) SetMaterial(shader, Material::orb_glowing_yellow);

		for (const auto& proj : activeSpells) {
			if (!proj.active) continue;

			Model->pushMatrix();
			Model->loadIdentity(); // Start from identity for projectile

			// Use the pre-calculated transform from updateAABB
			Model->multMatrix(proj.transform);

			setModel(shader, Model);
			proj.model->Draw(shader); // Draw the sphere model

			Model->popMatrix();
		}

		if (secondPass) clearProgFlags(shader);

		shader->unbind();
	}

	/* boss projectiles */
	void drawBossProjectiles(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) {
		if (!shader || !Model || !sphere) return; // safety checks

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		if (secondPass) SetMaterial(shader, Material::gold); //TODO UPDATE MATERIAL

		for (const auto& proj : bossActiveSpells) {
			if (!proj.active) continue;
			Model->pushMatrix();
			Model->loadIdentity(); // Start from identity for projectile
			Model->multMatrix(proj.transform); // Use the pre-calculated transform from updateAABB
			setModel(shader, Model);
			proj.model->Draw(shader); // Draw the sphere model
			Model->popMatrix();
		}

		if (secondPass) clearProgFlags(shader);

		shader->unbind();
	}

	void updateBossProjectiles(float deltaTime) {
		if (!sphereAABBCalculated) return;

		float damageAmount = 25.0f;

		// Iterate using index for potential removal
		for (int i = 0; i < bossActiveSpells.size(); ++i) {
			if (!bossActiveSpells[i].active) continue;

			SpellProjectile& proj = bossActiveSpells[i]; // Use reference

			// Check lifetime
			if (glfwGetTime() - proj.spawnTime > proj.lifetime) {
				proj.active = false;
				// cout << "[DEBUG] Spell lifetime expired." << endl;
				continue;
			}

			// Update position
			proj.position += proj.direction * proj.speed * deltaTime;

			// Calculate transform HERE
			glm::quat rotation = glm::rotation(glm::vec3(0.0f, 0.0f, 1.0f), proj.direction);
			proj.transform = glm::translate(glm::mat4(1.0f), proj.position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), proj.scale);

			// Update AABB using Application's function
			this->updateBoundingBox(baseSphereLocalAABBMin, baseSphereLocalAABBMax, proj.transform, proj.aabbMin, proj.aabbMax);

			// Check collision with player
			// if (checkAABBCollision(proj.aabbMin, proj.aabbMax, playerLocalAABBMin, playerLocalAABBMax)) {
			// 	cout << "[DEBUG] Spell HIT player!" << endl;
			// 	player->takeDamage(damageAmount);
			// 	proj.active = false; // Deactivate projectile
			// 	break; // Hit the player
			// }
			if (checkSphereCollision(player->getPosition(), 1.5f, proj.aabbMin, proj.aabbMax)) {
				cout << "[DEBUG] Spell HIT player!" << endl;
				player->takeDamage(damageAmount);
				proj.active = false; // Deactivate projectile
				break; // Hit the player
			}
		} // End loop

		// Remove inactive projectiles
		bossActiveSpells.erase(
			std::remove_if(bossActiveSpells.begin(), bossActiveSpells.end(),
				[](const SpellProjectile& p) { return !p.active; }),
			bossActiveSpells.end()
		);
	}

	void shootBossSpell() {
		vec3 shootDir = bossEnemy->getBossDirection();

		vec3 bossRight = normalize(cross(shootDir, vec3(0.0f, 1.0f, 0.0f)));

		float forwardOffset = 0.5f; // How far in front of player center
		float upOffset = 0.8f;      // Height relative to player base (groundY)
		float rightOffset = 0.0f;   // Offset to the side (e.g., right hand)

		vec3 spawnPos = bossEnemy->getPosition()
			+ vec3(0.0f, upOffset, 0.0f) // Vertical offset from base
			+ shootDir * forwardOffset   // Forward offset along character's facing direction
			+ bossRight * rightOffset; // Sideways offset along character's right

		// Create and add projectile
		bossActiveSpells.emplace_back(spawnPos, shootDir, (float)glfwGetTime(), sphere);
	}

	void BossEnemyShoot(float deltaTime) {
		if (bossEnemy && bossfightstarted && !bossfightended && bossEnemy->isAlive()) {
			// increment every 2 seconds
			if (glfwGetTime() - bossEnemy->getSpecialAttackCooldown() > 0.8f) {
				bossEnemy->setSpecialAttackCooldown(glfwGetTime());
				shootBossSpell();
			}
			updateBossProjectiles(deltaTime);
		}
	}

	/* top down camera view  */
	mat4 SetTopView(shared_ptr<Program> curShade) { /*MINI MAP*/
		mat4 Cam = glm:: lookAt(eye + vec3(0, 12, 0), eye, lookAt - eye);
		glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
		return Cam;
	}

	mat4 SetOrthoMatrixMiniMap(shared_ptr<Program> curShade) {/*MINI MAP*/
		float wS = 1.5;
		mat4 ortho = glm::ortho(-15.0f*wS, 15.0f*wS, -15.0f*wS, 15.0f*wS, 2.1f, 100.f);
		glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(ortho));
		return ortho;
	}

  void drawMiniPlayer(shared_ptr<Program> curS, shared_ptr<MatrixStack> Model, bool secondPass) { /*MINI MAP*/
		curS->bind();

		if (secondPass) setProgFlags(curS, true, false); // material, no bones

		// Model matrix setup
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(player->getPosition()); // Use final player position
			// *** USE CAMERA ROTATION FOR MODEL ***
			Model->scale(1.0);
			if (secondPass) SetMaterial(curS, Material::mini_map);
			setModel(curS, Model);
			sphere->Draw(curS);
		Model->popMatrix();

		if (secondPass) clearProgFlags(curS);

		curS->unbind();
	}

	void drawHealthBar(shared_ptr<Program> curShade) {
		float heatlhBarWidth = 350.0f;
		float healthBarHeight = 25.0f;
		float healthBarStartX = 100.0f;
		float healthBarStartY = 100.0f;
		int screenWidth, screenHeight;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenWidth, &screenHeight);

		glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);

		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(healthBarStartX, healthBarStartY, 0.0f));  // HUD position
		model = glm::scale(model, glm::vec3(heatlhBarWidth, healthBarHeight, 1.0f));                          // HUD size

		curShade->bind();

		glUniformMatrix4fv(curShade->getUniform("projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(curShade->getUniform("model"), 1, GL_FALSE, value_ptr(model));
		glUniform1f(curShade->getUniform("healthPercent"), player->getHitpoints() / Config::PLAYER_HP_MAX); // Pass health value
		glUniform1f(curShade->getUniform("BarStartX"), healthBarStartX); // Pass max health value
		glUniform1f(curShade->getUniform("BarWidth"), heatlhBarWidth); // Pass max health value

		healthBar->Draw(curShade);
		curShade->unbind();
	}

	// Draw particles (TODO: sort them by z value)
	void drawParticles(shared_ptr<particleGen> gen, shared_ptr<Program> shader, shared_ptr<MatrixStack> Model) {
		shader->bind();
		Model->pushMatrix();
			glPointSize(10.0f);
			particleAlphaTex->bind(particleProg->getUniform("alphaTexture"));

			// Enable blending for transparency
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
			gen->drawMe(shader);

			glDisable(GL_BLEND); // Restore state
			particleAlphaTex->unbind();
		Model->popMatrix();
		shader->unbind();
	}

	void drawEnemyHealthBars(shared_ptr<Program> curShade, glm::mat4 viewMatrix, glm::mat4 projMatrix) {
		float healthBarWidth = 100.0f;
		float healthBarHeight = 10.0f;
		float healthBarOffsetY = 15.0f;  // Offset above enemy head

		int screenWidth, screenHeight;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenWidth, &screenHeight);

		glm::mat4 hudProjection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);

		for (auto* enemy : enemies) {
			if (!enemy || !enemy->isAlive() || (!enemy->isHit())) continue;

			glm::vec3 enemyWorldPos = enemy->getAABBMax(); // Top position in world coordinates

			// Transform enemy position to clip space
			glm::vec4 clipSpacePos = projMatrix * viewMatrix * glm::vec4(enemyWorldPos, 1.0f);

			// If enemy is behind camera, skip
			if (clipSpacePos.w <= 0) continue;

			// Perspective divide (NDC)
			glm::vec3 ndcPos = glm::vec3(clipSpacePos) / clipSpacePos.w;

			// Convert NDC (-1 to 1) to screen coordinates
			glm::vec2 screenPos;
			screenPos.x = (ndcPos.x * 0.5f + 0.5f) * screenWidth;
			screenPos.y = (ndcPos.y * 0.5f + 0.5f) * screenHeight;

			// Offset above enemy's head
			screenPos.y += healthBarOffsetY;

			// Set HUD Model matrix
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(screenPos.x - (healthBarWidth / 2.0f), screenPos.y, 0.0f));
			model = glm::scale(model, glm::vec3(healthBarWidth, healthBarHeight, 1.0f));

			curShade->bind();
			glUniformMatrix4fv(curShade->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(hudProjection));
			glUniformMatrix4fv(curShade->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform1f(curShade->getUniform("healthPercent"), enemy->getHitpoints() / ENEMY_HP_MAX);
			glUniform1f(curShade->getUniform("BarStartX"), screenPos.x - (healthBarWidth / 2.0f));
			glUniform1f(curShade->getUniform("BarWidth"), healthBarWidth);

			healthBar->Draw(curShade);
			curShade->unbind();
		}
	}
	
	void drawLock(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass){
		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // materials, no bones

		//top lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 2.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
			lockHandle->Draw(shader);
		Model->popMatrix();

		//middle lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 1.5f, 38.5f));  //doorPosition
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
			lockHandle->Draw(shader);
		Model->popMatrix();

		//lower lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 0.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
			lockHandle->Draw(shader);
		Model->popMatrix();

		if (secondPass) clearProgFlags(shader);

		shader->unbind();
	}

	void drawUnlocked(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass) { //unlock one of the locks if have a key (for now unlock all)
		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // materials, no bones

		//top lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 2.5f, 38.5f));  //doorPosition
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
		Model->popMatrix();

		//top handle
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 2.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->rotate( 1* glm::radians(15.0) + lTheta , vec3(0.0f, 0.0f, 1.0f)); //max -30?
			Model->scale(0.1f);
			// Model->rotate(  glm::radians(90.0) , vec3(0.0f, 1.0f, 0.0f)); //max -30
			if (secondPass) SetMaterial(shader, Material::wood);
			setModel(shader, Model);
			lockHandle->Draw(shader);
		Model->popMatrix();

		//middle lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 1.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
		Model->popMatrix();

		//midle handle
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 1.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->rotate( 1* glm::radians(15.0) + lTheta , vec3(0.0f, 0.0f, 1.0f)); //max -30?
			Model->scale(0.1f);
			// Model->rotate(  glm::radians(90.0) , vec3(0.0f, 1.0f, 0.0f)); //max -30
			if (secondPass) SetMaterial(shader, Material::brown);
			setModel(shader, Model);
			lockHandle->Draw(shader);
		Model->popMatrix();

		// lower lock
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 0.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->scale(0.1f);
			if (secondPass) SetMaterial(shader, Material::gold);
			setModel(shader, Model);
			lock->Draw(shader);
		Model->popMatrix();

		//lower handle
		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 0.5f, 38.5f));
			Model->rotate(glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
			Model->rotate( 1* glm::radians(15.0) + lTheta , vec3(0.0f, 0.0f, 1.0f)); //max -30?
			Model->scale(0.1f);
			// Model->rotate(  glm::radians(90.0) , vec3(0.0f, 1.0f, 0.0f)); //max -30
			if (secondPass) SetMaterial(shader, Material::wood);
			setModel(shader, Model);
			lockHandle->Draw(shader);
		Model->popMatrix();

		if (secondPass) clearProgFlags(shader);

		shader->unbind();
	}

	/* keyCollect */
	void drawKey(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model, bool secondPass){
		// Collision Check
		for (auto& key : keyCollectibles) {
			// Perform collision check ONLY if not collected AND in the IDLE state
			if (!key.collected && key.state == OrbState::IDLE &&
				checkAABBCollision(playerAABB.min + player->getPosition(), playerAABB.max + player->getPosition(), key.AABBmin, key.AABBmax)) {
				key.collected = true;
				keysCollectedCount++;
				std::cout << "Collected a key! (" << keysCollectedCount << ")\n";
			}
		}

		shader->bind();

		if (secondPass) setProgFlags(shader, true, false); // material, no bones

		int collectedKeyDrawIndex = 0;

		for (auto& key : keyCollectibles) {

			glm::vec3 currentDrawPosition;

			if (key.collected) {
				float backOffset = 0.4f;
				float upOffsetBase = 0.6f;
				float stackOffset = key.scale * 2.5f;
				float sideOffset = 0.15f;
				glm::vec3 playerForward = normalize(manMoveDir);
				glm::vec3 playerUp = glm::vec3(0.0f, 1.0f, 0.0f);
				glm::vec3 playerRight = normalize(cross(playerForward, playerUp));
				float currentUpOffset = upOffsetBase + (collectedKeyDrawIndex * stackOffset);
				float currentSideOffset = (collectedKeyDrawIndex % 2 == 0 ? -sideOffset : sideOffset);
				currentDrawPosition = player->getPosition() - playerForward * backOffset
					+ playerUp * currentUpOffset
					+ playerRight * currentSideOffset;
				collectedKeyDrawIndex++;
			}
			else {
				currentDrawPosition = key.position;
			}

			Model->pushMatrix();
				Model->loadIdentity();
				Model->translate(vec3(0.0f, 0.5f, 0.5f)); //last enemy pos
				Model->scale(2.0f);
				Model->rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
				Model->rotate(glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
				if (secondPass) SetMaterial(shader, Material::gold);
				setModel(shader, Model);
				key.model->Draw(shader);
			Model->popMatrix();
		} // End drawing loop

		Model->pushMatrix();
			Model->loadIdentity();
			Model->translate(vec3(0.0f, 0.5f, 0.5f)); //last enemy pos
			Model->scale(2.0f);
			Model->rotate(glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
			Model->rotate(glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
			if (secondPass) SetMaterial(shader, Material::gold); //gold
			setModel(shader, Model);
			key->Draw(shader);
		Model->popMatrix();

		if (secondPass) clearProgFlags(shader);

		shader->unbind();
	}

	void updateKeys(float currentTime) {
		for (auto& key : keyCollectibles) {
			if (!key.collected) { // Update levitation only if not already collected
				key.updateLevitation(currentTime);
			}
		}
	}

	void drawBossHealthBar(glm::mat4 viewMatrix, glm::mat4 projMatrix) {
		float healthBarWidth = 200.0f;
		float healthBarHeight = 20.0f;
		float healthBarOffsetY = 25.0f; // Offset above enemy head

		int screenWidth, screenHeight;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenWidth, &screenHeight);

		glm::mat4 hudProjection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);

		if (bossEnemy && bossEnemy->isAlive()) {
			glm::vec3 enemyWorldPos = bossEnemy->getAABBMax(); // Top position in world coordinates

			// Transform enemy position to clip space
			glm::vec4 clipSpacePos = projMatrix * viewMatrix * glm::vec4(enemyWorldPos, 1.0f);

			// If enemy is behind camera, skip
			if (clipSpacePos.w <= 0) return;

			// Perspective divide (NDC)
			glm::vec3 ndcPos = glm::vec3(clipSpacePos) / clipSpacePos.w;

			// Convert NDC (-1 to 1) to screen coordinates
			glm::vec2 screenPos;
			screenPos.x = (ndcPos.x * 0.5f + 0.5f) * screenWidth;
			screenPos.y = (ndcPos.y * 0.5f + 0.5f) * screenHeight;

			// Offset above enemy's head
			screenPos.y += healthBarOffsetY;

			// Set HUD Model matrix
			glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(screenPos.x - (healthBarWidth / 2.0f), screenPos.y, 0.0f));
			model = glm::scale(model, glm::vec3(healthBarWidth, healthBarHeight, 1.0f));

			hudProg->bind();
			glUniformMatrix4fv(hudProg->getUniform("projection"), 1, GL_FALSE, glm::value_ptr(hudProjection));
			glUniformMatrix4fv(hudProg->getUniform("model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform1f(hudProg->getUniform("healthPercent"), bossEnemy->getHitpoints() / BOSS_HP_MAX);
			glUniform1f(hudProg->getUniform("BarStartX"), screenPos.x - (healthBarWidth / 2.0f));
			glUniform1f(hudProg->getUniform("BarWidth"), healthBarWidth);
			healthBar->Draw(hudProg);
			hudProg->unbind();
		}
	}

	void drawDamageIndicator(shared_ptr<Program>& shader, float alpha) {
		int screenWidth, screenHeight;
		glfwGetFramebufferSize(windowManager->getHandle(), &screenWidth, &screenHeight);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// glDisable(GL_DEPTH_TEST);
		shader->bind();
		glm::mat4 proj = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, -1.0f, 1.0f);
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
		model = glm::scale(model, glm::vec3(screenWidth, screenHeight, 1.0f));
		glUniformMatrix4fv(shader->getUniform("projection"), 1, GL_FALSE, value_ptr(proj));
		glUniformMatrix4fv(shader->getUniform("model"), 1, GL_FALSE, value_ptr(model));
		glUniform1f(shader->getUniform("alpha"), alpha); // Red color with alpha
		healthBar->Draw(shader);
		shader->unbind();
	}

	void drawOcclusionBoxAtPlayer(shared_ptr<Program> shader, shared_ptr<MatrixStack> Model) {
		if (!shader || !Model || !sphere) return; // Need shader, stack, model

		shader->bind();
		Model->pushMatrix();
		Model->loadIdentity(); // Start from identity for projectile

		Model->translate(player->getPosition() + glm::vec3(0, 1.0f, 0));
		Model->scale(0.25f);

		setModel(shader, Model);
		sphere->Draw(shader); // Draw the sphere model

		Model->popMatrix();
		shader->unbind();
	}

	void setCameraProjectionFromStack(shared_ptr<Program> curShade, shared_ptr<MatrixStack> projStack) {
		curShade->bind();
		glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE, value_ptr(projStack->topMatrix()));
	}

	void setCameraViewFromStack(shared_ptr<Program> curShade, shared_ptr<MatrixStack> viewStack) {
		curShade->bind();
		glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(viewStack->topMatrix()));
	}

	// Draw the scene for shadow map generation (Draw only shadow-casting objects) (First Pass)
	void drawSceneForShadowMap(shared_ptr<Program>& prog) {
		auto Model = make_shared<MatrixStack>();
		
		drawBorderWalls(prog, Model, false); // Border walls
		drawLibGrnd(prog, Model, false); // Floor
		drawLibrary(prog, Model, false, false); // Shelves
		drawBossRoom(prog, Model, false, false); // Boss room


		
		drawBooks(prog, Model, false); // Books
		drawEnemies(prog, Model, false); // Enemies
		drawPlayer(prog, Model, 0, false); // Player
		
		
	}

	// Draw the scene with shadows (Second Pass)
	void drawMainScene(const shared_ptr<Program>& prog, shared_ptr<MatrixStack>& Model, float animTime) {
		drawBorderWalls(prog, Model, true); // Border walls
		drawLibGrnd(prog, Model, true); // Floor
		drawLibrary(prog, Model, true, true); // Shelves
		drawBossRoom(prog, Model, true, true); // Boss room

		occlusionQuery(prog, Model); // NOTE: walls, ground, library and boss room need to be called before Occlusion Query

		drawPlayer(prog, Model, animTime, true); // Player (with animation)
		drawBooks(prog, Model, true); // Books
		drawEnemies(prog, Model, true); // Enemies
		drawOrbs(prog, Model, true); // Orbs
		drawProjectiles(prog, Model, true); // Projectiles
		drawBossProjectiles(prog, Model, true); // Boss Projectiles
		(unlocked) ? drawUnlocked(prog, Model, true) : drawLock(prog, Model, true); // Locks
		drawBossEnemy(prog, Model, true); // Big Boss Man
	}

	void occlusionQuery(const shared_ptr<Program>& shader, shared_ptr<MatrixStack>& Model) {
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable color writes
		glDepthMask(GL_FALSE); // disable depth writes
		glBeginQuery(GL_ANY_SAMPLES_PASSED, occlusionQueryID); // begin occlusion query
		drawOcclusionBoxAtPlayer(shader, Model);
		glEndQuery(GL_ANY_SAMPLES_PASSED);

		GLuint resultofQuery = 0;
		glGetQueryObjectuiv(occlusionQueryID, GL_QUERY_RESULT, &resultofQuery);
		visible = resultofQuery;

		// re-enable color writes and depth writes
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
	}

	// Draw the mini map
	void drawMiniMap(const shared_ptr<Program>& prog, shared_ptr<MatrixStack>& Model, int height) {
		prog->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, height - 300, 300, 300);

		SetOrthoMatrixMiniMap(prog);
		SetTopView(prog);

		// Draw mini map elements
		/*drawDoor(prog, Model, true);
		drawBooks(prog, Model, true);
		drawEnemies(prog, Model, true);*/
		drawLibrary(prog, Model, false, true);
		drawBossRoom(prog, Model, false, true);
		drawBossEnemy(prog, Model, true);
		drawMiniPlayer(prog, Model, true);
		drawBorderWalls(prog, Model, true);
		drawLibGrnd(prog, Model, true);
		drawBossRoom(prog, Model, false, true); //boss room not drawing

		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, height - 300, 300, 300);
		SetTopView(prog);
		drawEnemies(prog, Model, true);

		prog->unbind();
	}

	void render(float frametime, float animTime) {
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height); // Get current frame buffer size
		float aspect = width / (float)height;

		// --- Update Game Logic ---
		charMove();
		updateCameraVectors();
		updateBooks(frametime);
		updateOrbs((float)glfwGetTime());
		//updateKeys((float)glfwGetTime());
		updateEnemies(frametime);
		updateProjectiles(frametime);
		particleSystem->update(frametime); // Update particles
		checkAllEnemies();
		checkBossfight();
		BossEnemyShoot(frametime);
		restartGeneration();

		// Create the matrix stacks
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		vec3 lightPos = vec3(10); // Fixed light position above the scene
		vec3 lightTarget = libraryCenter; // Light looks at library center
		vec3 lightDir = normalize(lightPos - lightTarget); // Light direction
		vec3 lightUp = vec3(0, 1, 0);
		mat4 LO, LV, LSpace;
		
		// ========================================================================
		// First Pass: Render scene from light's perspective to generate depth map
		// ========================================================================
		if (Config::SHADOW) {
			glViewport(0, 0, S_WIDTH, S_HEIGHT); // Set viewport for shadow map
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO); // Bind shadow framebuffer
			glClear(GL_DEPTH_BUFFER_BIT); // Clear depth buffer
			glCullFace(GL_FRONT); // Cull front faces for shadow map

			DepthProg->bind(); // Setup shadow shader and draw the scene

			// Create a stable orthographic projection that covers the scene
			float size = Config::ORTHO_SIZE;
			LO = glm::ortho(-size, size, -size, size, 1.0f, 200.0f);
			glUniformMatrix4fv(DepthProg->getUniform("LP"), 1, GL_FALSE, value_ptr(LO));

			// Create a stable light view matrix
			LV = glm::lookAt(lightPos, lightTarget, lightUp);
			glUniformMatrix4fv(DepthProg->getUniform("LV"), 1, GL_FALSE, value_ptr(LV));

			drawSceneForShadowMap(DepthProg); // Draw the scene from the lights perspective

			DepthProg->unbind();
			glCullFace(GL_BACK); // Reset culling to default
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind shadow framebuffer (hard coded 0 is the screen)
		}

		// ===================================================
		// Prepare for Second Pass (Main Rendering to Screen)
		// ===================================================
		glViewport(0, 0, width, height); // Return viewport to screen size
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear framebuffer

		// Setup Camera
		Projection->pushMatrix();
		Projection->perspective(radians(45.0f), aspect, 0.1f, 100.0f);
		View->pushMatrix();
		View->loadIdentity();
		View->lookAt(eye, lookAt, up); // Use updated eye/lookAt

		// Update frustum planes and light space matrix for shadow mapping
		ExtractVFPlanes(Projection->topMatrix(), View->topMatrix(), planes);

		// ==============================
		// Second Pass: Render to Screen
		// ==============================
		if (Config::DEBUG_LIGHTING) { // Debugging light view from lights perspective
			if (Config::DEBUG_GEOM) {
				DepthProgDebug->bind();

				glUniformMatrix4fv(DepthProg->getUniform("LP"), 1, GL_FALSE, value_ptr(LO));

				glUniformMatrix4fv(DepthProg->getUniform("LV"), 1, GL_FALSE, value_ptr(LV));

				drawSceneForShadowMap(DepthProgDebug); // Draw the scene from the lights perspective for debugging
				
				DepthProgDebug->unbind();
			}
			else { // Draw the depth map texture to a quad for visualization
				DebugProg->bind();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, depthMap);
				glUniform1i(DebugProg->getUniform("texBuf"), 0);

				glEnableVertexAttribArray(0); // Now we actually draw the quad
				glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(0);
				DebugProg->unbind();
			}
		}
		else { // Render the scene like normal with shadow mapping
			ShadowProg->bind();

			// Setup shadow mapping
			glActiveTexture(GL_TEXTURE10);
			glBindTexture(GL_TEXTURE_2D, depthMap); // Bind shadow map texture
			glUniform1i(ShadowProg->getUniform("shadowDepth"), 10); // Set uniform for shadow map

			// Set light and camera uniforms
			glUniform3f(ShadowProg->getUniform("lightDir"), lightDir.x, lightDir.y, lightDir.z); // Set light direction
			glUniform3f(ShadowProg->getUniform("lightColor"), 1.0f, 1.0f, 1.0f); // White light
			glUniform1f(ShadowProg->getUniform("lightIntensity"), 1.0f); // Full intensity
			glUniform3fv(ShadowProg->getUniform("cameraPos"), 1, glm::value_ptr(eye));

			setCameraProjectionFromStack(ShadowProg, Projection);
			setCameraViewFromStack(ShadowProg, View);

			LSpace = LO * LV;
			glUniformMatrix4fv(ShadowProg->getUniform("LV"), 1, GL_FALSE, value_ptr(LSpace)); // Set light space matrix

			drawMainScene(ShadowProg, Model, animTime); // Draw the entire scene with shadows

			ShadowProg->unbind();
		}

		if (Config::SHOW_MINIMAP) { // Draw the mini map
			drawMiniMap(ShadowProg, Model, height);
		}

		if (Config::SHOW_HEALTHBAR) { // Draw the health bar
			drawHealthBar(hudProg);
			drawEnemyHealthBars(hudProg, View->topMatrix(), Projection->topMatrix());
			if (bossfightstarted && !bossfightended) drawBossHealthBar(View->topMatrix(), Projection->topMatrix());
		}
		
		if (player->getDamageTimer() > 0.0f) { // red flash
			player->setDamageTimer(player->getDamageTimer() - frametime);
			float alpha = player->getDamageTimer() / Config::PLAYER_HIT_DURATION;
			drawDamageIndicator(redFlashProg, alpha);
		}
		else if (!player->isAlive() && !debugCamera) { // If player is dead, show red flash
			
			movingForward = false;
			movingBackward = false;
			movingLeft = false;
			movingRight = false;
			drawDamageIndicator(redFlashProg, 1.0f);
		}

		// --- Cleanup ---
		Projection->popMatrix();
		View->popMatrix();

		// Unbind any VAO or Program that might be lingering (belt-and-suspenders)
		glBindVertexArray(0);
		glUseProgram(0);
	}

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (player->isAlive() || debugCamera) {
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
				manState = Man_State::WALKING;

				//Movement Variable
				movingForward = true;
				if (debug_pos) {
					cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
					cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
				}
			}
			else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
				manState = Man_State::STANDING;
				//Movement Variable
				movingForward = false;
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
				manState = Man_State::WALKING;

				//Movement Variable
				movingBackward = true;

				if (debug_pos) {
					cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
					cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
				}

			}
			else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
				manState = Man_State::STANDING;
				//Movement Variable
				movingBackward = false;
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
				manState = Man_State::WALKING;

				//Movement Variable
				movingLeft = true;

				if (debug_pos) {
					cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
					cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
				}

			}
			else if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
				manState = Man_State::STANDING;
				//Movement Variable
				movingLeft = false;
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
				manState = Man_State::WALKING;

				//Movement Variable
				movingRight = true;

				if (debug_pos) {
					cout << "eye: " << eye.x << " " << eye.y << " " << eye.z << endl;
					cout << "lookAt: " << lookAt.x << " " << lookAt.y << " " << lookAt.z << endl;
				}
			}
			else if (key == GLFW_KEY_D && action == GLFW_RELEASE) {
				manState = Man_State::STANDING;
				//Movement Variable
				movingRight = false;
			}
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS) { // Interaction Key
			interactWithBooks();
		}
		if (key == GLFW_KEY_L && action == GLFW_PRESS) {
			cursor_visable = !cursor_visable;
			if (cursor_visable) {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			else {
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			}
		}
		if (key == GLFW_KEY_U && action == GLFW_PRESS) {
			unlocked = true;
		}
		if (key == GLFW_KEY_K && action == GLFW_PRESS) {
			debugCamera = !debugCamera;
		}
		if (!player->isAlive() && key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			restartGen = true;
		}
		if (key == GLFW_KEY_O && action == GLFW_PRESS) { // toggle debug light
			Config::DEBUG_LIGHTING = !Config::DEBUG_LIGHTING;
		}
		if (key == GLFW_KEY_P && action == GLFW_PRESS) { // toggle debug geom
			Config::DEBUG_GEOM = !Config::DEBUG_GEOM;
		}
	}

	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY)
	{
		theta = theta + deltaX * glm::radians(Config::CAMERA_SCROLL_SENSITIVITY_DEGREES);
		phi = phi - deltaY * glm::radians(Config::CAMERA_SCROLL_SENSITIVITY_DEGREES);

		if (phi > glm::radians(Config::CAMERA_PHI_MAX_DEGREES)) {
			phi = glm::radians(Config::CAMERA_PHI_MAX_DEGREES);
		}
		if (phi < glm::radians(Config::CAMERA_PHI_MIN_DEGREES)) {
			phi = glm::radians(Config::CAMERA_PHI_MIN_DEGREES);
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

		theta = theta + deltaX * Config::CAMERA_MOUSE_SENSITIVITY;
		phi = phi + deltaY * Config::CAMERA_MOUSE_SENSITIVITY;

		if (phi > glm::radians(Config::CAMERA_PHI_MAX_DEGREES)) {
			phi = glm::radians(Config::CAMERA_PHI_MAX_DEGREES);
		}
		if (phi < radians(-80.0f))
		{
			phi = radians(-80.0f);
		}

		updateCameraVectors();
	}
};

void mouseMoveCallbackWrapper(GLFWwindow* window, double xpos, double ypos) {
	Application* app = (Application*)glfwGetWindowUserPointer(window);
	app->mouseMoveCallback(window, xpos, ypos);
}

int main(int argc, char *argv[]) {
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	std::shared_ptr<Player> playerPtr = std::make_shared<Player>(
		vec3(0, 0, 0),
		Config::PLAYER_HP_MAX,
		Config::PLAYER_MOVE_SPEED,
		application->CatWizard,
		vec3(1.0f, 1.0f, 1.0f),
		vec3(0.0f, 0.0f, 0.0f)
	);
	application->player = playerPtr;

	// Your main will always include a similar set up to establish your window
	// and GL context, etc

	WindowManager *windowManager = new WindowManager();
	windowManager->init(640, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowUserPointer(windowManager->getHandle(), application);
	glfwSetCursorPosCallback(windowManager->getHandle(), mouseMoveCallbackWrapper);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initMapGen();
	application->initGeom(resourceDir);
	application->initGround();
	glGenQueries(1, &application->occlusionQueryID);

	auto lastTime = chrono::high_resolution_clock::now();

	glfwSetInputMode(windowManager->getHandle(), GLFW_STICKY_KEYS, GLFW_TRUE);

	cout << "Controls: " << endl << "WASD: Move" << endl << "Mouse: Look around" << endl
		<< "'F': Interact with book" << "F11 Fullscreen" << endl << "'L': Toggle cursor mode" << endl;

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle())) {
		auto nextLastTIme = chrono::high_resolution_clock::now();

		float deltaTime = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - lastTime).count();

		deltaTime *= 0.000001f; // convert to seconds

		lastTime = nextLastTIme;

		float AnimcurrFrame = glfwGetTime();
		application->AnimDeltaTime = AnimcurrFrame - application->AnimLastFrame;
		application->AnimLastFrame = AnimcurrFrame;
		
		application->render(deltaTime, application->AnimDeltaTime); // Render scene

		glfwSwapBuffers(windowManager->getHandle()); // Swap front and back buffers
		
		glfwPollEvents(); // Poll for and process events
	}

	// Quit program
	windowManager->shutdown();
	return 0;
}
