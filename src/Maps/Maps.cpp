#include "Maps.hpp"
#include "../Components/Components.hpp"

namespace Maps {
Entity CreateBlock(Registry &reg, Vector3 pos, Vector3 size, Color color,
                   Color wireColor) {
    Entity block = reg.CreateEntity();
    reg.AddComponent<TransformComponent>(block, {pos.x, pos.y, pos.z});
    reg.AddComponent<ColliderComponent>(block, {size.x, size.y, size.z, true});
    reg.AddComponent<BasicRenderComponent>(
        block, {size.x, size.y, size.z, color, wireColor});
    return block;
}

void LoadMap1(Registry &registry) {

    CreateBlock(registry, {0.0f, -1.0f, 0.0f}, {400.0f, 1.0f, 400.0f},
                RAYWHITE);

    CreateBlock(registry, {0.0f, 0.0f, -5.0f}, {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {5.0f, 0.0f, -5.0f}, {4.0f, 1.0f, 4.0f});

    CreateBlock(registry, {-6.0f, 0.0f, -2.0f}, {1.0f, 3.0f, 8.0f});

    for (int i = 0; i < 100; i++) {
        CreateBlock(registry, {0.0f, 0.0f + i * 0.4f, 5.0f + i * 0.4f},
                    {4.0f, 0.4f, 0.4f}, LIME);
    }

    CreateBlock(registry, {0.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, 5.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, 1.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, 1.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {4.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
    CreateBlock(registry, {-4.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});

    CreateBlock(registry, {0.0f, 0.0f + 99 * 0.4f, -3.0f + 99 * 0.4f + 2.2f},
                {4.0f, 0.4f, 4.0f});
}
void LoadMap2(Registry &registry) {
    CreateBlock(registry, {0.0f, -1.0f, 0.0f}, {12.0f, 1.0f, 12.0f},
                {72, 78, 88, 255}, {30, 32, 38, 255});
    CreateBlock(registry, {2.1f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f},
                {220, 90, 80, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {3.1f, 1.1f, 0.0f}, {1.0f, 1.0f, 1.0f},
                {220, 90, 80, 255}, {90, 30, 28, 255});
    CreateBlock(registry, {-3.3f, 1.1f, 2.1f}, {1.0f, 1.0f, 5.2f},
                {220, 90, 80, 255}, {90, 30, 28, 255});
}
} // namespace Maps
