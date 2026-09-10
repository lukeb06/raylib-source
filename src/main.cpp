#include "Components/Components.hpp"
#include "Registry/Registry.hpp"
#include "Systems/Systems.hpp"
#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Entity CreateBlock(Registry &reg, Vector3 pos, Vector3 size, Color color = GRAY,
                   Color wireColor = DARKGRAY) {
    Entity block = reg.CreateEntity();
    reg.AddComponent<TransformComponent>(block, {pos.x, pos.y, pos.z});
    reg.AddComponent<ColliderComponent>(block, {size.x, size.y, size.z, true});
    reg.AddComponent<ColorComponent>(block, {color, wireColor});
    return block;
}

int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Raylib Source");

    // SetTargetFPS(240);
    DisableCursor();

    Registry registry;

    Entity localPlayer = registry.CreateEntity();
    registry.AddComponent<TransformComponent>(localPlayer, {0.0f, 0.0f, 0.0f});
    registry.AddComponent<VelocityComponent>(localPlayer);
    registry.AddComponent<ColliderComponent>(localPlayer,
                                             {0.8f, 1.8f, 0.8f, false});
    registry.AddComponent<PlayerInfoComponent>(localPlayer, {"LocalHero"});
    registry.AddComponent<LocalPlayerTag>(localPlayer);
    registry.AddComponent<InputComponent>(localPlayer);
    registry.AddComponent<CameraComponent>(localPlayer);

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

    InputSystem inputSystem;
    MovementSystem movementSystem;
    CameraSystem cameraSystem;
    NetworkSystem networkSystem;

    SetExitKey(KEY_NULL);

    bool gamePaused = false;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        inputSystem.Update(registry);
        movementSystem.Update(registry, deltaTime);
        networkSystem.Update(registry, deltaTime);

        int key = GetKeyPressed();

        if (key == KEY_ESCAPE) {
            gamePaused = !gamePaused;
            std::cout << "Pressed escape, new state: " << gamePaused
                      << std::endl;

            if (gamePaused) {
                EnableCursor(); // Unlocks mouse and shows it
            } else {
                DisableCursor(); // Locks mouse to center and hides it
            }
        }

        BeginDrawing();
        if (!gamePaused) {
            ClearBackground(SKYBLUE);

            cameraSystem.Update(registry);

            DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 3.0f,
                       DARKGRAY);

            DrawText("Raylib Source", 10, 10, 20, DARKGRAY);
            DrawFPS(10, 40);

            int screenWidth = GetScreenWidth();
            int screenHeight = GetScreenHeight();

            auto transforms = registry.View<TransformComponent>();
            for (auto &[entity, transform] : transforms->data) {
                if (registry.HasComponent<CameraComponent>(entity)) {
                    if (auto vel =
                            registry.GetComponent<VelocityComponent>(entity)) {
                        float speed2D =
                            sqrtf(vel->x * vel->x + vel->z * vel->z);

                        const char *speedText = TextFormat("%.1f u/s", speed2D);
                        int fontSize = 28;
                        int textWidth = MeasureText(speedText, fontSize);

                        int posX = (screenWidth - textWidth) / 2;
                        int posY = screenHeight - 55;

                        Color speedColor = LIME;
                        if (speed2D < 0.1f) {
                            speedColor = LIGHTGRAY;
                        } else if (speed2D <= 8.0f) {
                            float t = speed2D / 8.0f;
                            speedColor = ColorAlphaBlend(
                                RAYWHITE, LIME,
                                ColorFromNormalized(Vector4{t, t, t, 1.0f}));
                        } else if (speed2D <= 16.0f) {
                            float t = (speed2D - 8.0f) / 8.0f;
                            speedColor =
                                Color{(unsigned char)(255 * t), 255, 0, 255};
                        } else {
                            speedColor = ORANGE;
                            if (speed2D > 22.0f)
                                speedColor = RED;
                        }

                        DrawText(speedText, posX + 2, posY + 2, fontSize,
                                 Color{0, 0, 0, 180});
                        DrawText(speedText, posX, posY, fontSize, speedColor);

                        int barWidth = 200;
                        int barHeight = 4;
                        int barX = (screenWidth - barWidth) / 2;
                        int barY = posY + fontSize + 4;

                        DrawRectangle(barX, barY, barWidth, barHeight,
                                      Color{40, 40, 40, 150});
                        float fillPercent = std::min(speed2D / 24.0f, 1.0f);
                        DrawRectangle(barX, barY, (int)(barWidth * fillPercent),
                                      barHeight, speedColor);
                    }
                    break;
                }
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
