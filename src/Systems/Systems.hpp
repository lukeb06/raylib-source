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
    void PollIncomingPackets(Registry &registry);
    void ApplyRemotePlayerState(Registry &registry,
                                const PlayerStatePacket &packet);
};
