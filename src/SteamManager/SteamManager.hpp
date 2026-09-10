#pragma once
#include <iostream>
#include <steam/isteamnetworkingsockets.h>
#include <steam/steam_api.h>
#include <steam/steamnetworkingtypes.h>
#include <vector>

class SteamManager {
  public:
    static SteamManager &Get() {
        static SteamManager instance;
        return instance;
    }

    // FIX 2: Default argument lets HostLobby() be called with 0 arguments
    bool HostLobby(uint16_t port = 0);
    bool ConnectToUser(CSteamID steamIDHost);

    // FIX 1 & 3: Add Update() and Shutdown() declarations
    void Update();
    void Shutdown();

    ISteamNetworkingSockets *GetSockets() { return m_pSockets; }
    bool IsHost() const { return m_isHost; }
    HSteamNetConnection GetConnection() const { return m_hConnection; }
    const std::vector<HSteamNetConnection> &GetClientConnections() const {
        return m_clientConnections;
    }

    void AddClientConnection(HSteamNetConnection conn) {
        m_clientConnections.push_back(conn);
    }

  private:
    SteamManager() = default;

    ISteamNetworkingSockets *m_pSockets = nullptr;
    HSteamListenSocket m_hListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
    std::vector<HSteamNetConnection> m_clientConnections;
    bool m_isHost = false;

    STEAM_CALLBACK(SteamManager, OnConnectionStatusChanged,
                   SteamNetConnectionStatusChangedCallback_t);
    STEAM_CALLBACK(SteamManager, OnGameJoinRequested,
                   GameRichPresenceJoinRequested_t);
};