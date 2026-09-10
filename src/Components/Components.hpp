#pragma once

#include "raylib.h"
#include <cstdint>
#include <string>

struct TransformComponent {
    float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct VelocityComponent {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    bool isGrounded = false;
    float stepOffset = 0.0f;
};

struct ColliderComponent {
    float width = 0.8f;
    float height = 1.8f;
    float depth = 0.8f;
    bool isStatic = false;
};

struct BasicRenderComponent {
    float width = 0.8f;
    float height = 1.8f;
    float depth = 0.8f;
    Color color = GRAY;
    Color wireColor = DARKGRAY;
};

struct ModelRenderComponent {
    Model model;
};

struct PlayerInfoComponent {
    std::string name;
    int health = 100;
};

struct LocalPlayerTag {};

struct InputComponent {
    float moveX = 0.0f;
    float moveZ = 0.0f;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    bool jumpRequested = false;
};

struct CameraComponent {
    float pitch = 0.0f;
    float yaw = -90.0f;
    float fov = 90.0f;
};

struct NetworkSyncComponent {
    uint32_t networkId = 0;
    float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f, targetYaw = -90.0f;
};