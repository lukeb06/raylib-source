#pragma once
#include "../Maps/Maps.hpp"
#include <cstdint>
#include <typeindex>

enum class PacketType : uint8_t {
    PlayerState = 1,
    Map = 2,
    MapRequest = 3,
};

#pragma pack(push, 1)
struct PlayerStatePacket {
    PacketType type = PacketType::PlayerState;
    uint32_t networkID;
    float posX, posY, posZ;
    float yaw;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MapPacket {
    PacketType type = PacketType::Map;
    MapData mapData;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct MapRequestPacket {
    PacketType type = PacketType::MapRequest;
};
#pragma pack(pop)