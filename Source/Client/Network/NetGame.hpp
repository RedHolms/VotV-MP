#pragma once

#include "../../Network/IP.hpp"
#include "../../Network/Socket.hpp"
#include <memory>
#include <stdint.h>
#include <string>

class NetGame {
public:
  enum class State {
    // Initial idle state
    DISCONNECTED,
    // UDP Socket created, trying to connect to the game
    CONNECTING,
    // Successfully connected. Initializing the game now
    INITIALIZING,
    // Game initialized. Synchronizing game state with the host (if we're not the host)
    SYNCHRONIZATION,
    // Everything is ready to play
    CONNECTED,
    // Connection lost and trying to reinitiate the connection
    // Will pause the game and jump to CONNECTING
    RECONNECTING
  };

private:
  IP m_address;
  uint16_t m_port;
  std::string m_key;

  std::unique_ptr<Socket> m_socket;
  State m_state = State::DISCONNECTED;
  bool m_host = false;

public:
  NetGame(IP const& address, uint16_t port, std::string_view const& key);
  ~NetGame();

  NetGame(NetGame const&) = delete;
  NetGame(NetGame&&) = delete;
  NetGame& operator=(NetGame const&) = delete;
  NetGame& operator=(NetGame&&) = delete;

public:
  void beginConnection();
};

extern NetGame* gNetGame;
