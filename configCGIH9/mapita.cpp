// ============================================================
// Tank City - main.cpp
// Juego de tanques en OpenGL con mapa tipo tile, enemigos con IA,
// sistema de items, sombras dinamicas y pantallas retro.
//
// Dependencias: GLEW, GLFW3, GLM, GDI+ (Windows)
// Compilar con: g++ main.cpp -lglew -lglfw -lopengl32 -lgdi32
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// ============================================================
// Declaraciones de funciones
// ============================================================

// --- Camara ---
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void PrintCameraMenu();
void ToggleCameraMode();
glm::vec3 GetFollowCameraPosition();
glm::mat4 GetCurrentViewMatrix();
glm::vec3 GetCurrentCameraPosition();

// --- Movimiento y logica del jugador ---
void DoMovement();
void Animation();
void ShootBullet();
void UpdateBullet();
void TriggerPlayerExplosion();
void ResetGame();

// --- Sistema de enemigos ---
void InitEnemySystem();
void UpdateEnemies();
int  CountActiveEnemies();
int  ChooseEnemyTypeForLevel();
float GetEnemySpeed(int type);
int  GetEnemyHP(int type);
glm::vec3 GetForwardFromRot(float rotY);
bool IsBlockedPositionEnemy(const glm::vec3& pos);
void DamageEnemy(int index);
void EnemyShoot(int index);
void UpdateEnemyBullets();
bool CollidesWithAnyEnemy(const glm::vec3& pos, int ignoreIndex);
bool CollidesWithPlayer(const glm::vec3& pos);
int  EnemyPoints(int type);

// --- Animacion por keyframes: tanque gris especial ---
void InitGrayTankKeyFrames();
void PlayGrayTankKeyFrameAnimation();
void UpdateGrayTankKeyFrameAnimation();
void ResetGrayTankKeyFrameAnimation();
void SaveGrayTankFrame(float x, float y, float z, float rotY);
void ResetGrayTankElements();
void GrayTankInterpolation();
void GrayTankShootAtEagle();
glm::vec3 GetEaglePositionFromMap();

// --- Aguila (base) ---
void TriggerEagleExplosion();

// --- Score popups ---
void AddScorePopup(const glm::vec3& pos, int points);

// --- Items coleccionables ---
void InitItems();
void UpdateItems(float dt);
void DrawItems(Shader& textureShader,
    Model& cascoModel, Model& relojModel, Model& palaModel,
    Model& estrellaModel, Model& granadaModel, Model& vidaModel);
void CheckItemPickup();
void ApplyPalaEffect();
void RestorePalaEffect();
void SpawnRandomItemAt(const glm::vec3& pos);
glm::vec3 GetRandomFreeItemPosition();
void SpawnKonamiItems();
void ProcessKonamiCode(int key);
void ActivateMaxPower();
void DestroyEnemiesWithGrenade();

void DrawItemHUD(Shader& shader, GLuint VAO, GLint modelLoc);

// --- Dibujado ---
void DrawCube(Shader& shader, GLuint VAO, GLint modelLoc,
    const glm::vec3& position, const glm::vec3& scale, const glm::vec3& color);
void DrawModelAt(Model& model3D, Shader& shader,
    const glm::vec3& position, const glm::vec3& scale,
    const glm::vec3& rotationDeg, const glm::vec3& color);
void DrawTexturedModelAt(Model& model3D, Shader& shader,
    const glm::vec3& position, const glm::vec3& scale,
    const glm::vec3& rotationDeg);
void DrawTexturedModelAtDepth(Model& model3D, Shader& depthShader,
    const glm::vec3& position, const glm::vec3& scale,
    const glm::vec3& rotationDeg);

// --- Mapa principal ---
void DrawTankCityMap(
    Shader& cubeShader, Shader& modelShader, Shader& textureShader,
    GLuint VAO, GLint cubeModelLoc,
    Model& ladrilloModel, Model& metalModel, Model& adoquinModel,
    Model& aguaModel, Model& pastoModel, Model& hojasModel, Model& pisoModel,
    Model& aguilaModel, Model& barrilModel, Model& cajaModel,
    Model& tanquePropioModel, Model& tanque2Model, Model& tanque3Model, Model& tanque4Model,
    Model& generacion1Model, Model& generacion2Model,
    Model& explosion1Model, Model& explosion2Model, Model& explosion3Model,
    Model& explosion4Model, Model& explosion5Model,
    Model& balaModel,
    Model& score100Model, Model& score200Model, Model& score300Model,
    Model& cascoModel, Model& relojModel, Model& palaModel,
    Model& estrellaModel, Model& granadaModel, Model& vidaModel);


// --- Pase de profundidad para sombras ---
void DrawSceneDepthPass(
    Shader& depthShader, GLuint VAO,
    Model& ladrilloModel, Model& metalModel, Model& adoquinModel,
    Model& pisoModel, Model& barrilModel, Model& cajaModel,
    Model& tanquePropioModel, Model& tanque2Model,
    Model& tanque3Model, Model& tanque4Model);

// --- Utilidades del mapa ---
void InitPlayerFromMap();
glm::vec3 GetTankForward();
bool IsSolidTile(char tile);
bool IsBlockedPosition(const glm::vec3& pos);
char GetTileFromWorld(float worldX, float worldZ);
float GetLeafAlphaAt(float worldX, float worldZ);

// --- Pantallas de juego ---
void StartGameFromMenu();
void DrawStartScreen(Shader& shader, GLuint VAO, GLint modelLoc);
void DrawFinalScreen(Shader& shader, GLuint VAO, GLint modelLoc, bool win);
void DrawSecretScreen(Shader& shader, GLuint VAO, GLint modelLoc);
void ProcessSecretCode(int key);

// --- Texto retro (pixel art con cubos) ---
void DrawRetroText(Shader& shader, GLuint VAO, GLint modelLoc,
    const std::string& text, float x, float y, float px, const glm::vec3& color);

// --- UI con texturas ---
GLuint CompileShaderFromSource(GLenum type, const char* src);
GLuint CreateUITextureProgram();
void   InitUIQuad();
GLuint LoadTextureFromPNG(const char* path);
void   DrawUIImage(GLuint texture, float x, float y, float w, float h);

// ============================================================
// contador de ítems
// ============================================================
int itemsCollectedTotal = 0;
int cascoCollected = 0;
int relojCollected = 0;
int palaCollected = 0;
int estrellaCollected = 0;
int granadaCollected = 0;
int vidaCollected = 0;

// ============================================================
// Constantes de pantalla
// ============================================================
const GLuint WIDTH = 1280;
const GLuint HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// ============================================================
// Camara
// ============================================================
Camera camera(glm::vec3(0.0f, 18.0f, 32.0f));
bool     keys[1024] = { false };
GLfloat  lastX = WIDTH / 2.0f;
GLfloat  lastY = HEIGHT / 2.0f;
bool     firstMouse = true;

// Modos de camara disponibles
enum CameraMode { CAMERA_FREE, CAMERA_FOLLOW, CAMERA_FIRST_PERSON };
CameraMode cameraMode = CAMERA_FREE;

// Parametros de camara en tercera persona
float followCameraDistance = 10.0f;
float followCameraHeight = 7.0f;

// Parametros de camara en primera persona
float firstPersonHeight = 1.15f;
float firstPersonForwardOffset = 0.65f;

// ============================================================
// Estado general del juego
// ============================================================
enum GameScreen { SCREEN_START, SCREEN_PLAYING, SCREEN_WIN, SCREEN_LOSE, SCREEN_SECRET };
GameScreen gameScreen = SCREEN_START;

const int TOTAL_ENEMIES_PER_LEVEL = 10; // Enemigos totales por nivel

// Buffer para el codigo secreto escrito en la pantalla de inicio
std::string       secretBuffer = "";
const std::string SECRET_CODE = "CGIH0920262";

// ============================================================
// UI: textura de imagen en pantalla (pantalla secreta)
// ============================================================
GLuint uiTextureProgram = 0;
GLuint uiQuadVAO = 0;
GLuint uiQuadVBO = 0;
GLuint adventureTexture = 0;

#ifdef _WIN32
ULONG_PTR gdiplusToken = 0;
#endif

// ============================================================
// Tiempo de frame
// ============================================================
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// ============================================================
// Mapa del juego (30x30 tiles)
// Leyenda de tiles:
//   M = Metal (indestructible)
//   B = Ladrillo (destruible con 1 bala)
//   H = Hojas (decorativo, transparente)
//   A = Adoquin (destruible con 2 balas)
//   W = Agua (bloqueante)
//   G = Aguila (base a proteger)
//   P = Posicion inicial del jugador
//   T = Punto de spawn enemigo tipo 2
//   Y = Punto de spawn enemigo tipo 3
//   Z = Punto de spawn enemigo tipo 4
//   . = Suelo libre
// ============================================================
const float TILE = 2.0f; // Tamanio de cada tile en unidades de mundo

std::vector<std::string> mapa = {
    "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMM",
    "M.HHHB...A........A...HBHHH..M",
    "M.BBBB..MM....WW....MM.BBBB..M",
    "M.BHH.B....BB.WW.BB....B..B..M",
    "M.B.H.B.AA....WWH...AA.BH.B..M",
    "M...HBHH..B..HHHH.BHHH.B.....M",
    "M.MMH.HHBBBB..AAH.BBBB...MM..M",
    "M..HHWHH..A.....H.AHHHHW..H..M",
    "M.BB.WHMM....B.BH..HMM.W.BB..M",
    "M....WHHHHBBBB.BBBBHHH.W.....M",
    "M.AA....B...HHHHH...B....AA..M",
    "M...........T...Y............M",
    "M..BB........................M",
    "M......B.............B..H....M",
    "M.AA....B....H.H....B....AA..M",
    "M....W....BBBB.BBBB....W.....M",
    "M.BB.W.MM....A.A....MM.W.BB..M",
    "M..H.W....AHH...HHA....W..HHHM",
    "M.MM...BBBBHHB.BHHBBBB...MMHHM",
    "M....B....BHHHHHHHB....B..HHHM",
    "M.B..B.AA...HW.WH...AA.B..BHHM",
    "M.B..B....BBHW.WHBB....B..BHHM",
    "M.BBBB.MM...HW.WH.MM.B.B.BBHHM",
    "M.HHHH......H...H........HHHHM",
    "M.H.HH..B.A.......A...B..HH.HM",
    "M.HHHH.BBB..BBBBB....BBB.HHHHM",
    "M..Z........B...B.......PHHHHM",
    "M......BBB..B.G.B...BBB......M",
    "M...........BBBBB............M",
    "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMM"
};

// Copia original del mapa para resetear la partida
std::vector<std::string> mapaOriginal = mapa;

// ============================================================
// Jugador
// ============================================================
const float PLAYER_Y = 0.28f; // Altura del jugador sobre el suelo

glm::vec3 playerPos(0.0f, PLAYER_Y, 0.0f);
float playerRotY = 180.0f;  // Rotacion en grados
float playerMoveSpeed = 6.0f;
float playerTurnSpeed = 120.0f;  // Grados por segundo
float playerCollisionRadius = 0.85f;
bool  playerReady = false;
int   playerLives = 3;
const int PLAYER_MAX_LIVES = 3;

// Estados posibles del jugador durante el juego
enum PlayerState { PLAYER_OFF, PLAYER_SPAWNING, PLAYER_ALIVE, PLAYER_EXPLODING };
PlayerState playerState = PLAYER_OFF;

float playerAnimTimer = 0.0f;
int   spawnFrame = 0;    // Frame actual de la animacion de spawn
int   explosionFrame = 0;    // Frame actual de la animacion de explosion

// Tiempos por frame de animacion
const float spawnFrameTime = 0.35f;
const float explosionFrameTime = 0.18f;

// Poder maximo (estrella / Konami)
bool  maxPower = false;
float maxPowerTimer = 0.0f;

// ============================================================
// Bala del jugador (primaria)
// ============================================================
enum BulletState { BULLET_OFF, BULLET_MOVING, BULLET_HIT };

BulletState bulletState = BULLET_OFF;
glm::vec3   bulletPos(0.0f);
glm::vec3   bulletDir(0.0f);
float bulletSpeed = 7.0f;
float bulletY = 1.10f;  // Altura de la bala
float bulletSpawnDistance = 1.65f;  // Distancia inicial desde el cañon
float bulletRotY = 0.0f;
glm::vec3 bulletScale(5.00f, 5.00f, 5.00f);

// Bala secundaria (solo activa con poder maximo)
BulletState bullet2State = BULLET_OFF;
glm::vec3   bullet2Pos(0.0f);
glm::vec3   bullet2Dir(0.0f);
float       bullet2RotY = 0.0f;

// Daño acumulado en tiles de adoquin (necesitan 2 impactos)
int adoquinHP[30][30] = { 0 };

// ============================================================
// Sistema de enemigos
// ============================================================
enum EnemyType { ENEMY_GRAY, ENEMY_RED, ENEMY_GREEN };
enum EnemyState { ENEMY_INACTIVE, ENEMY_SPAWNING, ENEMY_ALIVE, ENEMY_EXPLODING };

struct Enemy {
    EnemyType  type;
    EnemyState state;
    glm::vec3  pos;
    float rotY;
    float timer;       // Temporizador de animacion
    float turnTimer;   // Tiempo desde el ultimo giro
    int   frame;       // Frame de animacion actual
    int   hp;
    float shootTimer;  // Tiempo desde el ultimo disparo
    float flashTimer;  // Para el parpadeo del tanque rojo
    bool  flashOn;
};

const int MAX_ACTIVE_ENEMIES = 4;          // Maximos enemigos simultaneos
std::vector<Enemy>    enemies(MAX_ACTIVE_ENEMIES);
std::vector<glm::vec3> enemySpawnPoints;   // Posiciones de aparicion

int   currentLevel = 1;
int   enemiesRemainingToSpawn = 10;      // Enemigos que faltan por aparecer
bool  greenSpawnedThisLevel = false;   // Si ya aparecio un verde este nivel
int   enemiesDestroyed = 0;
float enemySpawnTimer = 0.0f;
float enemySpawnInterval = 4.0f;   // Segundos entre apariciones
float enemyCollisionRadius = 0.85f;
float enemyBulletHitRadius = 1.30f;  // Radio de colision bala-enemigo

// Balas de enemigos
struct EnemyBullet {
    bool      active;
    glm::vec3 pos;
    glm::vec3 dir;
    float     rotY;
};

const int MAX_ENEMY_BULLETS = 8;
std::vector<EnemyBullet> enemyBullets(MAX_ENEMY_BULLETS);
float enemyBulletSpeed = 5.2f;
float enemyBulletY = 1.10f;
glm::vec3 enemyBulletScale(5.00f, 5.00f, 5.00f);
float playerHitRadius = 1.35f; // Radio de colision bala enemiga - jugador

// ============================================================
// Aguila (base a proteger)
// ============================================================
bool      eagleAlive = true;
bool      eagleExploding = false;
float     eagleExplosionTimer = 0.0f;
int       eagleExplosionFrame = 0;
glm::vec3 eaglePos(0.0f, 0.19f, 0.0f);

// ============================================================
// Animacion por keyframes: tanque gris especial
// Se basa en la practica de KeyFrames: guardar cuadros clave,
// calcular incrementos e interpolar paso a paso.
// ============================================================
float grayKFPosX = 0.0f;
float grayKFPosY = PLAYER_Y;
float grayKFPosZ = 0.0f;
float grayKFRotY = 180.0f;

#define MAX_GRAY_FRAMES 5
int gray_i_max_steps = 40;
int gray_i_curr_steps = 0;

typedef struct _grayFrame {
    float posX;
    float posY;
    float posZ;
    float rotY;

    float incX;
    float incY;
    float incZ;
    float rotYInc;
} GRAY_FRAME;

GRAY_FRAME GrayKeyFrame[MAX_GRAY_FRAMES];
int  grayFrameIndex = 0;
int  grayPlayIndex = 0;
bool grayPlay = false;
bool grayTankVisible = false;

// Disparos al terminar el recorrido por keyframes
bool  grayShootingPhase = false;
int   grayShotsDone = 0;
float grayShotTimer = 0.0f;
const float grayShotDelay = 0.90f;

// Aparicion visual antes de iniciar el movimiento
bool  graySpawnPhase = false;
float graySpawnTimer = 0.0f;
float graySpawnTotalTimer = 0.0f;
int   graySpawnFrame = 0;
const float graySpawnDuration = 0.90f;
const float graySpawnFrameTime = 0.18f;

// Bala visual exclusiva de esta animacion.
// No usa la logica normal de colisiones para no romper el juego definitivo.
bool grayAnimBulletActive = false;
glm::vec3 grayAnimBulletPos(0.0f);
glm::vec3 grayAnimBulletDir(0.0f);
float grayAnimBulletRotY = 0.0f;
float grayAnimBulletTimer = 0.0f;
const float grayAnimBulletDuration = 1.0f;
const float grayAnimBulletSpeed = 9.0f;

// ============================================================
// Puntuacion
// ============================================================
int score = 0;
int grayDestroyed = 0;
int redDestroyed = 0;
int greenDestroyed = 0;

// Popups flotantes de puntos al destruir enemigos
struct ScorePopup {
    bool      active;
    glm::vec3 pos;
    int       points;
    float     timer;   // Tiempo visible
    float     delay;   // Retardo antes de aparecer
};
std::vector<ScorePopup> scorePopups(8);

// ============================================================
// Items coleccionables y sus efectos activos
// ============================================================
bool  cascoActive = false;
float cascoTimer = 0.0f;
const float CASCO_DURATION = 30.0f;  // Segundos de invulnerabilidad

bool  relojActive = false;
float relojTimer = 0.0f;
const float RELOJ_DURATION = 30.0f;  // Segundos de enemigos congelados

bool  palaActive = false;
float palaTimer = 0.0f;
const float pala_DURATION = 30.0f;   // Segundos de muros metalicos

// Tiles convertidos por la pala (para restaurarlos al expirar)
std::vector<std::pair<int, int>> palaConvertedTiles;

const float MAX_POWER_DURATION = 30.0f;
const float BULLET_SPEED_NORMAL = 7.0f;
const float BULLET_SPEED_MAX_POWER = 11.0f;

// Tipos de items disponibles
enum ItemType { ITEM_CASCO = 0, ITEM_RELOJ = 1, ITEM_PALA = 2, ITEM_ESTRELLA = 3, ITEM_GRANADA = 4, ITEM_VIDA = 5 };

struct Item {
    ItemType  type;
    glm::vec3 worldPos;
    bool      active;
    float     bobTimer; // Para animacion de flotacion
};
std::vector<Item> items;

const float ITEM_PICKUP_DIST = 1.5f; // Distancia de recogida

// ============================================================
// Codigo Konami
// Secuencia: Arriba, Arriba, Abajo, Abajo, Izq, Der, Izq, Der, B, A
// ============================================================
int konamiIndex = 0;
const int KONAMI_SEQUENCE[10] = {
    GLFW_KEY_UP, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_DOWN,
    GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
    GLFW_KEY_B, GLFW_KEY_A
};

// ============================================================
// Sol dinamico (ciclo dia/noche simplificado)
// ============================================================
float sunAngle = 1.1f;
float sunSpeed = 0.04f;
bool  sunPaused = false;
const float SUN_RADIUS = 80.0f;
const float SUN_HEIGHT = 80.0f;

// Radio en que las hojas se vuelven transparentes cerca del jugador
const float LEAF_RADIUS = TILE * 1.5f;

// ============================================================
// Geometria del cubo unitario (36 vertices, sin normales/UV)
// Se usa para cubos de UI retro y suelo exterior
// ============================================================
float cubeVertices[] = {
    -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,
     0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f,-0.5f, 0.5f,
    -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
    -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
     0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f,  0.5f,-0.5f,-0.5f,
     0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f, 0.5f, 0.5f,
    -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,
     0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
    -0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,
     0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f
};

// Quad de pantalla completa para el shader de cielo
float skyQuadVertices[] = {
    -1.0f,-1.0f,  1.0f,-1.0f,  1.0f, 1.0f,
    -1.0f,-1.0f,  1.0f, 1.0f, -1.0f, 1.0f
};

// Resolucion del shadow map
const GLuint SHADOW_WIDTH = 4096;
const GLuint SHADOW_HEIGHT = 4096;

// ============================================================
// main()
// ============================================================
int main()
{
    // --- Inicializacion de GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Tank City - Mapa Base", nullptr, nullptr);
    if (!window)
    {
        std::cout << "No se pudo crear la ventana.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);

    // --- Inicializacion de GLEW ---
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "No se pudo inicializar GLEW.\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // --- GDI+ para cargar PNG en Windows ---
#ifdef _WIN32
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    srand((unsigned int)time(NULL));

    // --- UI de imagen para pantalla secreta ---
    uiTextureProgram = CreateUITextureProgram();
    InitUIQuad();
    // Intentar cargar la imagen desde rutas posibles
    adventureTexture = LoadTextureFromPNG("Models/Adventure.png");
    if (adventureTexture == 0) adventureTexture = LoadTextureFromPNG("Model/Adventure.png");
    if (adventureTexture == 0) adventureTexture = LoadTextureFromPNG("Models/Adventure");
    if (adventureTexture == 0) adventureTexture = LoadTextureFromPNG("Model/Adventure");

    // --- Shaders ---
    Shader cubeShader("Shader/core.vs", "Shader/core.frag");
    Shader modelShader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader textureShader("Shader/texture.vs", "Shader/texture.frag");
    Shader skyShader("Shader/sky.vs", "Shader/sky.frag");
    Shader depthShader("Shader/depth.vs", "Shader/depth.frag");

    // Configurar unidades de textura en los shaders que las necesitan
    textureShader.Use();
    glUniform1i(glGetUniformLocation(textureShader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(textureShader.Program, "shadowMap"), 1);

    modelShader.Use();
    glUniform1i(glGetUniformLocation(modelShader.Program, "texture_diffuse1"), 0);
    glUniform1i(glGetUniformLocation(modelShader.Program, "shadowMap"), 1);

    // --- Carga de modelos 3D ---
    // Escenario
    Model ladrilloModel((GLchar*)"Models/ladrillos.obj");
    Model metalModel((GLchar*)"Models/metal.obj");
    Model adoquinModel((GLchar*)"Models/adoquin.obj");
    Model aguaModel((GLchar*)"Models/agua.obj");
    Model pastoModel((GLchar*)"Models/pasto.obj");
    Model hojasModel((GLchar*)"Models/Hojas.obj");
    Model pisoModel((GLchar*)"Models/piso.obj");
    Model aguilaModel((GLchar*)"Models/aguila.obj");
    Model barrilModel((GLchar*)"Models/barril.obj");  // Solo en depth pass
    Model cajaModel((GLchar*)"Models/caja.obj");    // Solo en depth pass

    // Tanques
    Model tanquePropioModel((GLchar*)"Models/tanque.obj");   // Jugador
    Model tanque2Model((GLchar*)"Models/tanque2.obj");  // Enemigo gris
    Model tanque3Model((GLchar*)"Models/tanque3.obj");  // Enemigo verde (HP alto)
    Model tanque4Model((GLchar*)"Models/tanque4.obj");  // Enemigo rojo / flash

    // Animaciones de aparicion y explosion
    Model generacion1Model((GLchar*)"Models/generacion1.obj");
    Model generacion2Model((GLchar*)"Models/generacion2.obj");
    Model explosion1Model((GLchar*)"Models/explosion1.obj");
    Model explosion2Model((GLchar*)"Models/explosion2.obj");
    Model explosion3Model((GLchar*)"Models/explosion3.obj");
    Model explosion4Model((GLchar*)"Models/explosion4.obj");
    Model explosion5Model((GLchar*)"Models/explosion5.obj");

    // Proyectil
    Model balaModel((GLchar*)"Models/bala.obj");

    // Puntuacion visual
    Model score100Model((GLchar*)"Models/score_100.obj");
    Model score200Model((GLchar*)"Models/score_200.obj");
    Model score300Model((GLchar*)"Models/score_300.obj");

    // Items coleccionables
    Model cascoModel((GLchar*)"Models/casco.obj");
    Model relojModel((GLchar*)"Models/reloj.obj");
    Model palaModel((GLchar*)"Models/pala.obj");
    Model estrellaModel((GLchar*)"Models/estrella.obj");
    Model granadaModel((GLchar*)"Models/granada.obj");
    Model vidaModel((GLchar*)"Models/tank.obj");

    // --- Inicializacion del estado del juego ---
    InitPlayerFromMap();
    InitEnemySystem();
    InitItems();
    PrintCameraMenu();

    // --- VAO/VBO del cubo unitario ---
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- VAO/VBO del quad de cielo ---
    GLuint skyVAO, skyVBO;
    glGenVertexArrays(1, &skyVAO);
    glGenBuffers(1, &skyVBO);
    glBindVertexArray(skyVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyQuadVertices), skyQuadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // --- Framebuffer y textura para shadow mapping ---
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    GLuint depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1,1,1,1 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ============================================================
    // Bucle principal
    // ============================================================
    while (!glfwWindowShouldClose(window))
    {
        // --- Delta time ---
        GLfloat currentFrame = (GLfloat)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        // --- Logica de juego (solo durante la partida activa) ---
        if (gameScreen == SCREEN_PLAYING)
        {
            DoMovement();
            Animation();
            if (!relojActive)       // El reloj congela a los enemigos
                UpdateEnemies();
            UpdateBullet();
            UpdateEnemyBullets();
            UpdateItems(deltaTime);
            if (playerState == PLAYER_ALIVE)
                CheckItemPickup();
        }

        // --- Avance del sol ---
        if (!sunPaused) sunAngle += sunSpeed * deltaTime;
        if (sunAngle >= glm::pi<float>()) sunAngle = 0.05f;

        // Posicion e intensidad del sol segun el angulo
        glm::vec3 sunPos(SUN_RADIUS * cos(sunAngle), SUN_HEIGHT * sin(sunAngle), -30.0f);
        float noon = sin(sunAngle); // 0 = amanecer/atardecer, 1 = mediodia

        // Color de la luz segun hora del dia
        glm::vec3 sunColor = glm::mix(
            glm::vec3(1.0f, 0.60f, 0.20f),   // Amanecer/atardecer (naranja)
            glm::vec3(1.0f, 0.95f, 0.80f),   // Mediodia (blanco calido)
            glm::clamp(noon, 0.0f, 1.0f));
        float ambientFactor = glm::mix(0.30f, 0.50f, glm::clamp(noon, 0.0f, 1.0f));

        // Matrices de camara y proyeccion
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.GetZoom()),
            (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 200.0f);
        glm::mat4 view = GetCurrentViewMatrix();
        glm::vec3 camPos = GetCurrentCameraPosition();

        // Posicion del sol en NDC (para el shader de lens flare del cielo)
        glm::vec4 sunClip = projection * view * glm::vec4(sunPos, 1.0f);
        glm::vec2 sunNDC(0.5f, 0.5f);
        if (sunClip.w > 0.0f)
        {
            sunNDC.x = sunClip.x / sunClip.w;
            sunNDC.y = sunClip.y / sunClip.w;
        }

        // --- Matriz de espacio de luz para shadow mapping ---
        float shadowOrthoSize = 70.0f;
        glm::mat4 lightProj = glm::ortho(
            -shadowOrthoSize, shadowOrthoSize,
            -shadowOrthoSize, shadowOrthoSize,
            1.0f, 250.0f);
        glm::mat4 lightView = glm::lookAt(sunPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
        glm::mat4 lightSpaceMatrix = lightProj * lightView;

        // =====================================================
        // PASE 1: Generar shadow map
        // =====================================================
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Culling frontal para evitar peter-panning en sombras
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);

        depthShader.Use();
        glUniformMatrix4fv(
            glGetUniformLocation(depthShader.Program, "lightSpaceMatrix"),
            1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

        DrawSceneDepthPass(depthShader, VAO,
            ladrilloModel, metalModel, adoquinModel,
            pisoModel, barrilModel, cajaModel,
            tanquePropioModel, tanque2Model, tanque3Model, tanque4Model);

        glDisable(GL_CULL_FACE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // =====================================================
        // PASE 2: Render principal
        // =====================================================
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        // Color de fondo segun hora del dia
        glm::vec3 bgColor = glm::mix(
            glm::vec3(0.75f, 0.65f, 0.55f),  // Amanecer/atardecer
            glm::vec3(0.18f, 0.48f, 0.85f),  // Mediodia (azul cielo)
            glm::clamp(noon, 0.0f, 1.0f));
        glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Dibujar cielo (quad full-screen sin depth write) ---
        glDepthMask(GL_FALSE);
        skyShader.Use();
        glUniform2f(glGetUniformLocation(skyShader.Program, "sunPos"), sunNDC.x, sunNDC.y);
        glUniform3f(glGetUniformLocation(skyShader.Program, "sunColor"), sunColor.r, sunColor.g, sunColor.b);
        glBindVertexArray(skyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);

        // Enlazar shadow map a unidad de textura 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        // Preparar cubeShader con su localizacion de modelo
        cubeShader.Use();
        GLint cubeModelLoc = glGetUniformLocation(cubeShader.Program, "model");

        // --- Pantallas especiales (no dibujan el mundo 3D) ---
        if (gameScreen == SCREEN_START)
        {
            DrawStartScreen(cubeShader, VAO, cubeModelLoc);
            glfwSwapBuffers(window);
            continue;
        }
        else if (gameScreen == SCREEN_SECRET)
        {
            DrawSecretScreen(cubeShader, VAO, cubeModelLoc);
            glfwSwapBuffers(window);
            continue;
        }
        else if (gameScreen == SCREEN_WIN)
        {
            DrawFinalScreen(cubeShader, VAO, cubeModelLoc, true);
            glfwSwapBuffers(window);
            continue;
        }
        else if (gameScreen == SCREEN_LOSE)
        {
            DrawFinalScreen(cubeShader, VAO, cubeModelLoc, false);
            glfwSwapBuffers(window);
            continue;
        }

        // Matrices de vista/proyeccion en cubeShader
        glUniformMatrix4fv(glGetUniformLocation(cubeShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(cubeShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Lambda: enviar uniforms de luz a un shader dado
        auto SetLightUniforms = [&](Shader& sh)
            {
                sh.Use();
                glUniformMatrix4fv(glGetUniformLocation(sh.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
                glUniformMatrix4fv(glGetUniformLocation(sh.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
                glUniformMatrix4fv(glGetUniformLocation(sh.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
                glUniform3f(glGetUniformLocation(sh.Program, "lightPos"), sunPos.x, sunPos.y, sunPos.z);
                glUniform3f(glGetUniformLocation(sh.Program, "lightColor"), sunColor.r, sunColor.g, sunColor.b);
                glUniform3f(glGetUniformLocation(sh.Program, "viewPos"), camPos.x, camPos.y, camPos.z);
                glUniform1f(glGetUniformLocation(sh.Program, "ambientFactor"), ambientFactor);
                glUniform1i(glGetUniformLocation(sh.Program, "shadowMap"), 1);
            };

        SetLightUniforms(modelShader);
        SetLightUniforms(textureShader);

        // Resetear flags de efecto especial en textureShader
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        glUniform1f(glGetUniformLocation(textureShader.Program, "waterTime"), (float)glfwGetTime());
        glUniform1f(glGetUniformLocation(textureShader.Program, "shieldTime"), (float)glfwGetTime());

        // --- Dibujar escena principal ---
        DrawTankCityMap(
            cubeShader, modelShader, textureShader,
            VAO, cubeModelLoc,
            ladrilloModel, metalModel, adoquinModel,
            aguaModel, pastoModel, hojasModel, pisoModel,
            aguilaModel, barrilModel, cajaModel,
            tanquePropioModel, tanque2Model, tanque3Model, tanque4Model,
            generacion1Model, generacion2Model,
            explosion1Model, explosion2Model, explosion3Model,
            explosion4Model, explosion5Model,
            balaModel,
            score100Model, score200Model, score300Model,
            cascoModel, relojModel, palaModel,
            estrellaModel, granadaModel, vidaModel);

        DrawItemHUD(cubeShader, VAO, cubeModelLoc);
        glfwSwapBuffers(window);
    }

    // --- Limpieza de recursos OpenGL ---
    glDeleteVertexArrays(1, &VAO);     glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &skyVAO);  glDeleteBuffers(1, &skyVBO);
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);

#ifdef _WIN32
    if (gdiplusToken != 0) Gdiplus::GdiplusShutdown(gdiplusToken);
#endif

    glfwTerminate();
    return 0;
}

// ============================================================
// PANTALLAS RETRO
// Dibujadas con cubos en 2D (sin dependencia de fuentes externas)
// ============================================================

// Inicia una nueva partida desde el menu principal
void StartGameFromMenu()
{
    secretBuffer.clear();
    gameScreen = SCREEN_PLAYING;
    playerState = PLAYER_SPAWNING;
    playerAnimTimer = 0.0f;
    spawnFrame = 0;
    explosionFrame = 0;
    playerRotY = 180.0f;
    bulletState = BULLET_OFF;
    bullet2State = BULLET_OFF;
    InitPlayerFromMap();
}

// Procesa teclas en la pantalla de inicio para detectar el codigo secreto
void ProcessSecretCode(int key)
{
    char c = 0;
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z)   c = (char)('A' + (key - GLFW_KEY_A));
    else if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)   c = (char)('0' + (key - GLFW_KEY_0));
    else if (key >= GLFW_KEY_KP_0 && key <= GLFW_KEY_KP_9) c = (char)('0' + (key - GLFW_KEY_KP_0));
    else return;

    secretBuffer += c;

    // Mantener solo los ultimos N caracteres necesarios
    if (secretBuffer.size() > SECRET_CODE.size())
        secretBuffer.erase(0, secretBuffer.size() - SECRET_CODE.size());

    if (secretBuffer == SECRET_CODE)
    {
        secretBuffer.clear();
        gameScreen = SCREEN_SECRET;
        return;
    }

    // Si el buffer ya no es prefijo del codigo, reiniciar
    if (SECRET_CODE.find(secretBuffer) != 0)
    {
        std::string last(1, c);
        secretBuffer = (SECRET_CODE.find(last) == 0) ? last : "";
    }
}

// Devuelve el patron de pixels (7 filas x 5 cols) de un caracter retro
std::vector<std::string> GetRetroChar(char c)
{
    switch (c)
    {
    case 'A': return { "01110","10001","10001","11111","10001","10001","10001" };
    case 'B': return { "11110","10001","10001","11110","10001","10001","11110" };
    case 'C': return { "01111","10000","10000","10000","10000","10000","01111" };
    case 'D': return { "11110","10001","10001","10001","10001","10001","11110" };
    case 'E': return { "11111","10000","10000","11110","10000","10000","11111" };
    case 'F': return { "11111","10000","10000","11110","10000","10000","10000" };
    case 'G': return { "01111","10000","10000","10111","10001","10001","01111" };
    case 'H': return { "10001","10001","10001","11111","10001","10001","10001" };
    case 'I': return { "11111","00100","00100","00100","00100","00100","11111" };
    case 'J': return { "00111","00010","00010","00010","10010","10010","01100" };
    case 'K': return { "10001","10010","10100","11000","10100","10010","10001" };
    case 'L': return { "10000","10000","10000","10000","10000","10000","11111" };
    case 'M': return { "10001","11011","10101","10101","10001","10001","10001" };
    case 'N': return { "10001","11001","10101","10011","10001","10001","10001" };
    case 'O': return { "01110","10001","10001","10001","10001","10001","01110" };
    case 'P': return { "11110","10001","10001","11110","10000","10000","10000" };
    case 'Q': return { "01110","10001","10001","10001","10101","10010","01101" };
    case 'R': return { "11110","10001","10001","11110","10100","10010","10001" };
    case 'S': return { "01111","10000","10000","01110","00001","00001","11110" };
    case 'T': return { "11111","00100","00100","00100","00100","00100","00100" };
    case 'U': return { "10001","10001","10001","10001","10001","10001","01110" };
    case 'V': return { "10001","10001","10001","10001","10001","01010","00100" };
    case 'W': return { "10001","10001","10001","10101","10101","10101","01010" };
    case 'X': return { "10001","10001","01010","00100","01010","10001","10001" };
    case 'Y': return { "10001","10001","01010","00100","00100","00100","00100" };
    case 'Z': return { "11111","00001","00010","00100","01000","10000","11111" };
    case '0': return { "01110","10001","10011","10101","11001","10001","01110" };
    case '1': return { "00100","01100","00100","00100","00100","00100","01110" };
    case '2': return { "01110","10001","00001","00010","00100","01000","11111" };
    case '3': return { "11110","00001","00001","01110","00001","00001","11110" };
    case '4': return { "00010","00110","01010","10010","11111","00010","00010" };
    case '5': return { "11111","10000","10000","11110","00001","00001","11110" };
    case '6': return { "01110","10000","10000","11110","10001","10001","01110" };
    case '7': return { "11111","00001","00010","00100","01000","01000","01000" };
    case '8': return { "01110","10001","10001","01110","10001","10001","01110" };
    case '9': return { "01110","10001","10001","01111","00001","00001","01110" };
    case ':': return { "00000","00100","00100","00000","00100","00100","00000" };
    case '-': return { "00000","00000","00000","11111","00000","00000","00000" };
    case '=': return { "00000","11111","00000","11111","00000","00000","00000" };
    default:  return { "00000","00000","00000","00000","00000","00000","00000" };
    }
}

// Dibuja texto en pantalla usando pequenos cubos (estilo pixel art retro)
// x,y: posicion en pixeles desde esquina superior izquierda
// px: tamanio de cada pixel del caracter
void DrawRetroText(Shader& shader, GLuint VAO, GLint modelLoc,
    const std::string& text, float x, float y, float px, const glm::vec3& color)
{
    shader.Use();
    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));

    float cursorX = x;
    for (char raw : text)
    {
        char c = (raw >= 'a' && raw <= 'z') ? (raw - 'a' + 'A') : raw; // Normalizar a mayusculas
        if (c == ' ')
        {
            cursorX += px * 4.0f;
            continue;
        }

        std::vector<std::string> pattern = GetRetroChar(c);
        for (int row = 0; row < 7; row++)
            for (int col = 0; col < 5; col++)
                if (pattern[row][col] == '1')
                {
                    float sx = cursorX + col * px;
                    float sy = (float)HEIGHT - y - row * px; // Y invertida (OpenGL)
                    DrawCube(shader, VAO, modelLoc,
                        glm::vec3(sx, sy, 0.0f),
                        glm::vec3(px * 0.82f, px * 0.82f, 1.0f),
                        color);
                }
        cursorX += px * 6.0f; // Avance horizontal: 5 cols + 1 espacio
    }
}

// Dibuja un icono simplificado de tanque con cubos (para las pantallas de menu)
void DrawRetroTankIcon(Shader& shader, GLuint VAO, GLint modelLoc,
    float x, float y, const glm::vec3& color)
{
    shader.Use();
    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));

    // Cuerpo principal
    DrawCube(shader, VAO, modelLoc, glm::vec3(x, HEIGHT - y, 0.0f), glm::vec3(44.0f, 24.0f, 1.0f), color);
    // Torreta
    DrawCube(shader, VAO, modelLoc, glm::vec3(x, HEIGHT - y + 18.0f, 0.0f), glm::vec3(24.0f, 20.0f, 1.0f), color);
    // Canon
    DrawCube(shader, VAO, modelLoc, glm::vec3(x + 28.0f, HEIGHT - y + 18.0f, 0.0f), glm::vec3(34.0f, 7.0f, 1.0f), color);
    // Orugas
    DrawCube(shader, VAO, modelLoc, glm::vec3(x - 17.0f, HEIGHT - y - 17.0f, 0.0f), glm::vec3(16.0f, 8.0f, 1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(x + 17.0f, HEIGHT - y - 17.0f, 0.0f), glm::vec3(16.0f, 8.0f, 1.0f), glm::vec3(0.05f, 0.05f, 0.05f));
}

// Pantalla de inicio
void DrawStartScreen(Shader& shader, GLuint VAO, GLint modelLoc)
{
    glDisable(GL_DEPTH_TEST);
    shader.Use();
    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));

    // Fondo y marco
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(WIDTH, HEIGHT, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(900.0f, 430.0f, 1.0f), glm::vec3(0.08f, 0.08f, 0.10f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(860.0f, 390.0f, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));

    DrawRetroText(shader, VAO, modelLoc, "TANK CITY", 330.0f, 145.0f, 18.0f, glm::vec3(0.95f, 0.82f, 0.25f));
    DrawRetroText(shader, VAO, modelLoc, "PRESIONA ESPACIO", 345.0f, 350.0f, 9.0f, glm::vec3(0.95f, 0.95f, 0.95f));
    DrawRetroText(shader, VAO, modelLoc, "PARA INICIAR", 410.0f, 405.0f, 9.0f, glm::vec3(0.95f, 0.95f, 0.95f));
    DrawRetroTankIcon(shader, VAO, modelLoc, 250.0f, 345.0f, glm::vec3(0.20f, 0.80f, 0.25f));
    DrawRetroTankIcon(shader, VAO, modelLoc, 1030.0f, 345.0f, glm::vec3(0.80f, 0.20f, 0.20f));

    glEnable(GL_DEPTH_TEST);
}

// Pantalla de fin de partida (victoria o derrota) con estadisticas
void DrawFinalScreen(Shader& shader, GLuint VAO, GLint modelLoc, bool win)
{
    glDisable(GL_DEPTH_TEST);
    shader.Use();
    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));

    int destroyed = grayDestroyed + redDestroyed + greenDestroyed;
    int remaining = TOTAL_ENEMIES_PER_LEVEL - destroyed;
    if (remaining < 0) remaining = 0;

    // Fondo y marco
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(WIDTH, HEIGHT, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(980.0f, 610.0f, 1.0f), glm::vec3(0.08f, 0.08f, 0.10f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(940.0f, 570.0f, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));

    if (win)
        DrawRetroText(shader, VAO, modelLoc, "YOU WIN", 410.0f, 70.0f, 18.0f, glm::vec3(0.25f, 0.95f, 0.35f));
    else
        DrawRetroText(shader, VAO, modelLoc, "YOU LOSE", 385.0f, 70.0f, 18.0f, glm::vec3(0.95f, 0.25f, 0.25f));

    DrawRetroText(shader, VAO, modelLoc, "REPORTE FINAL",
        405.0f, 180.0f, 8.0f, glm::vec3(0.95f, 0.82f, 0.25f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES TOTALES: " + std::to_string(TOTAL_ENEMIES_PER_LEVEL),
        285.0f, 250.0f, 6.0f, glm::vec3(0.90f, 0.90f, 0.90f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES GRISES: " + std::to_string(grayDestroyed) + " X 100 = " + std::to_string(grayDestroyed * 100),
        285.0f, 295.0f, 6.0f, glm::vec3(0.75f, 0.75f, 0.75f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES ROJOS: " + std::to_string(redDestroyed) + " X 200 = " + std::to_string(redDestroyed * 200),
        285.0f, 340.0f, 6.0f, glm::vec3(0.95f, 0.35f, 0.35f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES VERDES: " + std::to_string(greenDestroyed) + " X 300 = " + std::to_string(greenDestroyed * 300),
        285.0f, 385.0f, 6.0f, glm::vec3(0.35f, 0.95f, 0.35f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES DESTRUIDOS: " + std::to_string(destroyed),
        285.0f, 430.0f, 6.0f, glm::vec3(0.90f, 0.90f, 0.90f));
    DrawRetroText(shader, VAO, modelLoc,
        "TANQUES RESTANTES: " + std::to_string(remaining),
        285.0f, 475.0f, 6.0f, glm::vec3(0.90f, 0.90f, 0.90f));
    DrawRetroText(shader, VAO, modelLoc,
        "PUNTAJE FINAL: " + std::to_string(score),
        285.0f, 520.0f, 6.0f, glm::vec3(0.95f, 0.82f, 0.25f));
    DrawRetroText(shader, VAO, modelLoc,
        "PRESIONA ESPACIO PARA VOLVER A EMPEZAR",
        215.0f, 625.0f, 5.0f, glm::vec3(0.95f, 0.95f, 0.95f));

    glEnable(GL_DEPTH_TEST);
}

// Pantalla secreta (accesible con codigo secreto desde el menu)
void DrawSecretScreen(Shader& shader, GLuint VAO, GLint modelLoc)
{
    glDisable(GL_DEPTH_TEST);
    shader.Use();
    glm::mat4 uiProjection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT, -10.0f, 10.0f);
    glm::mat4 uiView = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));

    // Fondo y marco
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(WIDTH, HEIGHT, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(980.0f, 640.0f, 1.0f), glm::vec3(0.08f, 0.08f, 0.10f));
    DrawCube(shader, VAO, modelLoc, glm::vec3(WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f), glm::vec3(940.0f, 600.0f, 1.0f), glm::vec3(0.02f, 0.02f, 0.04f));

    if (adventureTexture != 0)
        DrawUIImage(adventureTexture, 340.0f, 90.0f, 600.0f, 430.0f);
    else
        DrawRetroText(shader, VAO, modelLoc, "NO SE ENCONTRO ADVENTURE.PNG",
            270.0f, 270.0f, 6.0f, glm::vec3(0.95f, 0.35f, 0.35f));

    // Restaurar shader de cubos despues de DrawUIImage
    shader.Use();
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(uiView));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(uiProjection));
    DrawRetroText(shader, VAO, modelLoc, "PRESIONA R PARA REGRESAR",
        330.0f, 620.0f, 6.0f, glm::vec3(0.95f, 0.95f, 0.95f));

    glEnable(GL_DEPTH_TEST);
}

// ============================================================
// UI CON TEXTURAS (imagen PNG para pantalla secreta)
// ============================================================

// Compila un shader GLSL desde codigo fuente en memoria
GLuint CompileShaderFromSource(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    return shader;
}

// Crea el programa de shader para dibujar un quad con textura en pantalla
GLuint CreateUITextureProgram()
{
    const char* vs =
        "#version 330 core\n"
        "layout(location=0) in vec2 aPos;\n"
        "layout(location=1) in vec2 aTex;\n"
        "out vec2 TexCoord;\n"
        "void main(){ TexCoord=aTex; gl_Position=vec4(aPos,0.0,1.0); }\n";

    const char* fs =
        "#version 330 core\n"
        "in vec2 TexCoord;\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D uiTexture;\n"
        "void main(){ FragColor=texture(uiTexture, TexCoord); }\n";

    GLuint v = CompileShaderFromSource(GL_VERTEX_SHADER, vs);
    GLuint f = CompileShaderFromSource(GL_FRAGMENT_SHADER, fs);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);
    glDeleteShader(f);
    return prog;
}

// Inicializa el VAO/VBO del quad de UI (vertices dinamicos)
void InitUIQuad()
{
    glGenVertexArrays(1, &uiQuadVAO);
    glGenBuffers(1, &uiQuadVBO);
    glBindVertexArray(uiQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, NULL, GL_DYNAMIC_DRAW);
    // Atributo 0: posicion (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Atributo 1: coordenada UV (vec2)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

// Carga una imagen PNG como textura OpenGL (solo Windows via GDI+)
GLuint LoadTextureFromPNG(const char* path)
{
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, path, -1, NULL, 0);
    if (len <= 0) return 0;
    std::vector<wchar_t> wpath(len);
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath.data(), len);

    Gdiplus::Bitmap bitmap(wpath.data());
    if (bitmap.GetLastStatus() != Gdiplus::Ok) return 0;

    int width = bitmap.GetWidth();
    int height = bitmap.GetHeight();
    Gdiplus::Rect rect(0, 0, width, height);
    Gdiplus::BitmapData data;
    if (bitmap.LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &data) != Gdiplus::Ok)
        return 0;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // GDI+ entrega los canales como BGRA, OpenGL necesita GL_BGRA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, data.Scan0);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    bitmap.UnlockBits(&data);
    return tex;
#else
    return 0; // Sin soporte en plataformas no-Windows
#endif
}

// Dibuja una imagen texturizada en coordenadas de pantalla (pixeles)
// x,y: esquina superior izquierda | w,h: ancho y alto en pixeles
void DrawUIImage(GLuint texture, float x, float y, float w, float h)
{
    if (texture == 0 || uiTextureProgram == 0 || uiQuadVAO == 0) return;

    // Convertir coordenadas de pantalla a NDC [-1, 1]
    float left = (x / WIDTH) * 2.0f - 1.0f;
    float right = ((x + w) / WIDTH) * 2.0f - 1.0f;
    float top = 1.0f - (y / HEIGHT) * 2.0f;
    float bottom = 1.0f - ((y + h) / HEIGHT) * 2.0f;

    // 6 vertices: 2 triangulos formando un quad
    // Formato: [posX, posY, u, v]
    float verts[] = {
        left,  bottom, 0.0f, 1.0f,
        right, bottom, 1.0f, 1.0f,
        right, top,    1.0f, 0.0f,
        left,  bottom, 0.0f, 1.0f,
        right, top,    1.0f, 0.0f,
        left,  top,    0.0f, 0.0f
    };

    glUseProgram(uiTextureProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(uiTextureProgram, "uiTexture"), 0);

    glBindVertexArray(uiQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiQuadVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ============================================================
// UTILIDADES DEL MAPA
// ============================================================

// Convierte posicion de tile (columna, fila) a coordenadas de mundo
glm::vec3 TileToWorld(int tileX, int tileZ)
{
    int cols = (int)mapa[0].size();
    int rows = (int)mapa.size();
    float ox = (cols * TILE) / 2.0f;
    float oz = (rows * TILE) / 2.0f;
    return glm::vec3(
        (tileX * TILE) - ox + TILE / 2.0f,
        1.4f,
        (tileZ * TILE) - oz + TILE / 2.0f
    );
}

// ============================================================
// SISTEMA DE ITEMS
// ============================================================

// Inicializa los slots de items (ninguno activo al inicio)
void InitItems()
{
    items.clear();
    for (int i = 0; i < 6; i++)
    {
        Item it;
        it.type = (ItemType)i;
        it.worldPos = glm::vec3(0.0f, 1.4f, 0.0f);
        it.active = false;
        it.bobTimer = 0.0f;
        items.push_back(it);
    }
    std::cout << "[ITEMS] Slots preparados: casco, reloj, pala, estrella, granada y vida.\n";
}

// Actualiza la animacion de flotacion de items y la duracion de efectos activos
void UpdateItems(float dt)
{
    // Animacion de flotacion (oscila en Y)
    for (auto& it : items)
        if (it.active)
        {
            it.bobTimer += dt;
            it.worldPos.y = 1.4f + 0.3f * sinf(it.bobTimer * 2.5f);
        }

    // Decrementar timers de efectos activos
    if (cascoActive)
    {
        cascoTimer -= dt;
        if (cascoTimer <= 0.0f)
        {
            cascoActive = false;
            cascoTimer = 0.0f;
            std::cout << "[CASCO] Proteccion terminada.\n";
        }
    }
    if (relojActive)
    {
        relojTimer -= dt;
        if (relojTimer <= 0.0f)
        {
            relojActive = false;
            relojTimer = 0.0f;
            std::cout << "[RELOJ] Tiempo terminado.\n";
        }
    }
    if (palaActive)
    {
        palaTimer -= dt;
        if (palaTimer <= 0.0f)
        {
            RestorePalaEffect();
            palaActive = false;
            palaTimer = 0.0f;
            std::cout << "[PALA] Tiempo terminado. Bloques restaurados.\n";
        }
    }
    if (maxPower)
    {
        maxPowerTimer -= dt;
        if (maxPowerTimer <= 0.0f)
        {
            maxPower = false;
            maxPowerTimer = 0.0f;
            bulletSpeed = BULLET_SPEED_NORMAL;
            std::cout << "[PODER] Poder maximo terminado.\n";
        }
    }
}

// Comprueba si el jugador toca algun item y aplica su efecto
void CheckItemPickup()
{
    for (auto& it : items)
    {
        if (!it.active) continue;

        float dx = playerPos.x - it.worldPos.x;
        float dz = playerPos.z - it.worldPos.z;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist <= ITEM_PICKUP_DIST)
        {
            it.active = false;
            itemsCollectedTotal++;
            switch (it.type)
            {
            case ITEM_CASCO:
                cascoCollected++;
                cascoActive = true;
                cascoTimer = CASCO_DURATION;
                std::cout << "[ITEM] Casco recogido! Proteccion 30s\n";
                break;
            case ITEM_RELOJ:
                relojCollected++;
                relojActive = true;
                relojTimer = RELOJ_DURATION;
                std::cout << "[ITEM] Reloj recogido! Tanques detenidos 30s\n";
                break;
            case ITEM_PALA:
                palaCollected++;
                ApplyPalaEffect();
                palaActive = true;
                palaTimer = pala_DURATION;
                std::cout << "[ITEM] Pala recogida! Bloques alrededor del aguila convertidos a metal por 30s.\n";
                break;
            case ITEM_ESTRELLA:
                estrellaCollected++;
                ActivateMaxPower();
                std::cout << "[ITEM] Poder maximo por 30s\n";
                break;
            case ITEM_GRANADA:
                granadaCollected++;
                DestroyEnemiesWithGrenade();
                std::cout << "[ITEM] Granada: enemigos destruidos.\n";
                break;
            case ITEM_VIDA:
                vidaCollected++;
                playerLives++;
                std::cout << "[ITEM] Vida extra. Vidas: " << playerLives << "\n";
                break;
            }
        }
    }
}

// Convierte los tiles de ladrillo adyacentes al aguila en metal durante 30s
void ApplyPalaEffect()
{
    if (!palaConvertedTiles.empty())
        RestorePalaEffect(); // Restaurar antes de aplicar de nuevo

    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    for (int z = 0; z < rows; z++)
        for (int x = 0; x < cols; x++)
            if (mapa[z][x] == 'G')
                for (int dz = -2; dz <= 2; dz++)
                    for (int dx = -2; dx <= 2; dx++)
                    {
                        int nx = x + dx, nz = z + dz;
                        if (nx >= 0 && nx < cols && nz >= 0 && nz < rows && mapa[nz][nx] == 'B')
                        {
                            mapa[nz][nx] = 'M';
                            palaConvertedTiles.push_back({ nx, nz });
                        }
                    }
    std::cout << "[PALA] Bloques alrededor del aguila convertidos a metal por 30s.\n";
}

// Restaura los tiles que convirtio la pala a su tipo original (ladrillo)
void RestorePalaEffect()
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    for (auto& pos : palaConvertedTiles)
    {
        int x = pos.first, z = pos.second;
        if (x >= 0 && x < cols && z >= 0 && z < rows && mapa[z][x] == 'M')
            mapa[z][x] = 'B';
    }
    palaConvertedTiles.clear();
}

// Dibuja todos los items activos con rotacion y flotacion animadas
void DrawItems(Shader& textureShader,
    Model& cascoModel, Model& relojModel, Model& palaModel,
    Model& estrellaModel, Model& granadaModel, Model& vidaModel)
{
    float rotY = (float)glfwGetTime() * 90.0f; // Rotacion continua

    for (auto& it : items)
    {
        if (!it.active) continue;

        Model* mdl = nullptr;
        glm::vec3 sc(0.55f, 0.55f, 0.55f);

        switch (it.type)
        {
        case ITEM_CASCO:    mdl = &cascoModel;    break;
        case ITEM_RELOJ:    mdl = &relojModel;    break;
        case ITEM_PALA:     mdl = &palaModel;     break;
        case ITEM_ESTRELLA: mdl = &estrellaModel; break;
        case ITEM_GRANADA:  mdl = &granadaModel;  break;
        case ITEM_VIDA:     mdl = &vidaModel;     break;
        }
        if (!mdl) continue;

        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "texture_diffuse1"), 0);

        glm::mat4 m = glm::translate(glm::mat4(1.0f), it.worldPos);
        m = glm::rotate(m, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
        m = glm::scale(m, sc);
        glUniformMatrix4fv(glGetUniformLocation(textureShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
        mdl->Draw(textureShader);
    }
}

// Genera un item aleatorio en la posicion donde fue destruido un tanque rojo
void SpawnRandomItemAt(const glm::vec3& pos)
{
    ItemType type = (ItemType)(rand() % 6); // Todos los tipos con igual probabilidad

    // Buscar slot libre o agregar uno nuevo
    int slot = -1;
    for (int i = 0; i < (int)items.size(); i++)
        if (!items[i].active) { slot = i; break; }

    if (slot == -1)
    {
        Item newItem;
        newItem.type = type;
        newItem.worldPos = glm::vec3(pos.x, 1.4f, pos.z);
        newItem.active = true;
        newItem.bobTimer = 0.0f;
        items.push_back(newItem);
    }
    else
    {
        items[slot].type = type;
        items[slot].worldPos = glm::vec3(pos.x, 1.4f, pos.z);
        items[slot].active = true;
        items[slot].bobTimer = 0.0f;
    }
    std::cout << "[ITEM] Item generado por tanque rojo.\n";
}

// Devuelve la alpha de transparencia de las hojas en una posicion dada
// (las hojas se vuelven transparentes cuando el jugador esta debajo)
float GetLeafAlphaAt(float wx, float wz)
{
    if (playerState != PLAYER_ALIVE) return 0.0f;
    float dx = playerPos.x - wx;
    float dz = playerPos.z - wz;
    float dist = sqrtf(dx * dx + dz * dz);
    if (dist > LEAF_RADIUS) return 0.0f;
    return glm::clamp(1.0f - glm::smoothstep(TILE * 0.4f, LEAF_RADIUS, dist), 0.0f, 0.80f);
}

// ============================================================
// PASE DE SOMBRAS (depth pass)
// Solo dibuja la geometria solida que proyecta sombras
// ============================================================
void DrawSceneDepthPass(
    Shader& depthShader, GLuint VAO,
    Model& ladrilloModel, Model& metalModel, Model& adoquinModel,
    Model& pisoModel, Model& barrilModel, Model& cajaModel,
    Model& tanquePropioModel, Model& tanque2Model,
    Model& tanque3Model, Model& tanque4Model)
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float ox = (cols * TILE) / 2.0f, oz = (rows * TILE) / 2.0f;

    // Escalas fijas para el depth pass
    glm::vec3 pisoScale(0.40f, 1, 0.40f);
    glm::vec3 brickScale(1, 1, 1), metalScale(1, 1, 1), adoquinScale(1, 1, 1);
    glm::vec3 barrilScale(0.5f, 0.5f, 0.5f), cajaScale(0.4f, 0.4f, 0.4f);
    glm::vec3 tanqueScale(0.5f, 0.5f, 0.5f);
    glm::vec3 rot0(0);

    depthShader.Use();
    for (int z = 0; z < rows; z++)
        for (int x = 0; x < cols; x++)
        {
            char  tile = mapa[z][x];
            float wx = (x * TILE) - ox + TILE / 2;
            float wz = (z * TILE) - oz + TILE / 2;

            DrawTexturedModelAtDepth(pisoModel, depthShader, glm::vec3(wx, 0, wz), pisoScale, rot0);

            switch (tile)
            {
            case 'B': DrawTexturedModelAtDepth(ladrilloModel, depthShader, glm::vec3(wx, 0.90f, wz), brickScale, rot0); break;
            case 'M': DrawTexturedModelAtDepth(metalModel, depthShader, glm::vec3(wx, 0.92f, wz), metalScale, rot0); break;
            case 'A': DrawTexturedModelAtDepth(adoquinModel, depthShader, glm::vec3(wx, 1.12f, wz), adoquinScale, rot0); break;
            case 'R': DrawTexturedModelAtDepth(barrilModel, depthShader, glm::vec3(wx, 1.50f, wz), barrilScale, rot0); break;
            case 'C': DrawTexturedModelAtDepth(cajaModel, depthShader, glm::vec3(wx, 0.32f, wz), cajaScale, rot0); break;
                // Tanques enemigos fijos en el mapa (puntos de spawn visuales)
            case 'T': DrawTexturedModelAtDepth(tanque2Model, depthShader, glm::vec3(wx, PLAYER_Y, wz), tanqueScale, glm::vec3(0, 180, 0)); break;
            case 'Y': DrawTexturedModelAtDepth(tanque3Model, depthShader, glm::vec3(wx, PLAYER_Y, wz), tanqueScale, glm::vec3(0, 180, 0)); break;
            case 'Z': DrawTexturedModelAtDepth(tanque4Model, depthShader, glm::vec3(wx, PLAYER_Y, wz), tanqueScale, glm::vec3(0, 180, 0)); break;
            default: break;
            }
        }

    // Sombra del tanque del jugador
    if (playerState == PLAYER_ALIVE)
        DrawTexturedModelAtDepth(tanquePropioModel, depthShader,
            playerPos, tanqueScale, glm::vec3(0, playerRotY, 0));

    // Sombra del tanque gris animado por keyframes
    if (grayTankVisible)
        DrawTexturedModelAtDepth(tanque2Model, depthShader,
            glm::vec3(grayKFPosX, grayKFPosY, grayKFPosZ), tanqueScale, glm::vec3(0, grayKFRotY, 0));
}

// ============================================================
// DIBUJADO DEL MAPA Y ENTIDADES
// ============================================================
void DrawTankCityMap(
    Shader& cubeShader, Shader& modelShader, Shader& textureShader,
    GLuint VAO, GLint cubeModelLoc,
    Model& ladrilloModel, Model& metalModel, Model& adoquinModel,
    Model& aguaModel, Model& pastoModel, Model& hojasModel, Model& pisoModel,
    Model& aguilaModel, Model& barrilModel, Model& cajaModel,
    Model& tanquePropioModel, Model& tanque2Model, Model& tanque3Model, Model& tanque4Model,
    Model& generacion1Model, Model& generacion2Model,
    Model& explosion1Model, Model& explosion2Model, Model& explosion3Model,
    Model& explosion4Model, Model& explosion5Model,
    Model& balaModel,
    Model& score100Model, Model& score200Model, Model& score300Model,
    Model& cascoModel, Model& relojModel, Model& palaModel,
    Model& estrellaModel, Model& granadaModel, Model& vidaModel)
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float ox = (cols * TILE) / 2.0f, oz = (rows * TILE) / 2.0f;

    // Borde exterior del mapa
    cubeShader.Use();
    DrawCube(cubeShader, VAO, cubeModelLoc,
        glm::vec3(0, -0.10f, 0),
        glm::vec3(cols * TILE + 4.0f, 0.05f, rows * TILE + 4.0f),
        glm::vec3(0.08f, 0.08f, 0.08f));

    // Escalas de modelos
    glm::vec3 brickScale(1, 1, 1), metalScale(1, 1, 1), adoquinScale(1, 1, 1);
    glm::vec3 aguaScale(1, 0.03f, 0.5f), hojasScale(1, 1, 1), pisoScale(0.4f, 1, 0.4f);
    glm::vec3 aguilaScale(0.35f, 0.55f, 0.35f);
    glm::vec3 tanquePropioScale(0.65f, 0.65f, 0.65f);
    glm::vec3 tanque2Scale(0.65f, 0.65f, 0.65f);
    glm::vec3 tanque3Scale(0.65f, 0.65f, 0.65f);
    glm::vec3 tanque4Scale(0.65f, 0.65f, 0.65f);
    glm::vec3 expScale(1.25f, 1.25f, 1.25f), scoreScale(1.15f, 1.15f, 1.15f);
    glm::vec3 genScale(1, 1, 1), rot0(0);
    float currentTime = (float)glfwGetTime();

    // ---- Dibujar tiles del mapa ----
    for (int z = 0; z < rows; z++)
    {
        for (int x = 0; x < cols; x++)
        {
            char  tile = mapa[z][x];
            float wx = (x * TILE) - ox + TILE / 2.0f;
            float wz = (z * TILE) - oz + TILE / 2.0f;

            // Piso en todos los tiles
            textureShader.Use();
            glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
            glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
            DrawTexturedModelAt(pisoModel, textureShader, glm::vec3(wx, 0.0f, wz), pisoScale, rot0);

            if (tile == 'H') continue; // Las hojas se dibujan al final (semitransparentes)

            switch (tile)
            {
                // Tiles sin modelo propio adicional (solo piso)
            case '.': case 'P': case 'T': case 'Y': case 'Z': case 'E': case 'C': case 'R':
                break;

            case 'M':
                DrawTexturedModelAt(metalModel, textureShader, glm::vec3(wx, 0.92f, wz), metalScale, rot0);
                break;
            case 'B':
                DrawTexturedModelAt(ladrilloModel, textureShader, glm::vec3(wx, 0.90f, wz), brickScale, rot0);
                break;
            case 'A':
                DrawTexturedModelAt(adoquinModel, textureShader, glm::vec3(wx, 1.12f, wz), adoquinScale, rot0);
                break;
            case 'W':
                // Agua con efecto de ondulacion (via uniform waterTime)
                glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 1);
                glUniform1f(glGetUniformLocation(textureShader.Program, "waterTime"), currentTime);
                glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
                glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
                DrawTexturedModelAt(aguaModel, textureShader, glm::vec3(wx, 0.02f, wz), aguaScale, rot0);
                glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
                break;

            case 'G': // Aguila (base a proteger)
            {
                // Base amarilla del aguila
                cubeShader.Use();
                DrawCube(cubeShader, VAO, cubeModelLoc,
                    glm::vec3(wx, 0.0f, wz), glm::vec3(2, 0.15f, 2), glm::vec3(0.65f, 0.55f, 0.20f));

                textureShader.Use();
                glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
                glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
                glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
                glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);

                eaglePos = glm::vec3(wx, 0.19f, wz);

                if (eagleAlive)
                    DrawTexturedModelAt(aguilaModel, textureShader, eaglePos, aguilaScale, rot0);
                else if (eagleExploding)
                {
                    // Animacion de explosion del aguila
                    glm::vec3 ePos = eaglePos; ePos.y = 0.90f;
                    if (eagleExplosionFrame == 0) DrawTexturedModelAt(explosion1Model, textureShader, ePos, expScale, rot0);
                    else if (eagleExplosionFrame == 1) DrawTexturedModelAt(explosion2Model, textureShader, ePos, expScale, rot0);
                    else if (eagleExplosionFrame == 2) DrawTexturedModelAt(explosion3Model, textureShader, ePos, expScale, rot0);
                    else if (eagleExplosionFrame == 3) DrawTexturedModelAt(explosion4Model, textureShader, ePos, expScale, rot0);
                    else                               DrawTexturedModelAt(explosion5Model, textureShader, ePos, expScale, rot0);
                }
                break;
            }
            default: break;
            }
        }
    }

    // ---- Jugador ----
    textureShader.Use();
    glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
    glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
    glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
    glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);

    glm::vec3 animPos = playerPos; animPos.y = 0.90f;

    if (playerState == PLAYER_SPAWNING)
    {
        // Animacion de aparicion (dos frames alternados)
        if (spawnFrame == 0) DrawTexturedModelAt(generacion1Model, textureShader, animPos, genScale, rot0);
        else                 DrawTexturedModelAt(generacion2Model, textureShader, animPos, genScale, rot0);
    }
    else if (playerState == PLAYER_ALIVE && cameraMode != CAMERA_FIRST_PERSON)
    {
        // El escudo del casco se renderiza con efecto especial
        if (cascoActive)
        {
            glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 1);
            glUniform1f(glGetUniformLocation(textureShader.Program, "shieldTime"), currentTime);
        }
        DrawTexturedModelAt(tanquePropioModel, textureShader, playerPos, tanquePropioScale, glm::vec3(0, playerRotY, 0));
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
    }
    else if (playerState == PLAYER_EXPLODING)
    {
        // Animacion de explosion del jugador
        if (explosionFrame == 0) DrawTexturedModelAt(explosion1Model, textureShader, animPos, expScale, rot0);
        else if (explosionFrame == 1) DrawTexturedModelAt(explosion2Model, textureShader, animPos, expScale, rot0);
        else if (explosionFrame == 2) DrawTexturedModelAt(explosion3Model, textureShader, animPos, expScale, rot0);
        else if (explosionFrame == 3) DrawTexturedModelAt(explosion4Model, textureShader, animPos, expScale, rot0);
        else                          DrawTexturedModelAt(explosion5Model, textureShader, animPos, expScale, rot0);
    }

    // ---- Enemigos ----
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        if (enemies[i].state == ENEMY_INACTIVE) continue;
        glm::vec3 enemyAnimPos = enemies[i].pos; enemyAnimPos.y = 0.90f;

        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);

        if (enemies[i].state == ENEMY_SPAWNING)
        {
            if (enemies[i].frame == 0) DrawTexturedModelAt(generacion1Model, textureShader, enemyAnimPos, genScale, rot0);
            else                       DrawTexturedModelAt(generacion2Model, textureShader, enemyAnimPos, genScale, rot0);
        }
        else if (enemies[i].state == ENEMY_ALIVE)
        {
            if (enemies[i].type == ENEMY_GRAY)
            {
                DrawTexturedModelAt(tanque2Model, textureShader,
                    enemies[i].pos, tanque2Scale, glm::vec3(0, enemies[i].rotY, 0));
            }
            else if (enemies[i].type == ENEMY_RED)
            {
                // El tanque rojo parpadea entre dos modelos
                if (enemies[i].flashOn)
                    DrawTexturedModelAt(tanque4Model, textureShader,
                        enemies[i].pos, tanque4Scale, glm::vec3(0, enemies[i].rotY, 0));
                else
                    DrawTexturedModelAt(tanque2Model, textureShader,
                        enemies[i].pos, tanque2Scale, glm::vec3(0, enemies[i].rotY, 0));
            }
            else // ENEMY_GREEN: cambia modelo segun HP restante
            {
                if (enemies[i].hp >= 4)  DrawTexturedModelAt(tanque3Model, textureShader, enemies[i].pos, tanque3Scale, glm::vec3(0, enemies[i].rotY, 0));
                else if (enemies[i].hp == 3)  DrawTexturedModelAt(tanquePropioModel, textureShader, enemies[i].pos, tanquePropioScale, glm::vec3(0, enemies[i].rotY, 0));
                else if (enemies[i].hp == 2)  DrawTexturedModelAt(tanque4Model, textureShader, enemies[i].pos, tanque4Scale, glm::vec3(0, enemies[i].rotY, 0));
                else                          DrawTexturedModelAt(tanque2Model, textureShader, enemies[i].pos, tanque2Scale, glm::vec3(0, enemies[i].rotY, 0));
            }
        }
        else if (enemies[i].state == ENEMY_EXPLODING)
        {
            if (enemies[i].frame == 0) DrawTexturedModelAt(explosion1Model, textureShader, enemyAnimPos, expScale, rot0);
            else if (enemies[i].frame == 1) DrawTexturedModelAt(explosion2Model, textureShader, enemyAnimPos, expScale, rot0);
            else if (enemies[i].frame == 2) DrawTexturedModelAt(explosion3Model, textureShader, enemyAnimPos, expScale, rot0);
            else if (enemies[i].frame == 3) DrawTexturedModelAt(explosion4Model, textureShader, enemyAnimPos, expScale, rot0);
            else                            DrawTexturedModelAt(explosion5Model, textureShader, enemyAnimPos, expScale, rot0);
        }
    }

    // ---- Tanque gris animado por keyframes ----
    if (grayTankVisible)
    {
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);

        glm::vec3 grayAnimPos(grayKFPosX, grayKFPosY, grayKFPosZ);
        if (graySpawnPhase)
        {
            grayAnimPos.y = 0.90f;
            if (graySpawnFrame == 0) DrawTexturedModelAt(generacion1Model, textureShader, grayAnimPos, genScale, rot0);
            else                     DrawTexturedModelAt(generacion2Model, textureShader, grayAnimPos, genScale, rot0);
        }
        else
        {
            DrawTexturedModelAt(tanque2Model, textureShader,
                grayAnimPos, tanque2Scale, glm::vec3(0, grayKFRotY, 0));
        }
    }

    // ---- Balas ----
    // Bala primaria del jugador
    if (bulletState == BULLET_MOVING)
    {
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        DrawTexturedModelAt(balaModel, textureShader, bulletPos, bulletScale, glm::vec3(0, bulletRotY, 0));
    }

    // Bala secundaria (solo con poder maximo)
    if (bullet2State == BULLET_MOVING)
    {
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        DrawTexturedModelAt(balaModel, textureShader, bullet2Pos, bulletScale, glm::vec3(0, bullet2RotY, 0));
    }

    // Balas enemigas
    for (int i = 0; i < (int)enemyBullets.size(); i++)
    {
        if (enemyBullets[i].active)
        {
            textureShader.Use();
            glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
            DrawTexturedModelAt(balaModel, textureShader,
                enemyBullets[i].pos, enemyBulletScale, glm::vec3(0, enemyBullets[i].rotY, 0));
        }
    }

    // Bala visual de la animacion por keyframes del tanque gris
    if (grayAnimBulletActive)
    {
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        DrawTexturedModelAt(balaModel, textureShader,
            grayAnimBulletPos, enemyBulletScale, glm::vec3(0, grayAnimBulletRotY, 0));
    }

    // ---- Popups de puntuacion ----
    for (int i = 0; i < (int)scorePopups.size(); i++)
    {
        if (!scorePopups[i].active || scorePopups[i].delay > 0.0f) continue;
        glm::vec3 scorePos = scorePopups[i].pos; scorePos.y = 2.10f;
        textureShader.Use();
        glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
        glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
        glm::vec3 scoreRot(0.0f, 90.0f, 0.0f);
        if (scorePopups[i].points == 100) DrawTexturedModelAt(score100Model, textureShader, scorePos, scoreScale, scoreRot);
        else if (scorePopups[i].points == 200) DrawTexturedModelAt(score200Model, textureShader, scorePos, scoreScale, scoreRot);
        else                                   DrawTexturedModelAt(score300Model, textureShader, scorePos, scoreScale, scoreRot);
    }

    // ---- Items coleccionables ----
    DrawItems(textureShader, cascoModel, relojModel, palaModel,
        estrellaModel, granadaModel, vidaModel);

    // ---- Hojas (semitransparentes, se dibujan al final para blending correcto) ----
    glDepthMask(GL_FALSE); // Sin escritura a depth para no ocluir otros objetos
    for (int z = 0; z < rows; z++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (mapa[z][x] != 'H') continue;
            float wx = (x * TILE) - ox + TILE / 2.0f;
            float wz = (z * TILE) - oz + TILE / 2.0f;
            float la = GetLeafAlphaAt(wx, wz);
            textureShader.Use();
            glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 1);
            glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), la);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
            glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
            DrawTexturedModelAt(hojasModel, textureShader, glm::vec3(wx, 0.90f, wz), hojasScale, rot0);
        }
    }
    glDepthMask(GL_TRUE);

    // Resetear flags al salir
    textureShader.Use();
    glUniform1i(glGetUniformLocation(textureShader.Program, "isLeaf"), 0);
    glUniform1f(glGetUniformLocation(textureShader.Program, "leafAlpha"), 0.0f);
    glUniform1i(glGetUniformLocation(textureShader.Program, "isWater"), 0);
    glUniform1i(glGetUniformLocation(textureShader.Program, "isShield"), 0);
}

// ============================================================
// HELPERS DE DIBUJADO
// ============================================================

// Dibuja un cubo unitario con color solido y transformacion TRS
void DrawCube(Shader& shader, GLuint VAO, GLint modelLoc,
    const glm::vec3& p, const glm::vec3& s, const glm::vec3& c)
{
    shader.Use();
    GLint cl = glGetUniformLocation(shader.Program, "objectColor");
    if (cl != -1) glUniform3f(cl, c.r, c.g, c.b);
    glm::mat4 m = glm::scale(glm::translate(glm::mat4(1), p), s);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// Dibuja un modelo 3D con color uniforme (sin textura)
void DrawModelAt(Model& model3D, Shader& shader,
    const glm::vec3& p, const glm::vec3& s, const glm::vec3& r, const glm::vec3& c)
{
    shader.Use();
    GLint cl = glGetUniformLocation(shader.Program, "modelColor");
    if (cl != -1) glUniform3f(cl, c.r, c.g, c.b);
    glm::mat4 m = glm::translate(glm::mat4(1), p);
    m = glm::rotate(m, glm::radians(r.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(r.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(r.z), glm::vec3(0, 0, 1));
    m = glm::scale(m, s);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
    model3D.Draw(shader);
}

// Dibuja un modelo 3D usando su textura original
void DrawTexturedModelAt(Model& model3D, Shader& shader,
    const glm::vec3& p, const glm::vec3& s, const glm::vec3& r)
{
    shader.Use();
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shader.Program, "texture_diffuse1"), 0);
    glm::mat4 m = glm::translate(glm::mat4(1), p);
    m = glm::rotate(m, glm::radians(r.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(r.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(r.z), glm::vec3(0, 0, 1));
    m = glm::scale(m, s);
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
    model3D.Draw(shader);
}

// Dibuja un modelo 3D para el pase de sombras (solo calcula posicion)
void DrawTexturedModelAtDepth(Model& model3D, Shader& ds,
    const glm::vec3& p, const glm::vec3& s, const glm::vec3& r)
{
    ds.Use();
    glm::mat4 m = glm::translate(glm::mat4(1), p);
    m = glm::rotate(m, glm::radians(r.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(r.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(r.z), glm::vec3(0, 0, 1));
    m = glm::scale(m, s);
    glUniformMatrix4fv(glGetUniformLocation(ds.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
    model3D.Draw(ds);
}

// ============================================================
// INICIALIZACION DEL MAPA Y JUGADOR
// ============================================================

// Busca la 'P' en el mapa y situa al jugador en esa posicion
void InitPlayerFromMap()
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;
    for (int z = 0; z < rows; z++)
        for (int x = 0; x < cols; x++)
            if (mapa[z][x] == 'P')
            {
                playerPos = glm::vec3((x * TILE) - offsetX + TILE / 2.0f, PLAYER_Y, (z * TILE) - offsetZ + TILE / 2.0f);
                playerReady = true;
                return;
            }
    // Posicion de respaldo si no hay 'P' en el mapa
    playerPos = glm::vec3(22.0f, PLAYER_Y, 23.0f);
    playerReady = true;
}

// Devuelve el vector de avance del tanque segun su rotacion actual
glm::vec3 GetTankForward()
{
    float rad = glm::radians(playerRotY);
    return glm::normalize(glm::vec3(-cos(rad), 0.0f, sin(rad)));
}

// Devuelve true si el tile es solido (bloquea movimiento)
bool IsSolidTile(char tile)
{
    return tile == 'M' || tile == 'B' || tile == 'A' || tile == 'W' ||
        tile == 'G' || tile == 'E' || tile == 'T' || tile == 'Y' || tile == 'Z';
}

// Obtiene el tile del mapa en una posicion de mundo
char GetTileFromWorld(float worldX, float worldZ)
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;
    int x = (int)floor((worldX + offsetX) / TILE);
    int z = (int)floor((worldZ + offsetZ) / TILE);
    if (x < 0 || x >= cols || z < 0 || z >= rows) return 'M'; // Fuera del mapa = metal
    return mapa[z][x];
}

// Verifica si una posicion esta bloqueada para el jugador (radio de 4 puntos)
bool IsBlockedPosition(const glm::vec3& pos)
{
    float r = playerCollisionRadius;
    return IsSolidTile(GetTileFromWorld(pos.x, pos.z)) ||
        IsSolidTile(GetTileFromWorld(pos.x, pos.z - r)) ||
        IsSolidTile(GetTileFromWorld(pos.x, pos.z + r)) ||
        IsSolidTile(GetTileFromWorld(pos.x - r, pos.z)) ||
        IsSolidTile(GetTileFromWorld(pos.x + r, pos.z));
}

// ============================================================
// DISPARO DEL JUGADOR
// ============================================================
// Dispara una bala en la direccion actual del tanque.
// Si maxPower esta activo y la bala primaria esta en uso, dispara la secundaria.
void ShootBullet()
{
    if (playerState != PLAYER_ALIVE) return;
    glm::vec3 dir = GetTankForward();

    if (bulletState == BULLET_OFF)
    {
        bulletDir = dir;
        bulletRotY = playerRotY;
        bulletPos = playerPos + bulletDir * bulletSpawnDistance;
        bulletPos.y = bulletY;
        bulletState = BULLET_MOVING;
        return;
    }

    // Segunda bala disponible solo con poder maximo
    if (maxPower && bullet2State == BULLET_OFF)
    {
        bullet2Dir = dir;
        bullet2RotY = playerRotY;
        bullet2Pos = playerPos + bullet2Dir * bulletSpawnDistance;
        bullet2Pos.y = bulletY;
        bullet2State = BULLET_MOVING;
    }
}

// Actualiza una bala del jugador: movimiento, colisiones con enemigos y tiles
void UpdateOnePlayerBullet(BulletState& state, glm::vec3& pos, glm::vec3& dir)
{
    if (state != BULLET_MOVING) return;

    pos += dir * bulletSpeed * deltaTime;

    // Colision con enemigos
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        if (enemies[i].state != ENEMY_ALIVE) continue;
        float dx = pos.x - enemies[i].pos.x;
        float dz = pos.z - enemies[i].pos.z;
        if (sqrt(dx * dx + dz * dz) <= enemyBulletHitRadius)
        {
            DamageEnemy(i);
            state = BULLET_OFF;
            return;
        }
    }

    // Colision con tiles del mapa
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;
    int x = (int)floor((pos.x + offsetX) / TILE);
    int z = (int)floor((pos.z + offsetZ) / TILE);
    if (x < 0 || x >= cols || z < 0 || z >= rows) { state = BULLET_OFF; return; }

    char tile = mapa[z][x];
    if (tile == '.' || tile == 'H' || tile == 'W') return; // Sin colision
    if (tile == 'B') { mapa[z][x] = '.';  state = BULLET_OFF; return; } // Destruye ladrillo
    if (tile == 'A')
    {
        // El adoquin necesita 2 impactos
        adoquinHP[z][x]++;
        if (adoquinHP[z][x] >= 2) { mapa[z][x] = '.'; adoquinHP[z][x] = 0; }
        state = BULLET_OFF;
        return;
    }
    if (tile == 'M')
    {
        // El metal solo se destruye con poder maximo
        if (maxPower) mapa[z][x] = '.';
        state = BULLET_OFF;
        return;
    }
    if (tile == 'G') { TriggerEagleExplosion(); state = BULLET_OFF; return; } // Destruye el aguila
    if (tile == 'C' || tile == 'R') return; // Caja/barril no bloquean la bala
    if (tile == 'T' || tile == 'Y' || tile == 'Z' || tile == 'E') { state = BULLET_OFF; return; }
}

// Llama a la actualizacion de ambas balas del jugador
void UpdateBullet()
{
    UpdateOnePlayerBullet(bulletState, bulletPos, bulletDir);
    UpdateOnePlayerBullet(bullet2State, bullet2Pos, bullet2Dir);
}

// ============================================================
// SISTEMA DE ENEMIGOS
// ============================================================

// Inicializa el sistema de enemigos: limpia el mapa de tiles de spawn
// y calcula tres puntos de aparicion en la parte superior del mapa
void InitEnemySystem()
{
    // Limpiar tiles de posicion inicial de enemigos del mapa
    for (int z = 0; z < (int)mapa.size(); z++)
        for (int x = 0; x < (int)mapa[z].size(); x++)
            if (mapa[z][x] == 'T' || mapa[z][x] == 'Y' ||
                mapa[z][x] == 'Z' || mapa[z][x] == 'E')
                mapa[z][x] = '.';

    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;
    enemySpawnPoints.clear();

    // Buscar un punto libre en cada tercio horizontal del mapa (parte superior)
    int ranges[3][2] = {
        {1,           cols / 3},
        {cols / 3,   (2 * cols) / 3},
        {(2 * cols) / 3, cols - 1}
    };
    for (int r = 0; r < 3; r++)
    {
        bool found = false;
        for (int z = 1; z < 7 && !found; z++)
            for (int x = ranges[r][0]; x < ranges[r][1]; x++)
                if (mapa[z][x] == '.' || mapa[z][x] == 'H')
                {
                    enemySpawnPoints.push_back(glm::vec3(
                        (x * TILE) - offsetX + TILE / 2.0f, PLAYER_Y,
                        (z * TILE) - offsetZ + TILE / 2.0f));
                    found = true;
                    break;
                }
    }
    // Punto de respaldo si no se encontraron posiciones
    while (enemySpawnPoints.size() < 3)
        enemySpawnPoints.push_back(glm::vec3(0.0f, PLAYER_Y, -24.0f));

    // Inicializar todos los slots de enemigos como inactivos
    for (int i = 0; i < MAX_ACTIVE_ENEMIES; i++)
    {
        enemies[i].state = ENEMY_INACTIVE;
        enemies[i].type = ENEMY_GRAY;
        enemies[i].pos = enemySpawnPoints[0];
        enemies[i].rotY = 90.0f;
        enemies[i].timer = 0.0f;
        enemies[i].turnTimer = 0.0f;
        enemies[i].frame = 0;
        enemies[i].hp = 1;
        enemies[i].shootTimer = 0.0f;
        enemies[i].flashTimer = 0.0f;
        enemies[i].flashOn = false;
    }
}

// Cuenta cuantos enemigos estan activos (spawning, alive o exploding)
int CountActiveEnemies()
{
    int c = 0;
    for (int i = 0; i < (int)enemies.size(); i++)
        if (enemies[i].state != ENEMY_INACTIVE) c++;
    return c;
}

// Elige el tipo de enemigo a spawnear segun la logica del nivel
int ChooseEnemyTypeForLevel()
{
    // Garantizar al menos un enemigo verde por nivel
    if (!greenSpawnedThisLevel && enemiesRemainingToSpawn <= 3) return ENEMY_GREEN;
    int r = rand() % 100;
    if (r < 50) return ENEMY_GRAY;
    if (r < 75) return ENEMY_RED;
    return ENEMY_GREEN;
}

// Velocidad de movimiento segun tipo de enemigo
float GetEnemySpeed(int type)
{
    if (type == ENEMY_RED)   return 3.4f; // El mas rapido
    if (type == ENEMY_GREEN) return 1.9f; // El mas lento (pero mas resistente)
    return 2.4f;                          // ENEMY_GRAY: velocidad media
}

// HP inicial segun tipo de enemigo
int GetEnemyHP(int type)
{
    if (type == ENEMY_GREEN) return 4; // Necesita 4 impactos
    return 1;                          // Gris y rojo mueren de 1 impacto
}

// Calcula el vector de avance de un enemigo desde su angulo de rotacion
glm::vec3 GetForwardFromRot(float rotY)
{
    float rad = glm::radians(rotY);
    return glm::normalize(glm::vec3(-cos(rad), 0.0f, sin(rad)));
}

// Comprueba si una posicion colisiona con algun enemigo activo
bool CollidesWithAnyEnemy(const glm::vec3& pos, int ignoreIndex)
{
    const float minDist = 1.45f;
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        if (i == ignoreIndex) continue;
        if (enemies[i].state != ENEMY_ALIVE && enemies[i].state != ENEMY_SPAWNING) continue;
        float dx = pos.x - enemies[i].pos.x, dz = pos.z - enemies[i].pos.z;
        if (sqrt(dx * dx + dz * dz) < minDist) return true;
    }
    return false;
}

// Comprueba si una posicion colisiona con el jugador
bool CollidesWithPlayer(const glm::vec3& pos)
{
    if (playerState != PLAYER_ALIVE) return false;
    float dx = pos.x - playerPos.x, dz = pos.z - playerPos.z;
    return sqrt(dx * dx + dz * dz) < 1.45f;
}

// Verifica si una posicion esta bloqueada para los enemigos
bool IsBlockedPositionEnemy(const glm::vec3& pos)
{
    float r = enemyCollisionRadius;
    return IsSolidTile(GetTileFromWorld(pos.x, pos.z)) ||
        IsSolidTile(GetTileFromWorld(pos.x, pos.z - r)) ||
        IsSolidTile(GetTileFromWorld(pos.x, pos.z + r)) ||
        IsSolidTile(GetTileFromWorld(pos.x - r, pos.z)) ||
        IsSolidTile(GetTileFromWorld(pos.x + r, pos.z));
}

// Aplica danio a un enemigo; si muere, activa explosion y suma puntos
void DamageEnemy(int index)
{
    if (index < 0 || index >= (int)enemies.size()) return;
    if (enemies[index].state != ENEMY_ALIVE) return;

    enemies[index].hp--;
    if (enemies[index].hp <= 0)
    {
        int points = EnemyPoints(enemies[index].type);
        score += points;
        AddScorePopup(enemies[index].pos, points);

        if (enemies[index].type == ENEMY_GRAY) grayDestroyed++;
        else if (enemies[index].type == ENEMY_RED) { redDestroyed++;  SpawnRandomItemAt(enemies[index].pos); }
        else                                         greenDestroyed++;

        enemies[index].state = ENEMY_EXPLODING;
        enemies[index].timer = 0.0f;
        enemies[index].frame = 0;
        enemiesDestroyed++;
    }
}

// Actualiza toda la logica de enemigos: spawn, movimiento, disparo
void UpdateEnemies()
{
    if (relojActive) return; // El reloj congela a los enemigos

    // Intentar spawnear un nuevo enemigo si hay cupo
    enemySpawnTimer += deltaTime;
    if (enemySpawnTimer >= enemySpawnInterval &&
        enemiesRemainingToSpawn > 0 &&
        CountActiveEnemies() < MAX_ACTIVE_ENEMIES)
    {
        enemySpawnTimer = 0.0f;
        for (int i = 0; i < (int)enemies.size(); i++)
        {
            if (enemies[i].state != ENEMY_INACTIVE) continue;

            int type = ChooseEnemyTypeForLevel();
            if (type == ENEMY_GREEN) greenSpawnedThisLevel = true;

            // Elegir punto de spawn sin colision
            int spawnIndex = rand() % (int)enemySpawnPoints.size();
            glm::vec3 spawnPos = enemySpawnPoints[spawnIndex];
            for (int tries = 0; tries < 8; tries++)
            {
                int ti = rand() % (int)enemySpawnPoints.size();
                glm::vec3 tp = enemySpawnPoints[ti];
                if (!CollidesWithAnyEnemy(tp, i) && !CollidesWithPlayer(tp))
                {
                    spawnIndex = ti;
                    spawnPos = tp;
                    break;
                }
            }

            enemies[i].type = (EnemyType)type;
            enemies[i].state = ENEMY_SPAWNING;
            enemies[i].pos = spawnPos;
            enemies[i].rotY = 90.0f;
            enemies[i].timer = 0.0f;
            enemies[i].turnTimer = 0.0f;
            enemies[i].frame = 0;
            enemies[i].hp = GetEnemyHP(type);
            enemies[i].shootTimer = 0.0f;
            enemies[i].flashTimer = 0.0f;
            enemies[i].flashOn = false;
            enemiesRemainingToSpawn--;
            break;
        }
    }

    // Actualizar cada enemigo individualmente
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        if (enemies[i].state == ENEMY_INACTIVE) continue;

        if (enemies[i].state == ENEMY_SPAWNING)
        {
            // Avanzar frame de aparicion
            enemies[i].timer += deltaTime;
            if (enemies[i].timer >= spawnFrameTime)
            {
                enemies[i].timer = 0.0f;
                enemies[i].frame++;
                if (enemies[i].frame > 1)
                {
                    enemies[i].frame = 0;
                    enemies[i].state = ENEMY_ALIVE;
                }
            }
        }
        else if (enemies[i].state == ENEMY_ALIVE)
        {
            enemies[i].turnTimer += deltaTime;

            // Parpadeo del tanque rojo
            if (enemies[i].type == ENEMY_RED)
            {
                enemies[i].flashTimer += deltaTime;
                if (enemies[i].flashTimer >= 0.25f)
                {
                    enemies[i].flashTimer = 0.0f;
                    enemies[i].flashOn = !enemies[i].flashOn;
                }
            }

            // Movimiento hacia adelante; girar si hay obstaculo
            glm::vec3 fwd = GetForwardFromRot(enemies[i].rotY);
            glm::vec3 newPos = enemies[i].pos + fwd * GetEnemySpeed(enemies[i].type) * deltaTime;
            newPos.y = PLAYER_Y;

            if (!IsBlockedPositionEnemy(newPos) &&
                !CollidesWithAnyEnemy(newPos, i) &&
                !CollidesWithPlayer(newPos))
                enemies[i].pos = newPos;
            else
            {
                // Giro aleatorio de 90 grados al chocar
                enemies[i].rotY = (rand() % 4) * 90.0f;
                enemies[i].turnTimer = 0.0f;
            }

            // Colision directa con el jugador = explosion del jugador
            if (playerState == PLAYER_ALIVE)
            {
                float dx = enemies[i].pos.x - playerPos.x;
                float dz = enemies[i].pos.z - playerPos.z;
                if (sqrt(dx * dx + dz * dz) <= 1.25f) TriggerPlayerExplosion();
            }

            // Disparar periodicamente
            enemies[i].shootTimer += deltaTime;
            float fireRate = 2.8f;
            if (enemies[i].type == ENEMY_RED)   fireRate = 2.2f;
            if (enemies[i].type == ENEMY_GREEN)  fireRate = 2.5f;
            if (enemies[i].shootTimer >= fireRate)
            {
                EnemyShoot(i);
                enemies[i].shootTimer = 0.0f;
            }

            // Giro aleatorio periodico
            if (enemies[i].turnTimer >= 3.0f)
            {
                enemies[i].rotY = (rand() % 4) * 90.0f;
                enemies[i].turnTimer = 0.0f;
            }
        }
        else if (enemies[i].state == ENEMY_EXPLODING)
        {
            // Avanzar frame de explosion
            enemies[i].timer += deltaTime;
            if (enemies[i].timer >= explosionFrameTime)
            {
                enemies[i].timer = 0.0f;
                enemies[i].frame++;
                if (enemies[i].frame > 4)
                {
                    enemies[i].state = ENEMY_INACTIVE;
                    enemies[i].frame = 0;
                }
            }
        }
    }

    // Condicion de victoria: todos los enemigos eliminados
    if (enemiesRemainingToSpawn <= 0 && CountActiveEnemies() == 0)
    {
        gameScreen = SCREEN_WIN;
        bulletState = BULLET_OFF;
        bullet2State = BULLET_OFF;
        for (int i = 0; i < (int)enemyBullets.size(); i++) enemyBullets[i].active = false;
    }
}

// Puntos otorgados al destruir cada tipo de enemigo
int EnemyPoints(int type)
{
    if (type == ENEMY_RED)   return 200;
    if (type == ENEMY_GREEN) return 300;
    return 100; // ENEMY_GRAY
}

// Agrega un popup de puntuacion en la posicion del enemigo destruido
void AddScorePopup(const glm::vec3& pos, int points)
{
    // Buscar slot libre; si no hay, sobreescribir el primero
    for (int i = 0; i < (int)scorePopups.size(); i++)
        if (!scorePopups[i].active)
        {
            scorePopups[i] = { true, pos, points, 0.0f, 0.75f };
            return;
        }
    scorePopups[0] = { true, pos, points, 0.0f, 0.75f };
}

// Activa la explosion del jugador (pierde una vida)
void TriggerPlayerExplosion()
{
    if (cascoActive || playerState != PLAYER_ALIVE) return;
    playerLives--;
    playerState = PLAYER_EXPLODING;
    playerAnimTimer = 0.0f;
    explosionFrame = 0;
    bulletState = BULLET_OFF;
    bullet2State = BULLET_OFF;
    for (int i = 0; i < (int)enemyBullets.size(); i++) enemyBullets[i].active = false;
}

// Activa la explosion del aguila (derrota inmediata)
void TriggerEagleExplosion()
{
    if (!eagleAlive || eagleExploding) return;
    eagleAlive = false;
    eagleExploding = true;
    eagleExplosionTimer = 0.0f;
    eagleExplosionFrame = 0;
    bulletState = BULLET_OFF;
    bullet2State = BULLET_OFF;
    for (int i = 0; i < (int)enemyBullets.size(); i++) enemyBullets[i].active = false;
}

// ============================================================
// ANIMACION POR KEYFRAMES: TANQUE GRIS ESPECIAL
// ============================================================

// Obtiene la posicion real del aguila desde el mapa.
glm::vec3 GetEaglePositionFromMap()
{
    int rows = (int)mapa.size();
    int cols = (int)mapa[0].size();
    float ox = (cols * TILE) / 2.0f;
    float oz = (rows * TILE) / 2.0f;

    for (int z = 0; z < rows; z++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (mapa[z][x] == 'G')
            {
                float wx = (x * TILE) - ox + TILE / 2.0f;
                float wz = (z * TILE) - oz + TILE / 2.0f;
                return glm::vec3(wx, 0.19f, wz);
            }
        }
    }

    return eaglePos;
}

void SaveGrayTankFrame(float x, float y, float z, float rotY)
{
    if (grayFrameIndex >= MAX_GRAY_FRAMES) return;

    GrayKeyFrame[grayFrameIndex].posX = x;
    GrayKeyFrame[grayFrameIndex].posY = y;
    GrayKeyFrame[grayFrameIndex].posZ = z;
    GrayKeyFrame[grayFrameIndex].rotY = rotY;

    GrayKeyFrame[grayFrameIndex].incX = 0.0f;
    GrayKeyFrame[grayFrameIndex].incY = 0.0f;
    GrayKeyFrame[grayFrameIndex].incZ = 0.0f;
    GrayKeyFrame[grayFrameIndex].rotYInc = 0.0f;

    grayFrameIndex++;
}

void ResetGrayTankElements()
{
    if (grayFrameIndex <= 0) return;

    grayKFPosX = GrayKeyFrame[0].posX;
    grayKFPosY = GrayKeyFrame[0].posY;
    grayKFPosZ = GrayKeyFrame[0].posZ;
    grayKFRotY = GrayKeyFrame[0].rotY;
}

void GrayTankInterpolation()
{
    if (grayPlayIndex < 0 || grayPlayIndex >= grayFrameIndex - 1) return;

    GrayKeyFrame[grayPlayIndex].incX =
        (GrayKeyFrame[grayPlayIndex + 1].posX - GrayKeyFrame[grayPlayIndex].posX) / gray_i_max_steps;
    GrayKeyFrame[grayPlayIndex].incY =
        (GrayKeyFrame[grayPlayIndex + 1].posY - GrayKeyFrame[grayPlayIndex].posY) / gray_i_max_steps;
    GrayKeyFrame[grayPlayIndex].incZ =
        (GrayKeyFrame[grayPlayIndex + 1].posZ - GrayKeyFrame[grayPlayIndex].posZ) / gray_i_max_steps;
    GrayKeyFrame[grayPlayIndex].rotYInc =
        (GrayKeyFrame[grayPlayIndex + 1].rotY - GrayKeyFrame[grayPlayIndex].rotY) / gray_i_max_steps;
}

void ResetGrayTankKeyFrameAnimation()
{
    grayPlay = false;
    grayTankVisible = false;
    grayShootingPhase = false;
    graySpawnPhase = false;
    grayFrameIndex = 0;
    grayPlayIndex = 0;
    gray_i_curr_steps = 0;
    grayShotsDone = 0;
    grayShotTimer = 0.0f;
    graySpawnTimer = 0.0f;
    graySpawnTotalTimer = 0.0f;
    graySpawnFrame = 0;
    grayKFPosX = 0.0f;
    grayKFPosY = PLAYER_Y;
    grayKFPosZ = 0.0f;
    grayKFRotY = 180.0f;
    grayAnimBulletActive = false;
    grayAnimBulletTimer = 0.0f;
    grayAnimBulletPos = glm::vec3(0.0f);
    grayAnimBulletDir = glm::vec3(0.0f);
    grayAnimBulletRotY = 0.0f;
}

void InitGrayTankKeyFrames()
{
    grayFrameIndex = 0;

    glm::vec3 p0 = TileToWorld(16, 11);
    glm::vec3 p1 = TileToWorld(22, 11);
    glm::vec3 p2 = TileToWorld(27, 11);
    glm::vec3 p3 = TileToWorld(27, 18);
    glm::vec3 p4 = TileToWorld(27, 23);

    p0.y = PLAYER_Y;
    p1.y = PLAYER_Y;
    p2.y = PLAYER_Y;
    p3.y = PLAYER_Y;
    p4.y = PLAYER_Y;

    SaveGrayTankFrame(p0.x, p0.y, p0.z, 180.0f);
    SaveGrayTankFrame(p1.x, p1.y, p1.z, 180.0f);
    SaveGrayTankFrame(p2.x, p2.y, p2.z, 90.0f);
    SaveGrayTankFrame(p3.x, p3.y, p3.z, 90.0f);
    SaveGrayTankFrame(p4.x, p4.y, p4.z, 90.0f);
}

void PlayGrayTankKeyFrameAnimation()
{
    InitGrayTankKeyFrames();

    if (grayFrameIndex > 1)
    {
        ResetGrayTankElements();
        grayPlayIndex = 0;
        gray_i_curr_steps = 0;
        GrayTankInterpolation();

        grayTankVisible = true;
        graySpawnPhase = true;
        grayPlay = false;
        grayShootingPhase = false;
        graySpawnTimer = 0.0f;
        graySpawnTotalTimer = 0.0f;
        graySpawnFrame = 0;
        grayShotsDone = 0;
        grayShotTimer = 0.0f;

        std::cout << "[KEYFRAMES] Tanque gris generado. Recorre pasillo derecho y dispara dos veces.\n";
    }
}

void GrayTankShootAtEagle()
{
    // Aunque se conserva el nombre para no mover prototipos,
    // ahora el disparo es solo visual: dura 1 segundo y no destruye la base.
    glm::vec3 currentPos(grayKFPosX, grayKFPosY, grayKFPosZ);
    glm::vec3 dir = GetForwardFromRot(grayKFRotY);

    grayAnimBulletActive = true;
    grayAnimBulletDir = dir;
    grayAnimBulletPos = currentPos + dir * 1.25f;
    grayAnimBulletPos.y = enemyBulletY;
    grayAnimBulletRotY = grayKFRotY;
    grayAnimBulletTimer = 0.0f;
}

void UpdateGrayTankKeyFrameAnimation()
{
    if (grayAnimBulletActive)
    {
        grayAnimBulletTimer += deltaTime;
        grayAnimBulletPos += grayAnimBulletDir * grayAnimBulletSpeed * deltaTime;
        if (grayAnimBulletTimer >= grayAnimBulletDuration)
            grayAnimBulletActive = false;
    }

    if (!grayTankVisible) return;

    if (graySpawnPhase)
    {
        graySpawnTimer += deltaTime;
        graySpawnTotalTimer += deltaTime;
        if (graySpawnTimer >= graySpawnFrameTime)
        {
            graySpawnTimer = 0.0f;
            graySpawnFrame = 1 - graySpawnFrame;
        }

        if (graySpawnTotalTimer >= graySpawnDuration)
        {
            graySpawnPhase = false;
            grayPlay = true;
            grayPlayIndex = 0;
            gray_i_curr_steps = 0;
            GrayTankInterpolation();
        }
        return;
    }

    if (grayPlay)
    {
        if (gray_i_curr_steps >= gray_i_max_steps)
        {
            grayPlayIndex++;

            if (grayPlayIndex > grayFrameIndex - 2)
            {
                grayPlay = false;
                grayShootingPhase = true;
                grayShotTimer = 0.0f;
            }
            else
            {
                gray_i_curr_steps = 0;
                GrayTankInterpolation();
            }
        }
        else
        {
            grayKFPosX += GrayKeyFrame[grayPlayIndex].incX;
            grayKFPosY += GrayKeyFrame[grayPlayIndex].incY;
            grayKFPosZ += GrayKeyFrame[grayPlayIndex].incZ;
            grayKFRotY += GrayKeyFrame[grayPlayIndex].rotYInc;
            gray_i_curr_steps++;
        }
    }

    if (grayShootingPhase)
    {
        grayShotTimer += deltaTime;

        if (grayShotTimer >= grayShotDelay && grayShotsDone < 2)
        {
            GrayTankShootAtEagle();
            grayShotsDone++;
            grayShotTimer = 0.0f;
            std::cout << "[KEYFRAMES] Tanque gris dispara bala visual (" << grayShotsDone << "/2).\n";
        }

        if (grayShotsDone >= 2)
        {
            grayShootingPhase = false;
            grayTankVisible = false;
            std::cout << "[KEYFRAMES] Animacion del tanque gris terminada.\n";
        }
    }
}

// Reinicia el juego al estado inicial completo
void ResetGame()
{
    eagleAlive = true;
    eagleExploding = false;
    eagleExplosionTimer = 0.0f;
    eagleExplosionFrame = 0;

    mapa = mapaOriginal; // Restaurar el mapa original

    itemsCollectedTotal = 0;
    cascoCollected = 0;
    relojCollected = 0;
    palaCollected = 0;
    estrellaCollected = 0;
    granadaCollected = 0;
    vidaCollected = 0;

    playerState = PLAYER_OFF;
    playerLives = PLAYER_MAX_LIVES;
    playerAnimTimer = 0.0f;
    spawnFrame = 0;
    explosionFrame = 0;
    bulletState = BULLET_OFF;
    bullet2State = BULLET_OFF;
    InitPlayerFromMap();

    for (int i = 0; i < (int)enemyBullets.size(); i++) enemyBullets[i].active = false;
    for (int i = 0; i < (int)scorePopups.size(); i++) scorePopups[i].active = false;

    currentLevel = 1;
    enemiesRemainingToSpawn = 10;
    greenSpawnedThisLevel = false;
    enemiesDestroyed = 0;
    enemySpawnTimer = 0.0f;
    grayDestroyed = 0;
    redDestroyed = 0;
    greenDestroyed = 0;
    score = 0;

    cascoActive = false; cascoTimer = 0.0f;
    relojActive = false; relojTimer = 0.0f;
    palaActive = false; palaTimer = 0.0f; palaConvertedTiles.clear();
    maxPower = false; maxPowerTimer = 0.0f; bulletSpeed = BULLET_SPEED_NORMAL;

    ResetGrayTankKeyFrameAnimation();

    InitEnemySystem();
    InitItems();
}

// Genera una bala enemiga en la direccion actual del enemigo
void EnemyShoot(int index)
{
    if (index < 0 || index >= (int)enemies.size()) return;
    if (enemies[index].state != ENEMY_ALIVE) return;

    // Buscar slot libre para la bala
    for (int i = 0; i < (int)enemyBullets.size(); i++)
        if (!enemyBullets[i].active)
        {
            enemyBullets[i].active = true;
            enemyBullets[i].rotY = enemies[index].rotY;
            enemyBullets[i].dir = GetForwardFromRot(enemies[index].rotY);
            enemyBullets[i].pos = enemies[index].pos + enemyBullets[i].dir * 1.15f;
            enemyBullets[i].pos.y = enemyBulletY;
            return;
        }
}

// Actualiza todas las balas enemigas activas
void UpdateEnemyBullets()
{
    for (int i = 0; i < (int)enemyBullets.size(); i++)
    {
        if (!enemyBullets[i].active) continue;

        enemyBullets[i].pos += enemyBullets[i].dir * enemyBulletSpeed * deltaTime;

        // Colision con el jugador
        if (playerState == PLAYER_ALIVE)
        {
            float dx = enemyBullets[i].pos.x - playerPos.x;
            float dz = enemyBullets[i].pos.z - playerPos.z;
            if (sqrt(dx * dx + dz * dz) <= playerHitRadius)
            {
                enemyBullets[i].active = false;
                TriggerPlayerExplosion();
                continue;
            }
        }

        // Colision con tiles del mapa
        int rows = (int)mapa.size(), cols = (int)mapa[0].size();
        float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;
        int x = (int)floor((enemyBullets[i].pos.x + offsetX) / TILE);
        int z = (int)floor((enemyBullets[i].pos.z + offsetZ) / TILE);
        if (x < 0 || x >= cols || z < 0 || z >= rows) { enemyBullets[i].active = false; continue; }

        char tile = mapa[z][x];
        if (tile == '.' || tile == 'H' || tile == 'W') continue;                                   // Sin colision
        if (tile == 'B') { mapa[z][x] = '.'; enemyBullets[i].active = false; continue; }           // Destruye ladrillo
        if (tile == 'A')
        {
            adoquinHP[z][x]++;
            if (adoquinHP[z][x] >= 2) { mapa[z][x] = '.'; adoquinHP[z][x] = 0; }
            enemyBullets[i].active = false;
            continue;
        }
        if (tile == 'G') { TriggerEagleExplosion(); enemyBullets[i].active = false; continue; }    // Destruye el aguila
        if (tile == 'M') { enemyBullets[i].active = false; continue; }                             // Metal detiene la bala
        if (tile == 'C' || tile == 'R') continue;                                                  // Caja/barril no detienen balas enemigas
    }
}

// ============================================================
// MOVIMIENTO DEL JUGADOR Y CAMARA LIBRE
// ============================================================
void DoMovement()
{
    if (playerState == PLAYER_ALIVE)
    {
        // Rotacion del tanque con flechas izquierda/derecha
        if (keys[GLFW_KEY_LEFT])  playerRotY += playerTurnSpeed * deltaTime;
        if (keys[GLFW_KEY_RIGHT]) playerRotY -= playerTurnSpeed * deltaTime;

        // Movimiento adelante/atras con flechas
        glm::vec3 fwd = GetTankForward();
        glm::vec3 newPos = playerPos;
        if (keys[GLFW_KEY_UP])   newPos += fwd * playerMoveSpeed * deltaTime;
        if (keys[GLFW_KEY_DOWN]) newPos -= fwd * playerMoveSpeed * deltaTime;
        newPos.y = PLAYER_Y;
        if (!IsBlockedPosition(newPos)) playerPos = newPos;
    }

    // Movimiento de camara libre con ESDF
    if (cameraMode == CAMERA_FREE)
    {
        if (keys[GLFW_KEY_E]) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (keys[GLFW_KEY_D]) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (keys[GLFW_KEY_S]) camera.ProcessKeyboard(LEFT, deltaTime);
        if (keys[GLFW_KEY_F]) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

// ============================================================
// ANIMACION (popups, explosiones, spawn)
// ============================================================
void Animation()
{
    // Actualizar timers de popups de puntuacion
    for (int i = 0; i < (int)scorePopups.size(); i++)
    {
        if (!scorePopups[i].active) continue;
        if (scorePopups[i].delay > 0.0f)
        {
            scorePopups[i].delay -= deltaTime;
            if (scorePopups[i].delay < 0.0f) scorePopups[i].delay = 0.0f;
        }
        else
        {
            scorePopups[i].timer += deltaTime;
            if (scorePopups[i].timer >= 2.5f) scorePopups[i].active = false;
        }
    }

    // Explosion del aguila -> derrota al terminar la animacion
    if (eagleExploding)
    {
        eagleExplosionTimer += deltaTime;
        if (eagleExplosionTimer >= explosionFrameTime)
        {
            eagleExplosionTimer = 0.0f;
            eagleExplosionFrame++;
            if (eagleExplosionFrame > 4) { gameScreen = SCREEN_LOSE; return; }
        }
    }

    // Animacion de aparicion del jugador
    if (playerState == PLAYER_SPAWNING)
    {
        playerAnimTimer += deltaTime;
        if (playerAnimTimer >= spawnFrameTime)
        {
            playerAnimTimer = 0.0f;
            spawnFrame++;
            if (spawnFrame > 1) { spawnFrame = 0; playerState = PLAYER_ALIVE; }
        }
    }
    // Animacion de explosion del jugador
    else if (playerState == PLAYER_EXPLODING)
    {
        playerAnimTimer += deltaTime;
        if (playerAnimTimer >= explosionFrameTime)
        {
            playerAnimTimer = 0.0f;
            explosionFrame++;
            if (explosionFrame > 4)
            {
                explosionFrame = 0;
                if (playerLives <= 0) { gameScreen = SCREEN_LOSE; return; }
                playerState = PLAYER_OFF; // Espera a que el jugador presione espacio
            }
        }
    }

    // Animacion especial por keyframes del tanque gris
    UpdateGrayTankKeyFrameAnimation();
}

// ============================================================
// PODERES ESPECIALES
// ============================================================

// Activa el poder maximo: bala rapida, doble disparo y rompe metal
void ActivateMaxPower()
{
    maxPower = true;
    maxPowerTimer = MAX_POWER_DURATION;
    bulletSpeed = BULLET_SPEED_MAX_POWER;
    std::cout << "[PODER] Tanque al maximo por 30s: bala rapida, doble disparo y rompe metal.\n";
}

// Destruye instantaneamente a todos los enemigos activos (efecto de granada)
void DestroyEnemiesWithGrenade()
{
    for (int i = 0; i < (int)enemies.size(); i++)
    {
        if (enemies[i].state != ENEMY_ALIVE && enemies[i].state != ENEMY_SPAWNING) continue;

        int points = EnemyPoints(enemies[i].type);
        score += points;
        AddScorePopup(enemies[i].pos, points);

        if (enemies[i].type == ENEMY_GRAY)  grayDestroyed++;
        else if (enemies[i].type == ENEMY_RED)   redDestroyed++;
        else if (enemies[i].type == ENEMY_GREEN) greenDestroyed++;

        enemiesDestroyed++;
        enemies[i].state = ENEMY_EXPLODING;
        enemies[i].timer = 0.0f;
        enemies[i].frame = 0;
    }
}

// Busca una posicion libre aleatoria en el mapa para generar un item
glm::vec3 GetRandomFreeItemPosition()
{
    int rows = (int)mapa.size(), cols = (int)mapa[0].size();
    float offsetX = (cols * TILE) / 2.0f, offsetZ = (rows * TILE) / 2.0f;

    for (int tries = 0; tries < 300; tries++)
    {
        int x = 1 + rand() % (cols - 2);
        int z = 1 + rand() % (rows - 2);
        if (mapa[z][x] == '.' || mapa[z][x] == 'H')
        {
            float worldX = (x * TILE) - offsetX + TILE / 2.0f;
            float worldZ = (z * TILE) - offsetZ + TILE / 2.0f;
            return glm::vec3(worldX, 1.4f, worldZ);
        }
    }
    // Respaldo: cerca del jugador
    return playerPos + glm::vec3(2.0f, 1.4f, 0.0f);
}

// Genera los 6 items al activar el codigo Konami y activa el poder maximo
void SpawnKonamiItems()
{
    for (int t = 0; t < 6; t++)
    {
        Item it;
        it.type = (ItemType)t;
        it.worldPos = GetRandomFreeItemPosition();
        it.active = true;
        it.bobTimer = 0.0f;
        items.push_back(it);
    }
    ActivateMaxPower();
    std::cout << "[KONAMI] Codigo correcto: 6 items generados y poder maximo por 30s.\n";
}

// Procesa la entrada del codigo Konami en tiempo real durante el juego
void ProcessKonamiCode(int key)
{
    if (key == KONAMI_SEQUENCE[konamiIndex])
    {
        konamiIndex++;
        std::cout << "[KONAMI] Progreso: " << konamiIndex << "/10\n";
        if (konamiIndex >= 10)
        {
            konamiIndex = 0;
            SpawnKonamiItems();
        }
    }
    else
    {
        // Si falla pero la tecla es el inicio de la secuencia, reiniciar desde 1
        konamiIndex = (key == KONAMI_SEQUENCE[0]) ? 1 : 0;
        if (konamiIndex == 1) std::cout << "[KONAMI] Progreso: 1/10\n";
    }
}

// ============================================================
// CALLBACKS DE ENTRADA
// ============================================================
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // ESC: salir del juego o volver al menu
    if (GLFW_KEY_ESCAPE == key && action == GLFW_PRESS)
    {
        if (gameScreen == SCREEN_PLAYING)
        {
            gameScreen = SCREEN_LOSE;
            bulletState = BULLET_OFF;
            bullet2State = BULLET_OFF;
            for (int i = 0; i < (int)enemyBullets.size(); i++) enemyBullets[i].active = false;
        }
        else
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        return;
    }

    // Registrar estado de teclas para movimiento continuo
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)   keys[key] = true;
        else if (action == GLFW_RELEASE) keys[key] = false;
    }

    // Codigo secreto (solo desde la pantalla de inicio)
    if (action == GLFW_PRESS && gameScreen == SCREEN_START)
        ProcessSecretCode(key);

    // ESPACIO: iniciar juego desde menu
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && gameScreen == SCREEN_START)
    {
        StartGameFromMenu();
        return;
    }

    // ESPACIO: reiniciar desde victoria o derrota
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS &&
        (gameScreen == SCREEN_WIN || gameScreen == SCREEN_LOSE))
    {
        ResetGame();
        StartGameFromMenu();
        return;
    }

    // R: regresar al menu desde pantalla secreta
    if (key == GLFW_KEY_R && action == GLFW_PRESS && gameScreen == SCREEN_SECRET)
    {
        secretBuffer.clear();
        gameScreen = SCREEN_START;
        return;
    }

    // Procesar Konami durante el juego
    if (action == GLFW_PRESS && gameScreen == SCREEN_PLAYING)
        ProcessKonamiCode(key);

    // ESPACIO: re-spawnear jugador despues de morir (si quedan vidas)
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS &&
        gameScreen == SCREEN_PLAYING && playerState == PLAYER_OFF)
    {
        playerState = PLAYER_SPAWNING;
        playerAnimTimer = 0.0f;
        spawnFrame = 0;
        explosionFrame = 0;
        playerRotY = 180.0f;
        bulletState = BULLET_OFF;
        bullet2State = BULLET_OFF;
        InitPlayerFromMap();
    }

    // A: disparar
    if (key == GLFW_KEY_A && action == GLFW_PRESS && gameScreen == SCREEN_PLAYING)
        ShootBullet();

    // P: pausar/reanudar el sol
    if (key == GLFW_KEY_P && action == GLFW_PRESS) sunPaused = !sunPaused;

    // Num+/Num-: velocidad del sol
    if (key == GLFW_KEY_KP_ADD && action == GLFW_PRESS) sunSpeed = glm::min(sunSpeed + 0.02f, 1.0f);
    if (key == GLFW_KEY_KP_SUBTRACT && action == GLFW_PRESS) sunSpeed = glm::max(sunSpeed - 0.02f, 0.0f);

    // C: alternar modo de camara
    if (key == GLFW_KEY_C && action == GLFW_PRESS) ToggleCameraMode();

    // K: reproducir animacion especial por keyframes del tanque gris
    if (key == GLFW_KEY_K && action == GLFW_PRESS && gameScreen == SCREEN_PLAYING)
        PlayGrayTankKeyFrameAnimation();

    // M: mostrar menu de controles en consola
    if (key == GLFW_KEY_M && action == GLFW_PRESS) PrintCameraMenu();
}

void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (cameraMode != CAMERA_FREE) return; // Solo en camara libre

    if (firstMouse) { lastX = (GLfloat)xPos; lastY = (GLfloat)yPos; firstMouse = false; }
    GLfloat xo = (GLfloat)xPos - lastX;
    GLfloat yo = lastY - (GLfloat)yPos;
    lastX = (GLfloat)xPos;
    lastY = (GLfloat)yPos;
    camera.ProcessMouseMovement(xo, yo);
}

// ============================================================
// CAMARA
// ============================================================
void PrintCameraMenu()
{
    std::cout << "\n==============================\n";
    std::cout << "        MENU DE CAMARA        \n";
    std::cout << "==============================\n";
    std::cout << "C  -> Alternar camara\n";
    std::cout << "M  -> Mostrar este menu\n";
    std::cout << "ESDF + Mouse -> Camara libre\n";
    std::cout << "Flechas -> Mover tanque\n";
    std::cout << "A -> Disparar\n";
    std::cout << "SPACE -> Iniciar / Respawnear\n";
    std::cout << "P -> Pausar sol  |  Num+/- -> Velocidad sol\n";
    std::cout << "Modo actual: ";
    if (cameraMode == CAMERA_FREE)         std::cout << "CAMARA LIBRE\n";
    else if (cameraMode == CAMERA_FOLLOW)       std::cout << "TERCERA PERSONA\n";
    else                                        std::cout << "PRIMERA PERSONA\n";
    std::cout << "==============================\n";
}

// Cicla entre los tres modos de camara: libre -> tercera persona -> primera persona
void ToggleCameraMode()
{
    if (cameraMode == CAMERA_FREE)
    {
        cameraMode = CAMERA_FOLLOW;
        std::cout << "Camara: TERCERA PERSONA\n";
    }
    else if (cameraMode == CAMERA_FOLLOW)
    {
        cameraMode = CAMERA_FIRST_PERSON;
        std::cout << "Camara: PRIMERA PERSONA\n";
    }
    else
    {
        cameraMode = CAMERA_FREE;
        firstMouse = true; // Resetear para evitar salto de camara al volver
        std::cout << "Camara: LIBRE\n";
    }
}

void DrawItemHUD(Shader& shader, GLuint VAO, GLint modelLoc)
{
    glDisable(GL_DEPTH_TEST);

    // Contador general de items
    DrawRetroText(shader, VAO, modelLoc,
        "ITEMS: " + std::to_string(itemsCollectedTotal),
        25.0f, 25.0f, 4.0f,
        glm::vec3(0.95f, 0.95f, 0.95f));

    // Vidas del jugador
    DrawRetroText(shader, VAO, modelLoc,
        "VIDAS: " + std::to_string(playerLives),
        25.0f, 55.0f, 4.0f,
        glm::vec3(0.95f, 0.95f, 0.95f));

    float y = 85.0f;

    // Cronometro del casco
    if (cascoActive)
    {
        DrawRetroText(shader, VAO, modelLoc,
            "CASCO: " + std::to_string((int)ceil(cascoTimer)) + "s",
            25.0f, y, 4.0f,
            glm::vec3(0.50f, 0.90f, 1.00f));
        y += 30.0f;
    }

    // Cronometro del reloj
    if (relojActive)
    {
        DrawRetroText(shader, VAO, modelLoc,
            "RELOJ: " + std::to_string((int)ceil(relojTimer)) + "s",
            25.0f, y, 4.0f,
            glm::vec3(0.95f, 0.95f, 0.35f));
        y += 30.0f;
    }

    // Cronometro de la pala
    if (palaActive)
    {
        DrawRetroText(shader, VAO, modelLoc,
            "PALA: " + std::to_string((int)ceil(palaTimer)) + "s",
            25.0f, y, 4.0f,
            glm::vec3(0.70f, 0.70f, 0.70f));
        y += 30.0f;
    }

    // Cronometro de la estrella
    if (maxPower)
    {
        DrawRetroText(shader, VAO, modelLoc,
            "ESTRELLA: " + std::to_string((int)ceil(maxPowerTimer)) + "s",
            25.0f, y, 4.0f,
            glm::vec3(1.00f, 0.85f, 0.20f));
        y += 30.0f;
    }

    glEnable(GL_DEPTH_TEST);
}

// Calcula la posicion de la camara en modo tercera persona (detras del tanque)
glm::vec3 GetFollowCameraPosition()
{
    return playerPos
        - GetTankForward() * followCameraDistance
        + glm::vec3(0.0f, followCameraHeight, 0.0f);
}

// Devuelve la matriz de vista segun el modo de camara activo
glm::mat4 GetCurrentViewMatrix()
{
    if (cameraMode == CAMERA_FOLLOW)
        return glm::lookAt(GetFollowCameraPosition(), playerPos + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

    if (cameraMode == CAMERA_FIRST_PERSON)
    {
        glm::vec3 forward = GetTankForward();
        glm::vec3 camPos = playerPos + glm::vec3(0.0f, firstPersonHeight, 0.0f)
            + forward * firstPersonForwardOffset;
        glm::vec3 target = camPos + forward * 5.0f;
        return glm::lookAt(camPos, target, glm::vec3(0, 1, 0));
    }

    return camera.GetViewMatrix(); // CAMERA_FREE
}

// Devuelve la posicion de la camara activa (para calcular iluminacion especular)
glm::vec3 GetCurrentCameraPosition()
{
    if (cameraMode == CAMERA_FOLLOW)
        return GetFollowCameraPosition();
    if (cameraMode == CAMERA_FIRST_PERSON)
        return playerPos + glm::vec3(0.0f, firstPersonHeight, 0.0f)
        + GetTankForward() * firstPersonForwardOffset;
    return camera.GetPosition();
}