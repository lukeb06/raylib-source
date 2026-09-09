#pragma once

#include "../Registry/Registry.hpp"

class InputSystem {
  public:
    void Update(Registry &registry);
};

class MovementSystem {
  public:
    void Update(Registry &registry, float deltaTime);
};

class CameraSystem {
  public:
    void Update(Registry &registry);
};

class NetworkSystem {
  public:
    void Update(Registry &registry, float deltaTime);
};
