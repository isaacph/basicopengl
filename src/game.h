#ifndef SRC_GAME_H_INCLUDED
#define SRC_GAME_H_INCLUDED
#include <iostream>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <stdexcept>
#include <string_view>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "graphics/graphics.h"
#include "graphics/simple.h"
#include "graphics/texture.h"
#include "util.h"
#include <span>
#define MY_PI 3.1415926535979323f
#include "grid.h"
#include <memory>
#include <cmath>
#include <functional>
#include "events.h"
#include "physics.h"
#include <box2d/box2d.h>
#include <set>

const float GRAV_ACCEL= 20.0f;
const float MAX_FALL= 10.0f;
const float JUMP_INIT_VELOCITY= -5.0f;
const float VERT_FRICTION= 1.0f;
const float VERT_ACCEL=2.0f;

class PlayerInstruction{
public:
    bool isJump;
    bool moveRight;
    bool moveLeft;
};
class Status{
    int hp;
    bool stunned;
    bool invunreable;
};


glm::mat4 toMatrix(Box box);

class Camera {
public:
    void onResize(int windowWidth, int windowHeight);
    void center(float worldX, float worldY);
    void zoom(float worldTilesPerScreen);
    glm::mat4 getView() const;
    glm::vec2 getCenter() const;
    glm::vec2 toWorldCoordinate(glm::vec2 screenCoordinate) const;
    glm::vec2 toScreenCoordinate(glm::vec2 worldCoordinate) const;
private:
    void update();
    int windowWidth, windowHeight;
    glm::vec2 centerPos;
    float zoomMagnitude;
    glm::mat4 view, invView;
};

struct BodyType {};
struct BoxBodyType : public BodyType {
    glm::vec2 scale;
};

class World;
class Game;
class Behavior;
class GameObject {
public:
    enum Type {
        PLAYER, GROUND, ENEMY
    };
    std::set<Type> types;
    std::string name;
    std::unique_ptr<BodyType> bodyType;
    b2Body* rigidBody;
    b2Fixture* fixture;
    GameObject(World* world);
    virtual void update(double timeStep, World* world);
    virtual ~GameObject();

    bool onGround = false;
    float airTime = 0;
    bool faceRight = true;

    // list of things this GameObject can do
    // to avoid duplication of code if multiple enemy types with different behaviors
    // can do similar things

    // this is the script that controls this GameObject
    std::unique_ptr<Behavior> behavior;
private:
    World* world;
};

class Behavior {
public:
    inline Behavior(GameObject* gameObject) : gameObject(gameObject) {}
    inline virtual ~Behavior() {}
    virtual void update(double timeStep, World* world) = 0;
    GameObject* const gameObject;
};

class EnemyClap : public Behavior {
public:
    inline EnemyClap(GameObject* gameObject) : Behavior(gameObject) {}
    virtual void update(double timeStep, World* world);
    float timer = 0.0f;
    enum Mode {
        ASLEEP, AWAKE, ATTACKED
    } mode = ASLEEP;
};

class EnemyShoot : public Behavior {
public:
    inline EnemyShoot(GameObject* gameObject) : Behavior(gameObject) {}
    virtual void update(double timeStep, World* world);
    float timer = 0.0f;
    enum Mode {
        ASLEEP, PRESHOOT, POSTSHOOT
    } mode = ASLEEP;
    struct Piece {
        std::unique_ptr<GameObject> gameObject;
        glm::vec2 mainPos;
        glm::vec2 size;
    };
    std::vector<Piece> pieces;
};

class Enemy : public GameObject {
public:
    inline Enemy(World* world) : GameObject(world) {}
    float timer = 0.0f;
    enum Mode {
        ASLEEP, AWAKE, ATTACKED
    } mode = ASLEEP;
};

struct Wave {
    glm::vec2 center;
    float timer;
};

class World {
public:
    World();
    void update(double timeStep, GLFWwindow* window);

    b2World box2dWorld = b2World(b2Vec2(0.0f, GRAV_ACCEL));
    GridManager gridManager;
    Camera camera;
    std::set<GameObject*> gameObjects;
    std::vector<Wave> waves;

    std::unique_ptr<GameObject> player;
    std::unique_ptr<GameObject> ground;
    std::unique_ptr<GameObject> enemy;
    std::unique_ptr<GameObject> enemy2;
};

std::unique_ptr<GameObject> makePlayer(World* world, glm::vec2 position);
std::unique_ptr<GameObject> makeEnemyClap(World* world, glm::vec2 position);
std::unique_ptr<GameObject> makeEnemyShoot(World* world, glm::vec2 position);
std::unique_ptr<GameObject> makeGroundType(World* world, Box bodyDef);
std::vector<std::unique_ptr<GameObject>> makeGround(World* world, GridPos gridPos, Grid grid);

class Game : public b2ContactListener {
public:
    void run();
    void move(float &x, float &y, float deltaf, float speed);
    void onResize(int width, int height);
    void onKey(int key, int scancode, int action, int mods);
    void onGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message);
private:
    GLFWwindow* window;
    int windowWidth = 0, windowHeight = 0;
    glm::mat4 proj;
    World world;
    bool foward;
    double physicsTime = 0;
    bool paused = false;
    //b2Body* groundBody;
    //b2Fixture* groundFixture;
    //b2Body* playerBody;
    //b2Fixture* playerFixture;
public:
    void BeginContact(b2Contact* contact);
     
    void EndContact(b2Contact* contact);
     
    void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
     
    void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};


class Player {
public:
    Box hitbox;
    float x_velocity;
    float y_velocity;
    inline Player() {
        hitbox.position = {0.0f, 0.0f};
        hitbox.scale = {1.0f, 1.0f};
    }
    inline void updateHitbox(float deltaF){
         hitbox.position.y+=x_velocity * deltaF;
         hitbox.position.x+=y_velocity * deltaF;
    }
    inline void updateXVelocity(float deltaV){
        x_velocity+=deltaV;
    };
    inline void updateYVelocity(float deltaV){
        y_velocity+=deltaV;
    };
    inline void jump(){
      if(numJumps>0){
          y_velocity= JUMP_INIT_VELOCITY;
          airtime*= 0.1f;
          numJumps--;
        }  
    };
    void playerMoveLeft(float deltaF);
    void playerMoveRight(float deltaF);
    float moveAccel=5;
    float airtime;
    int numJumps;
    float maxSpeed;
};

#endif
