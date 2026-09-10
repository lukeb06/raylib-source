#include "SteamManager.hpp"
#include <string>

// Runs per-frame in your game loop or network system
void SteamManager::Update() {
    // Dispatches Steam callbacks like OnConnectionStatusChanged and
    // OnGameJoinRequested
    SteamAPI_RunCallbacks();

    if (m_pSockets) {
        m_pSockets->RunCallbacks();
    }
}

// Cleans up sockets and connections on exit
void SteamManager::Shutdown() {
    if (m_pSockets) {
        if (m_hListenSocket != k_HSteamListenSocket_Invalid) {
            m_pSockets->CloseListenSocket(m_hListenSocket);
            m_hListenSocket = k_HSteamListenSocket_Invalid;
        }

        if (m_hConnection != k_HSteamNetConnection_Invalid) {
            m_pSockets->CloseConnection(m_hConnection, 0, nullptr, false);
            m_hConnection = k_HSteamNetConnection_Invalid;
        }

        for (auto conn : m_clientConnections) {
            m_pSockets->CloseConnection(conn, 0, nullptr, false);
        }
        m_clientConnections.clear();
    }
}

bool SteamManager::HostLobby(uint16_t port) {
    m_pSockets = SteamNetworkingSockets();
    if (!m_pSockets)
        return false;

    // 1. Create P2P Listen Socket without local opt override
    m_hListenSocket = m_pSockets->CreateListenSocketP2P(0, 0, nullptr);
    m_isHost = true;

    // 2. Set Rich Presence connect string
    CSteamID mySteamID = SteamUser()->GetSteamID();
    std::string connectString =
        "+connect " + std::to_string(mySteamID.ConvertToUint64());
    SteamFriends()->SetRichPresence("connect", connectString.c_str());

    std::cout << "[Steam] Auto-lobby created! You can now invite friends via "
                 "Steam Overlay (Shift+Tab)."
              << std::endl;
    return true;
}

bool SteamManager::ConnectToUser(CSteamID steamIDHost) {
    m_pSockets = SteamNetworkingSockets();
    if (!m_pSockets)
        return false;

    SteamNetworkingIdentity identityRemote;
    identityRemote.SetSteamID(steamIDHost);

    // Create connection to host
    m_hConnection = m_pSockets->ConnectP2P(identityRemote, 0, 0, nullptr);
    m_isHost = false;

    return m_hConnection != k_HSteamNetConnection_Invalid;
}

// Global Connection Callback
void SteamManager::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
    if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected) {
        std::cout << "[Steam] Connection established with peer handle: "
                  << pInfo->m_hConn << std::endl;
    }

    if (pInfo->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_Connecting &&
        IsHost()) {
        // Accept incoming connection request from joining friend
        EResult res = m_pSockets->AcceptConnection(pInfo->m_hConn);
        if (res == k_EResultOK) {
            AddClientConnection(pInfo->m_hConn);
            std::cout << "[Steam] Accepted client connection: "
                      << pInfo->m_hConn << std::endl;
        } else {
            std::cout << "[Steam] Failed to accept connection: " << res
                      << std::endl;
        }
    }

    if (pInfo->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_ClosedByPeer ||
        pInfo->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {

        std::cout << "[Steam] Connection closed on handle: " << pInfo->m_hConn
                  << std::endl;
        m_pSockets->CloseConnection(pInfo->m_hConn, 0, nullptr, false);

        if (IsHost()) {
            std::erase(m_clientConnections, pInfo->m_hConn);
        }
    }
}

// Rich Presence Invite Callback
void SteamManager::OnGameJoinRequested(
    GameRichPresenceJoinRequested_t *pParam) {
    std::cout << "[Steam] Joining friend's session via invite string: "
              << pParam->m_rgchConnect << std::endl;

    std::string connectStr = pParam->m_rgchConnect;
    size_t spacePos = connectStr.find(' ');
    if (spacePos != std::string::npos) {
        uint64_t hostSteamID64 = std::stoull(connectStr.substr(spacePos + 1));
        CSteamID hostSteamID(hostSteamID64);

        ConnectToUser(hostSteamID);
    }
}