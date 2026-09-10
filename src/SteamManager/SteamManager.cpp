#include "SteamManager.hpp"

bool SteamManager::HostLobby(uint16_t port) {
    m_pSockets = SteamNetworkingSockets();
    if (!m_pSockets)
        return false;

    // 1. Create P2P Listen Socket (Hosted via SteamRelay)
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void *)OnConnectionStatusChanged);

    m_hListenSocket = m_pSockets->CreateListenSocketP2P(0, 1, &opt);
    m_isHost = true;

    // 2. Obtain local Steam ID
    CSteamID mySteamID = SteamUser()->GetSteamID();

    // 3. Set Rich Presence connect string so friends can join your P2P socket
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

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void *)OnConnectionStatusChanged);

    m_hConnection = m_pSockets->ConnectP2P(identityRemote, 0, 1, &opt);
    m_isHost = false;

    return m_hConnection != k_HSteamNetConnection_Invalid;
}

void SteamManager::OnConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
    auto &manager = SteamManager::Get();

    switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_Connecting: {
        if (manager.IsHost()) {
            if (manager.GetSockets()->AcceptConnection(pInfo->m_hConn) ==
                k_EResultOK) {
                // Track newly accepted client
                manager.AddClientConnection(pInfo->m_hConn);
            } else {
                manager.GetSockets()->CloseConnection(
                    pInfo->m_hConn, 0, "Failed to accept", false);
            }
        }
        break;
    }
    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
        manager.RemoveClientConnection(pInfo->m_hConn);
        manager.GetSockets()->CloseConnection(pInfo->m_hConn, 0, "Disconnected",
                                              false);
        break;
    }
    default:
        break;
    }
}

void SteamManager::OnGameJoinRequested(
    GameRichPresenceJoinRequested_t *pParam) {
    std::cout << "[Steam] Joining friend's session via invite string: "
              << pParam->m_rgchConnect << std::endl;

    // Parse host SteamID from connect string (+connect <SteamID64>)
    std::string connectStr = pParam->m_rgchConnect;
    size_t spacePos = connectStr.find(' ');
    if (spacePos != std::string::npos) {
        uint64_t hostSteamID64 = std::stoull(connectStr.substr(spacePos + 1));
        CSteamID hostSteamID(hostSteamID64);

        // Connect directly via Steam P2P Networking
        ConnectToUser(hostSteamID);
    }
}
