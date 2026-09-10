#pragma once
#define STEAM_API_NODLL // Remove if building dynamically
#include "steam/isteamnetworkingsockets.h"
#include "steam/steam_api.h"
#include "steam/steamnetworkingtypes.h"
#include <iostream>

class SteamManager {
  public:
    static SteamManager &Get() {
        static SteamManager instance;
        return instance;
    }

    bool Init() {
        if (!SteamAPI_Init()) {
            std::cerr << "[Steam] SteamAPI_Init() failed! Is Steam running?"
                      << std::endl;
            return false;
        }

        m_pSockets = SteamNetworkingSockets();
        if (!m_pSockets) {
            std::cerr
                << "[Steam] Failed to get ISteamNetworkingSockets interface!"
                << std::endl;
            return false;
        }

        std::cout << "[Steam] Logged in as: "
                  << SteamFriends()->GetPersonaName() << std::endl;
        return true;
    }

    void Update() {
        SteamAPI_RunCallbacks(); // Pump Steam callbacks
    }

    void Shutdown() {
        if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
            m_pSockets->CloseListenSocket(m_hListenSocket);
        }
        if (m_hConnection != k_HSteamNetConnection_Invalid) {
            m_pSockets->CloseConnection(m_hConnection, 0, "Host Shutdown",
                                        true);
        }
        SteamAPI_Shutdown();
    }

    // --- Networking Interface ---
    bool HostLobby(uint16_t port = 27015);
    bool ConnectToUser(CSteamID steamIDHost);

    ISteamNetworkingSockets *GetSockets() { return m_pSockets; }
    HSteamListenSocket GetListenSocket() const { return m_hListenSocket; }
    HSteamNetConnection GetConnection() const { return m_hConnection; }
    bool IsHost() const { return m_isHost; }

    const std::vector<HSteamNetConnection> &GetClientConnections() const {
        return m_clientConnections;
    }
    void AddClientConnection(HSteamNetConnection conn) {
        m_clientConnections.push_back(conn);
    }
    void RemoveClientConnection(HSteamNetConnection conn) {
        std::erase(m_clientConnections, conn);
    }

  private:
    SteamManager() = default;
    ~SteamManager() { Shutdown(); }

    ISteamNetworkingSockets *m_pSockets = nullptr;
    HSteamListenSocket m_hListenSocket = k_HSteamListenSocket_Invalid;
    HSteamNetConnection m_hConnection = k_HSteamNetConnection_Invalid;
    bool m_isHost = false;

    std::vector<HSteamNetConnection> m_clientConnections;

    static void
    OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo);

  private:
    STEAM_CALLBACK(SteamManager, OnGameJoinRequested,
                   GameRichPresenceJoinRequested_t);
    // Steam Callbacks
    // STEAM_CALLBACK(SteamManager, OnConnectionStatusChanged,
    //                SteamNetConnectionStatusChangedCallback_t);
};
