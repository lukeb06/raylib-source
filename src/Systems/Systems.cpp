#include "Systems.hpp"
#include "../Components/Components.hpp"
#include "../Maps/Maps.hpp"
#include "../SteamManager/SteamManager.hpp"
#include "raylib.h"
#include "raymath.h"
#include <algorithm>
#include <cmath>
#include <vector>

BoundingBox GetEntityAABB(const TransformComponent &transform,
                          const ColliderComponent &collider) {
    float halfW = collider.width * 0.5f;
    float halfH = collider.height * 0.5f;
    float halfD = collider.depth * 0.5f;
    return BoundingBox{
        Vector3{transform.x - halfW, transform.y - halfH, transform.z - halfD},
        Vector3{transform.x + halfW, transform.y + halfH, transform.z + halfD}};
}

BoundingBox GetEntityRenderBox(const TransformComponent &transform,
                               const BasicRenderComponent &collider) {
    float halfW = collider.width * 0.5f;
    float halfH = collider.height * 0.5f;
    float halfD = collider.depth * 0.5f;
    return BoundingBox{
        Vector3{transform.x - halfW, transform.y - halfH, transform.z - halfD},
        Vector3{transform.x + halfW, transform.y + halfH, transform.z + halfD}};
}

Entity SpawnRemotePlayer(Registry &registry, uint32_t networkID, float x,
                         float y, float z, float yaw) {
    Entity remotePlayer = registry.CreateEntity();
    registry.AddComponent<TransformComponent>(remotePlayer, {x, y, z});
    registry.AddComponent<ColliderComponent>(remotePlayer,
                                             {0.8f, 1.8f, 0.8f, false});
    // registry.AddComponent<BasicRenderComponent>(
    //     remotePlayer, {0.8f, 1.8f, 0.8f, RED, MAROON});

    Model model = LoadModel("resources/sponge.glb");

    registry.AddComponent<ModelRenderComponent>(remotePlayer, {model});

    // Wire up NetworkSyncComponent with networkID tracking
    NetworkSyncComponent sync;
    sync.networkId = networkID;
    sync.targetX = x;
    sync.targetY = y;
    sync.targetZ = z;
    sync.targetYaw = yaw - 90.0f;
    registry.AddComponent<NetworkSyncComponent>(remotePlayer, sync);

    return remotePlayer;
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

        if (IsKeyDown(KEY_R)) {
            auto transform = registry.GetComponent<TransformComponent>(entity);
            transform->x = 0.0f;
            transform->y = 0.0f;
            transform->z = 0.0f;

            auto velocity = registry.GetComponent<VelocityComponent>(entity);
            velocity->x = 0.0f;
            velocity->y = 0.0f;
            velocity->z = 0.0f;
            velocity->isGrounded = true;
        }

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
    const float MAX_STEP_HEIGHT = 0.75f;

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
            // if (!vel->isGrounded)
            //     return false;

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
            raylibCamera.up = Vector3{0.0f, 1.0f, 0.0f};
            raylibCamera.fovy = cameraComp.fov;
            raylibCamera.projection = CAMERA_PERSPECTIVE;

            BeginMode3D(raylibCamera);
            // DrawGrid(400, 1.0f);

            auto models = registry.View<ModelRenderComponent>();
            for (auto &[e, model] : models->data) {
                auto t = registry.GetComponent<TransformComponent>(e);
                if (!t)
                    continue;

                float yaw = -90.0f;

                auto NetworkSyncComp =
                    registry.GetComponent<NetworkSyncComponent>(e);
                if (NetworkSyncComp) {
                    yaw = NetworkSyncComp->targetYaw;
                }

                DrawModelEx(model.model, Vector3{t->x, t->y, t->z},
                            Vector3{0.0f, 1.0f, 0.0f}, -yaw,
                            Vector3{0.25f, 0.25f, 0.25f}, WHITE);
            }

            auto colliders = registry.View<BasicRenderComponent>();
            for (auto &[e, col] : colliders->data) {
                auto t = registry.GetComponent<TransformComponent>(e);
                if (!t)
                    continue;

                Color fillColor = col.color;
                Color wireColor = col.wireColor;

                BoundingBox box = GetEntityRenderBox(*t, col);
                Vector3 size = Vector3Subtract(box.max, box.min);
                Vector3 center = Vector3Add(box.min, Vector3Scale(size, 0.5f));

                DrawCube(center, size.x, size.y, size.z, fillColor);
                DrawCubeWires(center, size.x, size.y, size.z, wireColor);

                // BoundingBox box = GetEntityAABB(*t, col);
                // Vector3 size = Vector3Subtract(box.max, box.min);
                // Vector3 center = Vector3Add(box.min, Vector3Scale(size,
                // 0.5f));

                // if (col.isStatic) {
                //     Color fillColor = GRAY;
                //     Color wireColor = DARKGRAY;
                //     if (auto colorComp =
                //             registry.GetComponent<ColorComponent>(e)) {
                //         fillColor = colorComp->color;
                //         wireColor = colorComp->wireColor;
                //     }

                //     DrawCube(center, size.x, size.y, size.z, fillColor);
                //     DrawCubeWires(center, size.x, size.y, size.z, wireColor);
                // } else if (e != entity) {
                //     DrawCube(center, size.x, size.y, size.z, RED);
                //     DrawCubeWires(center, size.x, size.y, size.z, MAROON);
                // }
            }
            EndMode3D();
        }
    }
}

void NetworkSystem::Update(Registry &registry, float deltaTime) {
    SteamAPI_RunCallbacks();

    SteamManager::Get().Update();

    auto pSockets = SteamManager::Get().GetSockets();
    if (pSockets) {
        // 1. Broadcast local player position
        SendLocalTransform(registry, pSockets);

        // 2. Read incoming network packets and update target positions
        PollIncomingPackets(registry);
    }

    // 3. Smoothly interpolate remote entities toward their target position
    auto netPool = registry.View<NetworkSyncComponent>();
    for (auto &[entity, net] : netPool->data) {
        if (registry.HasComponent<LocalPlayerTag>(entity))
            continue;

        auto transform = registry.GetComponent<TransformComponent>(entity);
        if (transform) {
            transform->x += (net.targetX - transform->x) * 5.0f * deltaTime;
            transform->y += (net.targetY - transform->y) * 5.0f * deltaTime;
            transform->z += (net.targetZ - transform->z) * 5.0f * deltaTime;
        }
    }
}

void NetworkSystem::SendLocalTransform(Registry &registry,
                                       ISteamNetworkingSockets *pSockets) {

    auto transforms = registry.View<TransformComponent>();
    for (auto &[entity, transform] : transforms->data) {
        if (registry.HasComponent<LocalPlayerTag>(entity)) {
            PlayerStatePacket packet;
            packet.type = PacketType::PlayerState;

            // Use SteamID for globally unique network IDs so host/client entity
            // IDs don't collide
            packet.networkID = static_cast<uint32_t>(
                SteamUser()->GetSteamID().ConvertToUint64());
            packet.posX = transform.x;
            packet.posY = transform.y;
            packet.posZ = transform.z;
            packet.yaw = -90.0f;

            auto camera = registry.GetComponent<CameraComponent>(entity);
            if (camera) {
                packet.yaw = camera->yaw;
            }

            if (SteamManager::Get().IsHost()) {
                // HOST: Send local transform to ALL connected clients
                for (HSteamNetConnection clientConn :
                     SteamManager::Get().GetClientConnections()) {
                    pSockets->SendMessageToConnection(
                        clientConn, &packet, sizeof(packet),
                        k_nSteamNetworkingSend_UnreliableNoNagle, nullptr);
                }
            } else {
                // CLIENT: Send local transform to Host connection
                HSteamNetConnection hostConn =
                    SteamManager::Get().GetConnection();
                if (hostConn != k_HSteamNetConnection_Invalid) {
                    EResult result = pSockets->SendMessageToConnection(
                        hostConn, &packet, sizeof(packet),
                        k_nSteamNetworkingSend_UnreliableNoNagle, nullptr);

                    if (result != k_EResultOK) {
                        std::cout << "[Net Send Error] Failed to send packet. "
                                     "EResult code: "
                                  << result << std::endl;
                    }
                }
            }
            break;
        }
    }
}

void NetworkSystem::PollIncomingPackets(Registry &registry) {
    auto pSockets = SteamManager::Get().GetSockets();
    if (!pSockets)
        return;

    ISteamNetworkingMessage *pIncomingMsgs[16];

    // Helper lambda to process messages on a specific handle
    auto processMessagesOnHandle = [&](HSteamNetConnection conn,
                                       const char *label) {
        int numMsgs =
            pSockets->ReceiveMessagesOnConnection(conn, pIncomingMsgs, 16);

        if (numMsgs < 0) {
            std::cout << "[Net Error] Invalid connection handle passed to "
                         "ReceiveMessages: "
                      << conn << std::endl;
            return;
        }

        for (int i = 0; i < numMsgs; ++i) {
            ISteamNetworkingMessage *pMsg = pIncomingMsgs[i];

            // Safety check for empty messages
            if (pMsg->m_cbSize < sizeof(PacketType)) {
                pMsg->Release();
                continue;
            }

            // Peek at the first element to determine the Packet Type
            PacketType incomingType =
                *reinterpret_cast<PacketType *>(pMsg->m_pData);

            // ==========================================
            // CASE 1: RECEIVED MAP REQUEST -> SEND MAP DATA
            // ==========================================
            if (pMsg->m_cbSize == sizeof(MapRequestPacket) &&
                incomingType == PacketType::MapRequest) {
                std::cout << "[Net] Sending map data to client " << conn
                          << std::endl;

                // 1. Serialize the dynamic map data into a temporary byte
                // buffer
                std::vector<uint8_t> send_buffer;

                // Write Header Type
                PacketType t = PacketType::Map;
                send_buffer.insert(
                    send_buffer.end(), reinterpret_cast<uint8_t *>(&t),
                    reinterpret_cast<uint8_t *>(&t) + sizeof(PacketType));

                // Write Format String (Length prefix + characters)
                uint32_t format_len =
                    static_cast<uint32_t>(Maps::loadedMapData.format.size());
                send_buffer.insert(send_buffer.end(),
                                   reinterpret_cast<uint8_t *>(&format_len),
                                   reinterpret_cast<uint8_t *>(&format_len) +
                                       sizeof(format_len));
                if (format_len > 0) {
                    send_buffer.insert(send_buffer.end(),
                                       Maps::loadedMapData.format.begin(),
                                       Maps::loadedMapData.format.end());
                }

                // Write Version
                send_buffer.insert(
                    send_buffer.end(),
                    reinterpret_cast<uint8_t *>(&Maps::loadedMapData.version),
                    reinterpret_cast<uint8_t *>(&Maps::loadedMapData.version) +
                        sizeof(int));

                // Write Total Blocks Count
                uint32_t total_blocks =
                    static_cast<uint32_t>(Maps::loadedMapData.blocks.size());
                send_buffer.insert(send_buffer.end(),
                                   reinterpret_cast<uint8_t *>(&total_blocks),
                                   reinterpret_cast<uint8_t *>(&total_blocks) +
                                       sizeof(total_blocks));

                // Write every individual block struct
                for (const auto &block : Maps::loadedMapData.blocks) {
                    // id
                    send_buffer.insert(
                        send_buffer.end(),
                        reinterpret_cast<const uint8_t *>(&block.id),
                        reinterpret_cast<const uint8_t *>(&block.id) +
                            sizeof(int));

                    // name string inside block
                    uint32_t name_len =
                        static_cast<uint32_t>(block.name.size());
                    send_buffer.insert(send_buffer.end(),
                                       reinterpret_cast<uint8_t *>(&name_len),
                                       reinterpret_cast<uint8_t *>(&name_len) +
                                           sizeof(name_len));
                    if (name_len > 0) {
                        send_buffer.insert(send_buffer.end(),
                                           block.name.begin(),
                                           block.name.end());
                    }

                    // Blitting pure metrics data fields (Vector3s and Colors)
                    send_buffer.insert(
                        send_buffer.end(),
                        reinterpret_cast<const uint8_t *>(&block.pos),
                        reinterpret_cast<const uint8_t *>(&block.pos) +
                            sizeof(Vector3));
                    send_buffer.insert(
                        send_buffer.end(),
                        reinterpret_cast<const uint8_t *>(&block.size),
                        reinterpret_cast<const uint8_t *>(&block.size) +
                            sizeof(Vector3));
                    send_buffer.insert(
                        send_buffer.end(),
                        reinterpret_cast<const uint8_t *>(&block.color),
                        reinterpret_cast<const uint8_t *>(&block.color) +
                            sizeof(Color));
                    send_buffer.insert(
                        send_buffer.end(),
                        reinterpret_cast<const uint8_t *>(&block.wireColor),
                        reinterpret_cast<const uint8_t *>(&block.wireColor) +
                            sizeof(Color));
                }

                // 2. Send out using RELIABLE flag so no packet streams get
                // fragmented/lost
                pSockets->SendMessageToConnection(
                    conn, send_buffer.data(), send_buffer.size(),
                    k_nSteamNetworkingSend_Reliable, nullptr);
            }

            // ==========================================
            // CASE 2: RECEIVED FULL MAP PACKET -> DESERIALIZE
            // ==========================================
            else if (incomingType == PacketType::Map) {
                std::cout << "[Net] Received map data from client " << conn
                          << std::endl;

                uint8_t *raw_bytes = reinterpret_cast<uint8_t *>(pMsg->m_pData);
                size_t offset = 0;

                // Skip past the PacketType token
                offset += sizeof(PacketType);

                MapData mapData;

                // Read Format String
                uint32_t format_len = 0;
                std::memcpy(&format_len, &raw_bytes[offset],
                            sizeof(format_len));
                offset += sizeof(format_len);
                if (format_len > 0) {
                    mapData.format = std::string(
                        reinterpret_cast<char *>(&raw_bytes[offset]),
                        format_len);
                    offset += format_len;
                }

                // Read Version
                std::memcpy(&mapData.version, &raw_bytes[offset], sizeof(int));
                offset += sizeof(int);

                // Read Blocks Vector Array
                uint32_t total_blocks = 0;
                std::memcpy(&total_blocks, &raw_bytes[offset],
                            sizeof(total_blocks));
                offset += sizeof(total_blocks);
                mapData.blocks.resize(total_blocks);

                for (uint32_t b_idx = 0; b_idx < total_blocks; ++b_idx) {
                    Block b;

                    // id
                    std::memcpy(&b.id, &raw_bytes[offset], sizeof(int));
                    offset += sizeof(int);

                    // name string
                    uint32_t name_len = 0;
                    std::memcpy(&name_len, &raw_bytes[offset],
                                sizeof(name_len));
                    offset += sizeof(name_len);
                    if (name_len > 0) {
                        b.name = std::string(
                            reinterpret_cast<char *>(&raw_bytes[offset]),
                            name_len);
                        offset += name_len;
                    }

                    // Vector3/Color structs
                    std::memcpy(&b.pos, &raw_bytes[offset], sizeof(Vector3));
                    offset += sizeof(Vector3);
                    std::memcpy(&b.size, &raw_bytes[offset], sizeof(Vector3));
                    offset += sizeof(Vector3);
                    std::memcpy(&b.color, &raw_bytes[offset], sizeof(Color));
                    offset += sizeof(Color);
                    std::memcpy(&b.wireColor, &raw_bytes[offset],
                                sizeof(Color));
                    offset += sizeof(Color);

                    mapData.blocks[b_idx] = b;
                }

                std::cout << "[Net] Loading map data from packet" << std::endl;
                int loadCode = Maps::LoadMapFromMapData(registry, mapData);

                if (loadCode != 0) {
                    std::cout << "[Net Error] Failed to load map data from "
                                 "packet. Error code: "
                              << loadCode << std::endl;
                }
            }

            // ==========================================
            // CASE 3: STANDARD PLAYER STATE PACKET
            // ==========================================
            else if (pMsg->m_cbSize == sizeof(PlayerStatePacket) &&
                     incomingType == PacketType::PlayerState) {
                PlayerStatePacket *packet =
                    reinterpret_cast<PlayerStatePacket *>(pMsg->m_pData);

                ApplyRemotePlayerState(registry, *packet);

                // If Host, relay to all other clients
                if (SteamManager::Get().IsHost()) {
                    for (HSteamNetConnection clientConn :
                         SteamManager::Get().GetClientConnections()) {
                        if (clientConn != conn) {
                            pSockets->SendMessageToConnection(
                                clientConn, packet, sizeof(PlayerStatePacket),
                                k_nSteamNetworkingSend_UnreliableNoNagle,
                                nullptr);
                        }
                    }
                }
            }

            pMsg->Release();
        }
    };

    if (SteamManager::Get().IsHost()) {
        const auto &clients = SteamManager::Get().GetClientConnections();
        for (HSteamNetConnection clientConn : clients) {
            processMessagesOnHandle(clientConn, "Host Client Poll");
        }
    } else {
        HSteamNetConnection hostConn = SteamManager::Get().GetConnection();
        if (hostConn != k_HSteamNetConnection_Invalid) {
            processMessagesOnHandle(hostConn, "Client Host Poll");
        }
    }
}

void NetworkSystem::ApplyRemotePlayerState(Registry &registry,
                                           const PlayerStatePacket &packet) {
    auto netPool = registry.View<NetworkSyncComponent>();
    bool foundEntity = false;

    for (auto &[entity, net] : netPool->data) {
        // Find existing remote player matching this networkID
        if (net.networkId == packet.networkID) {
            net.targetX = packet.posX;
            net.targetY = packet.posY;
            net.targetZ = packet.posZ;
            net.targetYaw = packet.yaw - 90.0f;

            foundEntity = true;
            break;
        }
    }

    // Spawn a new ECS remote player entity if we haven't seen this networkID
    // yet
    if (!foundEntity) {
        SpawnRemotePlayer(registry, packet.networkID, packet.posX, packet.posY,
                          packet.posZ, packet.yaw);
    }
}
