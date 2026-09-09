#include "Systems.hpp"
#include "../Components/Components.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <vector>

BoundingBox GetEntityAABB(const TransformComponent &transform,
                          const ColliderComponent &collider) {
    float halfW = collider.width * 0.5f;
    float halfD = collider.depth * 0.5f;
    return BoundingBox{
        Vector3{transform.x - halfW, transform.y, transform.z - halfD},
        Vector3{transform.x + halfW, transform.y + collider.height,
                transform.z + halfD}};
}

void InputSystem::Update(Registry &registry) {
    auto pool = registry.View<InputComponent>();
    for (auto &[entity, input] : pool->data) {
        input.moveX = 0.0f;
        input.moveZ = 0.0f;

        if (IsKeyDown(KEY_W))
            input.moveZ += 1.0f;
        if (IsKeyDown(KEY_S))
            input.moveZ -= 1.0f;
        if (IsKeyDown(KEY_A))
            input.moveX -= 1.0f;
        if (IsKeyDown(KEY_D))
            input.moveX += 1.0f;

        input.jumpRequested = IsKeyDown(KEY_SPACE);

        Vector2 mouseDelta = GetMouseDelta();
        input.mouseDeltaX = mouseDelta.x;
        input.mouseDeltaY = mouseDelta.y;
    }
}

void MovementSystem::Update(Registry &registry, float deltaTime) {
    const float GRAVITY = 25.0f;
    const float JUMP_FORCE = 8.5f;
    const float MAX_GROUND_SPEED = 8.0f;
    const float GROUND_ACCEL = 10.0f;
    const float FRICTION = 6.0f;
    const float STOP_SPEED = 1.5f;

    const float AIR_ACCEL = 100.0f;
    const float AIR_CAP = 1.2f;
    const float MAX_STEP_HEIGHT = 0.5f;

    struct StaticObstacle {
        Entity id;
        BoundingBox box;
    };
    std::vector<StaticObstacle> obstacles;

    auto colliders = registry.View<ColliderComponent>();
    for (auto &[e, col] : colliders->data) {
        if (col.isStatic) {
            auto t = registry.GetComponent<TransformComponent>(e);
            if (t)
                obstacles.push_back({e, GetEntityAABB(*t, col)});
        }
    }

    auto transforms = registry.View<TransformComponent>();

    for (auto &[entity, transform] : transforms->data) {
        auto vel = registry.GetComponent<VelocityComponent>(entity);
        auto col = registry.GetComponent<ColliderComponent>(entity);
        if (!vel || !col || col->isStatic)
            continue;

        auto input = registry.GetComponent<InputComponent>(entity);
        auto camera = registry.GetComponent<CameraComponent>(entity);

        Vector3 wishdir = {0.0f, 0.0f, 0.0f};
        if (input && camera) {
            float sensitivity = 0.15f;
            camera->yaw += input->mouseDeltaX * sensitivity;
            camera->pitch -= input->mouseDeltaY * sensitivity;

            if (camera->pitch > 89.0f)
                camera->pitch = 89.0f;
            if (camera->pitch < -89.0f)
                camera->pitch = -89.0f;

            float yawRad = camera->yaw * DEG2RAD;
            Vector3 forward = {cosf(yawRad), 0.0f, sinf(yawRad)};
            Vector3 right = {-sinf(yawRad), 0.0f, cosf(yawRad)};

            Vector3 moveVec = {
                forward.x * input->moveZ + right.x * input->moveX, 0.0f,
                forward.z * input->moveZ + right.z * input->moveX};

            float moveLen =
                sqrtf(moveVec.x * moveVec.x + moveVec.z * moveVec.z);
            if (moveLen > 0.0001f) {
                wishdir.x = moveVec.x / moveLen;
                wishdir.z = moveVec.z / moveLen;
            }
        }

        if (vel->isGrounded && input && input->jumpRequested) {
            vel->y = JUMP_FORCE;
            vel->isGrounded = false;
            vel->stepOffset = 0.0f;
            input->jumpRequested = false;
        } else if (vel->isGrounded) {
            float speed = sqrtf(vel->x * vel->x + vel->z * vel->z);
            if (speed > 0.0001f) {
                float control = (speed < STOP_SPEED) ? STOP_SPEED : speed;
                float drop = control * FRICTION * deltaTime;
                float newSpeed = std::max(0.0f, speed - drop) / speed;

                vel->x *= newSpeed;
                vel->z *= newSpeed;
            }
        }

        if (vel->isGrounded) {
            float currentSpeed = vel->x * wishdir.x + vel->z * wishdir.z;
            float addSpeed = MAX_GROUND_SPEED - currentSpeed;

            if (addSpeed > 0.0f) {
                float accelSpeed = GROUND_ACCEL * MAX_GROUND_SPEED * deltaTime;
                accelSpeed = std::min(accelSpeed, addSpeed);

                vel->x += accelSpeed * wishdir.x;
                vel->z += accelSpeed * wishdir.z;
            }
        } else {
            float currentSpeed = vel->x * wishdir.x + vel->z * wishdir.z;
            float addSpeed = AIR_CAP - currentSpeed;

            if (addSpeed > 0.0f) {
                float accelSpeed = AIR_ACCEL * MAX_GROUND_SPEED * deltaTime;
                accelSpeed = std::min(accelSpeed, addSpeed);

                vel->x += accelSpeed * wishdir.x;
                vel->z += accelSpeed * wishdir.z;
            }
        }

        vel->y -= GRAVITY * deltaTime;

        auto CheckCollision = [&](const TransformComponent &testTransform) {
            BoundingBox playerBox = GetEntityAABB(testTransform, *col);
            for (const auto &obs : obstacles) {
                if (CheckCollisionBoxes(playerBox, obs.box))
                    return true;
            }
            return false;
        };

        auto TryStepUp = [&](float oldY) {
            if (!vel->isGrounded)
                return false;

            const int STEP_STEPS = 16;
            const float STEP_INC = MAX_STEP_HEIGHT / (float)STEP_STEPS;

            for (int i = 1; i <= STEP_STEPS; ++i) {
                float probe = i * STEP_INC;
                TransformComponent stepTransform = transform;
                stepTransform.y = oldY + probe;

                if (!CheckCollision(stepTransform)) {
                    transform.y = oldY + probe;
                    vel->stepOffset -= probe;
                    return true;
                }
            }
            return false;
        };

        float prevY = transform.y;
        transform.x += vel->x * deltaTime;
        if (CheckCollision(transform)) {
            if (!TryStepUp(prevY)) {
                transform.x -= vel->x * deltaTime;
                vel->x = 0.0f;
            }
        }

        prevY = transform.y;
        transform.z += vel->z * deltaTime;
        if (CheckCollision(transform)) {
            if (!TryStepUp(prevY)) {
                transform.z -= vel->z * deltaTime;
                vel->z = 0.0f;
            }
        }

        transform.y += vel->y * deltaTime;
        vel->isGrounded = false;

        if (CheckCollision(transform)) {
            transform.y -= vel->y * deltaTime;

            if (vel->y < 0.0f) {
                vel->isGrounded = true;
            }
            vel->y = 0.0f;
        }

        if (transform.y <= -50.0f) {
            transform.y = 0.0f;
            transform.x = 0.0f;
            transform.z = 0.0f;
            vel->x = 0.0f;
            vel->y = 0.0f;
            vel->z = 0.0f;
            vel->isGrounded = true;
        }

        const float DECAY_SPEED = 18.0f;
        vel->stepOffset = vel->stepOffset * expf(-DECAY_SPEED * deltaTime);
        if (fabsf(vel->stepOffset) < 0.001f) {
            vel->stepOffset = 0.0f;
        }
    }
}

void CameraSystem::Update(Registry &registry) {
    auto cameras = registry.View<CameraComponent>();

    for (auto &[entity, cameraComp] : cameras->data) {
        auto transform = registry.GetComponent<TransformComponent>(entity);
        if (transform) {
            float pitchRad = cameraComp.pitch * DEG2RAD;
            float yawRad = cameraComp.yaw * DEG2RAD;

            Vector3 forward;
            forward.x = cosf(pitchRad) * cosf(yawRad);
            forward.y = sinf(pitchRad);
            forward.z = cosf(pitchRad) * sinf(yawRad);

            float visualOffset = 0.0f;
            if (auto vel = registry.GetComponent<VelocityComponent>(entity)) {
                visualOffset = vel->stepOffset;
            }

            Vector3 eyePos = {transform->x, transform->y + 1.6f + visualOffset,
                              transform->z};
            Vector3 target = Vector3Add(eyePos, forward);

            Camera3D raylibCamera = {0};
            raylibCamera.position = eyePos;
            raylibCamera.target = target;
            raylibCamera.up = (Vector3){0.0f, 1.0f, 0.0f};
            raylibCamera.fovy = cameraComp.fov;
            raylibCamera.projection = CAMERA_PERSPECTIVE;

            BeginMode3D(raylibCamera);
            DrawGrid(40, 1.0f);

            auto colliders = registry.View<ColliderComponent>();
            for (auto &[e, col] : colliders->data) {
                auto t = registry.GetComponent<TransformComponent>(e);
                if (!t)
                    continue;

                BoundingBox box = GetEntityAABB(*t, col);
                Vector3 size = Vector3Subtract(box.max, box.min);
                Vector3 center = Vector3Add(box.min, Vector3Scale(size, 0.5f));

                if (col.isStatic) {
                    Color fillColor = GRAY;
                    Color wireColor = DARKGRAY;
                    if (auto colorComp =
                            registry.GetComponent<ColorComponent>(e)) {
                        fillColor = colorComp->color;
                        wireColor = colorComp->wireColor;
                    }

                    DrawCube(center, size.x, size.y, size.z, fillColor);
                    DrawCubeWires(center, size.x, size.y, size.z, wireColor);
                } else if (e != entity) {
                    DrawCube(center, size.x, size.y, size.z, RED);
                    DrawCubeWires(center, size.x, size.y, size.z, MAROON);
                }
            }
            EndMode3D();
        }
    }
}

void NetworkSystem::Update(Registry &registry, float deltaTime) {
    auto netPool = registry.View<NetworkSyncComponent>();

    for (auto &[entity, net] : netPool->data) {
        if (registry.HasComponent<LocalPlayerTag>(entity))
            continue;

        auto transform = registry.GetComponent<TransformComponent>(entity);
        if (transform) {
            transform->x += (net.targetX - transform->x) * 5.0f * deltaTime;
            transform->z += (net.targetZ - transform->z) * 5.0f * deltaTime;
        }
    }
}
