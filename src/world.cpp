#include "game.h"

const float MAX_HORIZONTAL_VELOCITY = 10.0f;
const float MAX_VERTICAL_VELOCITY = 20.0f;
const float PLAYER_JUMP_IMPULSE_AMOUNT = 10.0f;
const float MOVE_INTERPOLATE_DISTANCE_LIMIT = 0.1f;

World::World() {
    player = makePlayer(this, {0.0f, -5.0f});
    ground = makeGroundType(this, Box{{0.0f, 5.0f}, {20.0f, 10.0f}});
    enemy = makeEnemy(this, {5.0f, -5.0f});
}

// later replace GLFWwindow* api use with a controller abstraction of some sort
void World::update(double timeStep, GLFWwindow* window) {
    // player movement
    // options I can think of for limiting movement:
    // only applyForce when velocity in a direction is below a limit
    // applyForce backwards when velocity in a direction is too high
    // setVelocity to the limit when it is above a limit
    b2Vec2 playerMove(0.0f, 0.0f);
    bool playerJump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool playerSlowDown = false;
    playerMove.x += (glfwGetKey(window, GLFW_KEY_D) - glfwGetKey(window, GLFW_KEY_A));
    if (playerMove.LengthSquared() > 0) {
        playerMove.x *= 1.0f / playerMove.Length();
        playerMove.y *= 1.0f / playerMove.Length();
    }

    // make player slow down if not trying to move
    if (playerMove.LengthSquared() < 0.00001f && player->onGround && abs(player->rigidBody->GetLinearVelocity().x) > 0) {
        float movingDir = player->rigidBody->GetLinearVelocity().x;
        if (movingDir > 0) playerMove.x = -1;
        if (movingDir < 0) playerMove.x = +1;
        playerSlowDown = true;
    }

    playerMove *= 1000 / 60.0f;
    if (player->onGround) {
        playerMove *= 2;
    }

    if (playerJump) {
        // check player can jump
        // std::cout << "Player trying to jump, og: " << player->onGround << std::endl;

        if (player->onGround) {
            b2Vec2 playerJumpImpulse(0, -PLAYER_JUMP_IMPULSE_AMOUNT);
            // now we decide to jump
            if (playerJumpImpulse.LengthSquared() > 0.00001f) {
                player->rigidBody->ApplyLinearImpulseToCenter(playerJumpImpulse, true);

                // force the object up a bit so that it is no longer counted as being on the ground
                player->rigidBody->SetTransform(player->rigidBody->GetPosition() + b2Vec2(0, -0.1f), player->rigidBody->GetAngle());
            }
        } else {
            if (player->airTime < 0.5f) {
                playerMove.y -= 10;
            }
        }
    }

    // stop player if he slows down enough
    if (playerSlowDown && abs(playerMove.x) * timeStep > abs(player->rigidBody->GetLinearVelocity().x)) {
        playerMove.x = 0;
        player->rigidBody->SetLinearVelocity(b2Vec2(0, player->rigidBody->GetLinearVelocity().y));
    }

    b2Vec2 playerMoveForce(playerMove.x, playerMove.y);
    if (playerMoveForce.LengthSquared() > 0.00001f) {
        player->rigidBody->ApplyForce(playerMoveForce, player->rigidBody->GetPosition(), true);
    }
    b2Vec2 velocity = player->rigidBody->GetLinearVelocity();
    velocity.x = limitMagnitude(velocity.x, MAX_HORIZONTAL_VELOCITY);
    velocity.y = limitMagnitude(velocity.y, MAX_VERTICAL_VELOCITY);
    player->rigidBody->SetLinearVelocity(velocity);

    std::vector<int> wavesToDelete;
    for (int i = 0; i < (int) waves.size(); ++i) {
        Wave& wave = waves[i];
        wave.timer += timeStep;
        if (wave.timer > 1.0f) {
            wavesToDelete.push_back(i);
        }
    }
    for (int i = (int) wavesToDelete.size() - 1; i >= 0; --i) {
        int index = wavesToDelete[i];
        waves.erase(waves.begin() + index);
    }

    bool playerInRange = (player->rigidBody->GetPosition() - enemy->rigidBody->GetPosition()).Length() < 8;
    switch (enemy->mode) {
        case Enemy::ASLEEP:
            if (playerInRange) {
                enemy->timer += timeStep;
                if (enemy->timer > 0.5f) {
                    enemy->mode = Enemy::AWAKE;
                    enemy->timer -= 0.5f;
                    enemy->faceRight = player->rigidBody->GetPosition().x > enemy->rigidBody->GetPosition().x;
                }
            } else {
                enemy->timer -= timeStep;
                enemy->timer = std::max(enemy->timer, 0.0f);
            }
            break;
        case Enemy::AWAKE:
            if (!playerInRange) {
                enemy->mode = Enemy::ASLEEP;
                enemy->timer = 0.5f;
            } else {
                enemy->timer += timeStep;
                if (enemy->timer > 0.2f) {
                    enemy->mode = Enemy::ATTACKED;
                    enemy->timer -= 0.2f;
                    Wave wave;
                    wave.center = glm::vec2(enemy->rigidBody->GetPosition().x + (!enemy->faceRight ? -0.76f : 0.76f), enemy->rigidBody->GetPosition().y - 0.65f);
                    wave.timer = 0.0f;
                    waves.push_back(wave);
                }
            }
            break;
        case Enemy::ATTACKED:
            enemy->timer += timeStep;
            if (enemy->timer > 1.0f) {
                enemy->mode = Enemy::AWAKE;
                enemy->timer -= 1.0f;
                enemy->faceRight = player->rigidBody->GetPosition().x > enemy->rigidBody->GetPosition().x;
            }
            break;
        default:
            enemy->timer = 0.0f;
            enemy->mode = Enemy::ASLEEP;
    }

    for (GameObject* obj : gameObjects) {
        if (obj->rigidBody->IsAwake()) {
            obj->onGround = false;
            obj->airTime += timeStep;
        }
    }
    box2dWorld.Step(timeStep, 8, 3);
}
