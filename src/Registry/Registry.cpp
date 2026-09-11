#include "Registry.hpp"
#include "../Components/Components.hpp"

Entity Registry::CreateEntity() { return nextEntity++; }

Entity Registry::CreateLocalPlayer() {
    Entity localPlayer = CreateEntity();
    AddComponent<TransformComponent>(localPlayer, {0.0f, 0.0f, 0.0f});
    AddComponent<VelocityComponent>(localPlayer);
    AddComponent<ColliderComponent>(localPlayer, {0.8f, 1.8f, 0.8f, false});
    AddComponent<PlayerInfoComponent>(localPlayer, {"LocalHero"});
    AddComponent<LocalPlayerTag>(localPlayer);
    AddComponent<InputComponent>(localPlayer);
    AddComponent<CameraComponent>(localPlayer);
    return localPlayer;
}

void Registry::Clear() {
    pools.clear();
    nextEntity = 1;
    CreateLocalPlayer();
}