#include "NetGame.hpp"

NetGame::NetGame(IP const& address, uint16_t port, std::string_view const& key)
  : m_address(address),
    m_port(port),
    m_key(key)
{}

NetGame::~NetGame() = default;

void NetGame::beginConnection() {
  m_socket = std::make_unique<Socket>(m_address.getVersion(), Protocol::UDP);
}
