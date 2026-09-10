#pragma once
#include <cstdint>

enum class PacketType : uint8_t {
    PlayerState = 1,
};

#pragma pack(push, 1)
struct PlayerStatePacket {
    PacketType type = PacketType::PlayerState;
    uint32_t networkID;
    float posX, posY, posZ;
    float velX, velY, velZ;
};
#pragma pack(pop)
