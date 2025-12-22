#pragma once

#include <WinSock2.h>

enum class IPVersion {
  v4,
  v6
};

constexpr int IPVersionToAF(IPVersion version) noexcept {
  switch (version) {
    case IPVersion::v4:
      return AF_INET;
    case IPVersion::v6:
      return AF_INET6;
  }

  return AF_UNSPEC;
}
