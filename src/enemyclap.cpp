#include "game.h"

std::unique_ptr<GameObject> makeEnemyClap(World* world, glm::vec2 position) {
    std::unique_ptr<GameObject> obj = std::unique_ptr<GameObject>(new GameObject(world));
    obj->types = {GameObject::ENEMY, GameObject::GROUND};
    obj->name = "EnemyClap";
    obj->behavior = std::unique_ptr<Behavior>(new EnemyClap(obj.get()));
    
    BoxBodyType* boxBody = new BoxBodyType();
    boxBody->scale = glm::vec2(1.0f, 1.0f);
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

    b2BodyDef enemyBodyDef;
    enemyBodyDef.type = b2_dynamicBody;
    enemyBodyDef.position.Set(position.x, position.y);
    enemyBodyDef.fixedRotation = true;
    b2Body* enemyBody = world->box2dWorld.CreateBody(&enemyBodyDef);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 1.0f;
    b2Fixture* enemyFixture = enemyBody->CreateFixture(&fixtureDef);
    enemyFixture->SetFriction(1.0f);
    enemyFixture->SetRestitution(0.0f);

    obj->rigidBody = enemyBody;
    obj->fixture = enemyFixture;

    enemyBody->GetUserData().pointer = reinterpret_cast<uintptr_t>(obj.get());

    return obj;
}

void EnemyClap::update(double timeStep, World* world) {
    GameObject* player = world->player.get();
    GameObject* enemy = gameObject;
    bool playerInRange = (player->rigidBody->GetPosition() - enemy->rigidBody->GetPosition()).Length() < 8;
    switch (mode) {
        case EnemyClap::ASLEEP:
            if (playerInRange) {
                timer += timeStep;
                if (timer > 0.5f) {
                    mode = EnemyClap::AWAKE;
                    timer -= 0.5f;
                    enemy->faceRight = player->rigidBody->GetPosition().x > enemy->rigidBody->GetPosition().x;
                }
            } else {
                timer -= timeStep;
                timer = std::max(timer, 0.0f);
            }
            break;
        case EnemyClap::AWAKE:
            if (!playerInRange) {
                mode = EnemyClap::ASLEEP;
                timer = 0.5f;
            } else {
                timer += timeStep;
                if (timer > 0.2f) {
                    mode = EnemyClap::ATTACKED;
                    timer -= 0.2f;
                    Wave wave;
                    wave.center = glm::vec2(enemy->rigidBody->GetPosition().x + (!enemy->faceRight ? -0.76f : 0.76f), enemy->rigidBody->GetPosition().y - 0.65f);
                    wave.timer = 0.0f;
                    world->waves.push_back(wave);
                }
            }
            break;
        case EnemyClap::ATTACKED:
            timer += timeStep;
            if (timer > 1.0f) {
                mode = EnemyClap::AWAKE;
                timer -= 1.0f;
                enemy->faceRight = player->rigidBody->GetPosition().x > enemy->rigidBody->GetPosition().x;
            }
            break;
        default:
            timer = 0.0f;
            mode = EnemyClap::ASLEEP;
    }
}
