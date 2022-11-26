#include "game.h"
#include <iostream>

glm::mat4 toMatrix(Box box) {
    glm::mat4 matrix = glm::mat4(1.0f);
    matrix = glm::translate(matrix, glm::vec3(box.position.x, box.position.y, 0));
    matrix = glm::scale(matrix, glm::vec3(box.scale.x, box.scale.y, 0));
    return matrix;
}

void Camera::onResize(int windowWidth, int windowHeight) {
    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
    update();
}

void Camera::center(float worldX, float worldY) {
    centerPos = glm::vec2(worldX, worldY);
    update();
}

void Camera::zoom(float worldTilesPerScreen) {
    zoomMagnitude = worldTilesPerScreen;
    update();
}

void Camera::update() {
    // windowWidth, windowHeight, zoomMagnitude, centerPos
    int mainSquarePix = std::min(windowWidth, windowHeight);
    float tileSize = mainSquarePix / zoomMagnitude;
    view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(windowWidth / 2.0f, windowHeight / 2.0f, 0.0f));
    view = glm::scale(view, glm::vec3(tileSize, tileSize, 0));
    view = glm::translate(view, glm::vec3(-centerPos.x, -centerPos.y, 0));
    invView = glm::inverse(view);
}

glm::vec2 Camera::getCenter() const {
    return centerPos;
}

glm::mat4 Camera::getView() const {
    return view;
}

glm::vec2 Camera::toWorldCoordinate(glm::vec2 screenCoordinate) const {
    int mainSquarePix = std::min(windowWidth, windowHeight);
    float tileSize = mainSquarePix / zoomMagnitude;
    screenCoordinate -= glm::vec2{windowWidth / 2.0f, windowHeight / 2.0f};
    screenCoordinate /= tileSize;
    screenCoordinate += centerPos;
    return screenCoordinate;
}

glm::vec2 Camera::toScreenCoordinate(glm::vec2 screenCoordinate) const {
    int mainSquarePix = std::min(windowWidth, windowHeight);
    float tileSize = mainSquarePix / zoomMagnitude;
    screenCoordinate -= centerPos;
    screenCoordinate *= tileSize;
    screenCoordinate += glm::vec2{windowWidth / 2.0f, windowHeight / 2.0f};
    return screenCoordinate;
}

std::unique_ptr<GameObject> makePlayer(World* world, glm::vec2 position) {
    std::unique_ptr<GameObject> obj = std::unique_ptr<GameObject>(new GameObject(world));
    obj->types = {GameObject::PLAYER};
    obj->name = "Player";
    
    BoxBodyType* boxBody = new BoxBodyType();
    boxBody->scale = glm::vec2(0.7f, 1.5f);
    obj->bodyType = std::unique_ptr<BodyType>(boxBody);

    b2PolygonShape dynamicBox;
    float shearX = 0.3f, shearY = 0.03f;
    //dynamicBox.SetAsBox(boxBody->scale.x / 2.0f, boxBody->scale.y / 2.0f);
    b2Vec2 vertices[6];
    vertices[0].Set(+boxBody->scale.x / 2.0f, -boxBody->scale.y / 2.0f);
    vertices[1].Set(-boxBody->scale.x / 2.0f, -boxBody->scale.y / 2.0f);
    vertices[2].Set(-boxBody->scale.x / 2.0f, +boxBody->scale.y / 2.0f - shearY);
    vertices[3].Set(-boxBody->scale.x / 2.0f + shearX, +boxBody->scale.y / 2.0f);
    vertices[4].Set(+boxBody->scale.x / 2.0f - shearX, +boxBody->scale.y / 2.0f);
    vertices[5].Set(+boxBody->scale.x / 2.0f, +boxBody->scale.y / 2.0f - shearY);
    dynamicBox.Set(vertices, 6);

    b2BodyDef playerBodyDef;
    playerBodyDef.type = b2_dynamicBody;
    playerBodyDef.position.Set(position.x, position.y);
    playerBodyDef.fixedRotation = true;
    b2Body* playerBody = world->box2dWorld.CreateBody(&playerBodyDef);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 1.0f;
    b2Fixture* playerFixture = playerBody->CreateFixture(&fixtureDef);
    playerFixture->SetFriction(0.1f);
    playerFixture->SetRestitution(0.0f);

    obj->rigidBody = playerBody;
    obj->fixture = playerFixture;

    playerBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(obj.get());

    return obj;
}

std::unique_ptr<GameObject> makeGroundType(World* world, Box bodyDef) {
    std::unique_ptr<GameObject> obj = std::unique_ptr<GameObject>(new GameObject(world));
    obj->types = {GameObject::GROUND};
    obj->name = "Ground";
    
    BoxBodyType* boxBody = new BoxBodyType();
    boxBody->scale = bodyDef.scale;
    obj->bodyType = std::unique_ptr<BodyType>(boxBody);

    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(bodyDef.position.x, bodyDef.position.y);
    b2Body* groundBody = world->box2dWorld.CreateBody(&groundBodyDef);
    b2PolygonShape b2GroundBox;
    b2GroundBox.SetAsBox(bodyDef.scale.x / 2, bodyDef.scale.y / 2);
    b2Fixture* groundFixture = groundBody->CreateFixture(&b2GroundBox, 0.0f);
    groundFixture->SetFriction(0.8f);
    b2Filter filter;
    filter.categoryBits = 1;
    filter.groupIndex = 0;
    filter.maskBits = 0xFFFF;
    groundFixture->SetFilterData(filter);

    obj->rigidBody = groundBody;
    obj->fixture = groundFixture;

    groundBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(obj.get());

    return obj;
}

std::vector<std::unique_ptr<GameObject>> makeGround(World* world, GridPos gridPos, Grid grid) {
    glm::vec2 gridStart;
    gridStart.x = gridPos.x * GRID_SIZE;
    gridStart.y = gridPos.y * GRID_SIZE;

    std::vector<std::unique_ptr<GameObject>> groundBodies;
    for (int y = 0; y < GRID_SIZE; ++y) {
        for (int x = 0; x < GRID_SIZE; ++x) {
            if (grid.blocks[y * GRID_SIZE + x] != air) {
                groundBodies.push_back(std::move(makeGroundType(world, Box{gridStart + glm::vec2{x + 0.5f, y + 0.5f}, {1, 1}})));
            }
        }
    }
    return groundBodies;
}

GameObject::GameObject(World* world) : world(world) {
    world->gameObjects.insert(this);
}
GameObject::~GameObject() {
    rigidBody->DestroyFixture(fixture);
    world->box2dWorld.DestroyBody(rigidBody);
    world->gameObjects.erase(this);
}
void GameObject::update(double timeStep, World* world) {
    if (behavior.get() != nullptr) {
        behavior->update(timeStep, world);
    }
}

void Game::BeginContact(b2Contact* contact) {
}
 
void Game::EndContact(b2Contact* contact) {
}
 
void Game::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
}
 
void Game::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
    GameObject* objA = reinterpret_cast<GameObject*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer);
    GameObject* objB = reinterpret_cast<GameObject*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer);

    // find if ground collides with non ground
    bool groundIsA = false;
    if (objA->types.contains(GameObject::GROUND)) {
        groundIsA = true;
    }
    if (objB->types.contains(GameObject::GROUND)) {
        if (groundIsA) {
            // ground colliding with ground? i don't care
            return;
        }
    } else {
        if (!groundIsA) {
            // nonGround colliding with nonGround? i don't care
            return;
        }
    }
    
    //b2Vec2 normal[2];
    //normal[0] = contact->GetManifold()->localNormal;
    //normal[1] = contact->GetManifold()->localNormal;
    //b2Vec2 fullImpulse;
    //fullImpulse.x = 0;
    //fullImpulse.y = 0;
    //for (int i = 0; i < 2; ++i) {
    //    normal[i] *= impulse->normalImpulses[i];
    //    normal[i] *= groundIsA ? -1 : 1;
    //    fullImpulse += normal[i];
    //}

    //std::cout << "PostSolve, gorundIsA: " << groundIsA << ", impulse: " << fullImpulse.x << ", " << fullImpulse.y << ", normal: " << contact->GetManifold()->localNormal.y << std::endl;

    if (contact->GetManifold()->localNormal.y > 0.5f) {
        // ground pushed object up
        if (groundIsA) {
            objB->onGround = true;
            objB->airTime = 0;
        } else {
            objA->onGround = true;
            objA->airTime = 0;
        }
    }
}
