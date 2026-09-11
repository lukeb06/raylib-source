#pragma once

#include "../NetworkPackets/NetworkPackets.hpp"
#include "../Registry/Registry.hpp"
#include "steam/isteamnetworkingsockets.h"

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

  private:
    void SendLocalTransform(Registry &registry,
                            ISteamNetworkingSockets *pSockets);

    // Helper to write raw data types (int, float, Vector3, Color) into the byte
    // buffer
    template <typename T>
    void WriteToBuffer(std::vector<uint8_t> &buffer, const T &data) {
        const uint8_t *ptr = reinterpret_cast<const uint8_t *>(&data);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }

    // Helper to read raw data types back out of the byte buffer
    template <typename T>
    void ReadFromBuffer(const std::vector<uint8_t> &buffer, size_t &offset,
                        T &out_data) {
        std::memcpy(&out_data, &buffer[offset], sizeof(T));
        offset += sizeof(T);
    }

    // Serialize the string (writes length first, then characters)
    void SerializeString(std::vector<uint8_t> &buffer, const std::string &str) {
        uint32_t len = static_cast<uint32_t>(str.size());
        WriteToBuffer(buffer, len);
        if (len > 0) {
            buffer.insert(buffer.end(), str.begin(), str.end());
        }
    }

    // Deserialize the string
    std::string DeserializeString(const std::vector<uint8_t> &buffer,
                                  size_t &offset) {
        uint32_t len;
        ReadFromBuffer(buffer, offset, len);
        if (len == 0)
            return "";

        std::string str(reinterpret_cast<const char *>(&buffer[offset]), len);
        offset += len;
        return str;
    }

    void PollIncomingPackets(Registry &registry);
    void ApplyRemotePlayerState(Registry &registry,
                                const PlayerStatePacket &packet);
};
