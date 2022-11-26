#include "game.h"

std::unique_ptr<GameObject> makeEnemyShoot(World* world, glm::vec2 position) {
    std::unique_ptr<GameObject> obj = std::unique_ptr<GameObject>(new GameObject(world));
    obj->types = {GameObject::ENEMY, GameObject::GROUND};
    obj->name = "EnemyShoot";
    EnemyShoot* behavior = new EnemyShoot(obj.get());
    obj->behavior = std::unique_ptr<Behavior>(behavior);

    BoxBodyType* boxBody = new BoxBodyType();
    boxBody->scale = glm::vec2(1.0f, 0.9f);
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

    // units start in pixels here, will be converted
    struct PieceSpecs {
        glm::vec2 pos;
        glm::vec2 size;
        int pieceNum;
    };
    std::vector<PieceSpecs> pieces = {
        {glm::vec2(+7.0f, +0.0f), glm::vec2(9, 7), 0},
        {glm::vec2(+0.0f, -7.5f), glm::vec2(7, 10), 1},
        {glm::vec2(-6.5f, +2.5f), glm::vec2(8, 8), 4},
        {glm::vec2(+1.0f, +7.0f), glm::vec2(9, 9), 5},
    };

    float pixelScale = 1.0f / 16.0f;
    glm::vec2 centerOffsetPix(-0.5f, 7.5f);
    for (PieceSpecs piece : pieces) {
        std::unique_ptr<GameObject> pieceObject = std::unique_ptr<GameObject>(new GameObject(world));

        BoxBodyType* boxBody = new BoxBodyType();
        boxBody->scale = glm::vec2(piece.size.x * pixelScale, piece.size.y * pixelScale);
        pieceObject->bodyType = std::unique_ptr<BodyType>(boxBody);
        pieceObject->types = {GameObject::GROUND};
        pieceObject->name = "EnemyShootPiece";

        glm::vec2 center = position + (piece.pos + centerOffsetPix) * pixelScale;
        b2BodyDef bodyDef;
        bodyDef.position.Set(center.x, center.y);
        bodyDef.type = b2_dynamicBody;
        b2Body* body = world->box2dWorld.CreateBody(&bodyDef);
        b2PolygonShape box;

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &box;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 1.0f;

        box.SetAsBox(piece.size.x * pixelScale / 2.0f, piece.size.y * pixelScale / 2.0f);
        b2Fixture* fixture = body->CreateFixture(&fixtureDef);
        fixture->SetFriction(0.8f);
        fixture->SetDensity(1.0f);
        fixture->SetRestitution(0.2f);
        pieceObject->rigidBody = body;
        pieceObject->fixture = fixture;
        body->GetUserData().pointer = reinterpret_cast<uintptr_t>(pieceObject.get());
        b2Filter filter;
        filter.categoryBits = 1;
        filter.maskBits = 1;
        filter.groupIndex = 0;
        fixture->SetFilterData(filter);

        std::unique_ptr<EnemyShoot::Piece> actual = std::unique_ptr<EnemyShoot::Piece>(new EnemyShoot::Piece(pieceObject.get()));
        actual->mainPos = center;
        actual->size = glm::vec2(piece.size.x * pixelScale, piece.size.y * pixelScale);
        actual->pieceNum = piece.pieceNum;
        pieceObject->behavior = std::move(actual);
        behavior->pieces.push_back(std::move(pieceObject));
    }

    return obj;
}

void EnemyShoot::update(double timeStep, World* world) {
    GameObject* player = world->player.get();
    GameObject* enemy = gameObject;
    bool playerInRange = (player->rigidBody->GetPosition() - enemy->rigidBody->GetPosition()).Length() < 8;
    switch (mode) {
        case EnemyShoot::ASLEEP:
            if (playerInRange) {
                timer += timeStep;
                if (timer > 0.5f) {
                    mode = EnemyShoot::PRESHOOT;
                    timer -= 0.5f;
                    for (std::unique_ptr<GameObject>& obj : pieces) {
                        obj->rigidBody->SetGravityScale(0);
                        b2Filter filter;
                        filter.categoryBits = 0;
                        filter.maskBits = 0;
                        filter.groupIndex = -1;
                        obj->rigidBody->GetFixtureList()->SetFilterData(filter);
                    }
                }
            } else {
                timer -= timeStep;
                timer = std::max(timer, 0.0f);
            }
            break;

        case EnemyShoot::PRESHOOT:
            if (!playerInRange) {
                for (std::unique_ptr<GameObject>& obj : pieces) {
                    obj->rigidBody->SetGravityScale(1);
                }
                mode = EnemyShoot::ASLEEP;
                timer = 0.5f;
            } else {
                timer += timeStep;
                if (timer > 0.2f) {
                    mode = EnemyShoot::POSTSHOOT;
                    timer -= 0.2f;
                    Wave wave;
                    wave.center = glm::vec2(enemy->rigidBody->GetPosition().x, enemy->rigidBody->GetPosition().y);
                    wave.timer = 0.0f;
                    world->waves.push_back(wave);

                    glm::vec2 centerOffsetPix(-0.5f, 7.5f);
                    centerOffsetPix *= 1.0f / 16.0f;
                    glm::vec2 dest(gameObject->rigidBody->GetPosition().x, gameObject->rigidBody->GetPosition().y);
                    dest += centerOffsetPix;
                    for (std::unique_ptr<GameObject>& obj : pieces) {
                        glm::vec2 piecePos(obj->rigidBody->GetPosition().x, obj->rigidBody->GetPosition().y);
                        glm::vec2 dir = dest - piecePos;
                        dir = glm::normalize(dir);
                        dir *= 10.0f * -1;
                        obj->rigidBody->ApplyLinearImpulseToCenter(b2Vec2(dir.x, dir.y), true);
                        b2Filter filter;
                        filter.categoryBits = 1;
                        filter.maskBits = 1;
                        filter.groupIndex = 0;
                        obj->rigidBody->GetFixtureList()->SetFilterData(filter);
                    }
                }
            }
            break;

        case EnemyShoot::POSTSHOOT:
            timer += timeStep;
            if (timer > 1.0f) {
                mode = EnemyShoot::PRESHOOT;
                timer -= 1.0f;
            }
            break;
        default:
            timer = 0.0f;
            mode = EnemyShoot::ASLEEP;
    }

    glm::vec2 centerOffsetPix(-0.5f, 7.5f);
    centerOffsetPix *= 1.0f / 16.0f;
    if (playerInRange) {
        glm::vec2 playerPos(player->rigidBody->GetPosition().x, player->rigidBody->GetPosition().y);
        glm::vec2 dest(gameObject->rigidBody->GetPosition().x, gameObject->rigidBody->GetPosition().y);
        dest += centerOffsetPix;
        for (std::unique_ptr<GameObject>& obj : pieces) {
            glm::vec2 piecePos(obj->rigidBody->GetPosition().x, obj->rigidBody->GetPosition().y);
            glm::vec2 dir = dest - piecePos;
            dir *= 1.0f / dir.length() * 10.0f;
            obj->rigidBody->ApplyForceToCenter(b2Vec2(dir.x, dir.y), true);
        }
    }
}

