#include "Components/Components.hpp"
#include "Maps/Maps.hpp"
#include "Registry/Registry.hpp"
#include "SteamManager/SteamManager.hpp"
#include "Systems/Systems.hpp"
#include "raylib.h"
#include "steam/steam_api.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

    const int screenWidth = 1280;
    const int screenHeight = 720;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Spongebob BHOP");

    // SetTargetFPS(240);
    DisableCursor();

    Registry registry;

    Entity localPlayer = registry.CreateLocalPlayer();

    if (!SteamAPI_Init()) {
        std::cerr << "[Steam] Error: Steam must be running to play!"
                  << std::endl;
        return 1;
    }

    SteamNetworkingUtils()->InitRelayNetworkAccess();
    SteamNetworking()->AllowP2PPacketRelay(true);

    bool joiningHostFromLaunch = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "+connect" && i + 1 < argc) {
            try {
                uint64_t hostSteamID64 = std::stoull(argv[i + 1]);
                SteamManager::Get().ConnectToUser(CSteamID(hostSteamID64));
                joiningHostFromLaunch = true;
                std::cout << "[Steam] Joining host session from launch args: "
                          << hostSteamID64 << std::endl;
            } catch (...) {
                std::cerr << "[Steam] Failed to parse +connect launch argument."
                          << std::endl;
            }
            break;
        }
    }

    // 3. Auto-host own lobby if not joining another player
    if (!joiningHostFromLaunch) {
        SteamManager::Get().HostLobby(0);
    }

    // Maps::LoadMap4(registry);
    int loadCode = Maps::LoadGameMap(registry);

    if (loadCode != 0) {
        return 1;
    }

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

        // NetworkSystem::Update pumps SteamCallbacks & handles packet sync
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

            DrawText("Spongebob BHOP", 10, 10, 20, DARKGRAY);
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

        // --- Draw 2D HUD / Pause Menu (Pure Raylib) ---
        if (gamePaused) {
            // 1. Semi-transparent dark overlay over the 3D scene
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          Color{0, 0, 0, 150});

            // 2. Center Pause Card Box
            int boxWidth = 350;
            int boxHeight = 300;
            int boxX = (GetScreenWidth() - boxWidth) / 2;
            int boxY = (GetScreenHeight() - boxHeight) / 2;

            DrawRectangle(boxX, boxY, boxWidth, boxHeight, RAYWHITE);
            DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, DARKGRAY);

            // Header Text
            DrawText("PAUSED",
                     boxX + (boxWidth - MeasureText("PAUSED", 30)) / 2,
                     boxY + 25, 30, DARKGRAY);

            // 3. Interactive Resume Button
            Rectangle resumeBtn = {(float)boxX + 50, (float)boxY + 90,
                                   (float)boxWidth - 100, 45.0f};
            Vector2 mousePos = GetMousePosition();
            bool hoverResume = CheckCollisionPointRec(mousePos, resumeBtn);

            DrawRectangleRec(resumeBtn, hoverResume ? LIGHTGRAY : GRAY);
            DrawRectangleLinesEx(resumeBtn, 2, DARKGRAY);
            DrawText("Resume Game",
                     (int)resumeBtn.x +
                         (resumeBtn.width - MeasureText("Resume Game", 20)) / 2,
                     (int)resumeBtn.y + 12, 20, hoverResume ? BLACK : WHITE);

            if (hoverResume && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                gamePaused = false;
                DisableCursor();
            }

            // 4. Interactive Quit Button
            Rectangle quitBtn = {(float)boxX + 50, (float)boxY + 160,
                                 (float)boxWidth - 100, 45.0f};
            bool hoverQuit = CheckCollisionPointRec(mousePos, quitBtn);

            DrawRectangleRec(quitBtn, hoverQuit ? RED : MAROON);
            DrawRectangleLinesEx(quitBtn, 2, DARKGRAY);
            DrawText("Quit",
                     (int)quitBtn.x +
                         (quitBtn.width - MeasureText("Quit", 20)) / 2,
                     (int)quitBtn.y + 12, 20, RAYWHITE);

            if (hoverQuit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                CloseWindow();
            }
        }

        EndDrawing();
    }

    CloseWindow();

    // 4. Clean shutdown of Steam API & sockets
    SteamManager::Get().Shutdown();
    SteamAPI_Shutdown();

    return 0;
}
