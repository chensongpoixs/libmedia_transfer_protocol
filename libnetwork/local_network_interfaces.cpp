/******************************************************************************
 *  Copyright (c) 2025 The CRTC project authors . All Rights Reserved.
 *
 *  Please visit https://chensongpoixs.github.io for detail
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 ******************************************************************************/
#include "libmedia_transfer_protocol/libnetwork/local_network_interfaces.h"

#include <cstring>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#endif

namespace libmedia_transfer_protocol {
namespace libnetwork {

namespace {

std::string AsciiLowerCopy(std::string s) {
  for (char& c : s) {
    if (static_cast<unsigned char>(c) < 128 && c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return s;
}

// 常见虚拟/隧道网卡：名称命中则不作为「局域网对外可达」地址来源（仍可能为 RFC1918，如 WSL 172.29.x）
bool IsLikelyVirtualAdapterByName(const std::string& name) {
  const std::string l = AsciiLowerCopy(name);
  static const char* kSubstrings[] = {
      "vethernet",  // Windows Hyper-V / WSL / Docker Desktop
      "wsl",
      "hyper-v",
      "virtualbox",
      "vmware",
      "docker",
      "vboxnet",
      "npcap",
      "tap-windows",
      "zerotier",
      "tailscale",
      "wireguard",
      "bluetooth",   // 蓝牙 PAN 等，一般不作为 GB 媒体面
  };
  for (const char* sub : kSubstrings) {
    if (l.find(sub) != std::string::npos) {
      return true;
    }
  }
  // Linux 常见虚拟接口名
  if (l == "docker0" || l.compare(0, 3, "br-") == 0 || l.compare(0, 4, "veth") == 0) {
    return true;
  }
  if (l.find("virbr") != std::string::npos) {
    return true;
  }
  return false;
}

bool IsRfc1918Ipv4Bytes(const uint8_t b[4]) {
  if (b[0] == 10) {
    return true;
  }
  if (b[0] == 172 && b[1] >= 16 && b[1] <= 31) {
    return true;
  }
  if (b[0] == 192 && b[1] == 168) {
    return true;
  }
  return false;
}

#if defined(_WIN32)

bool InitWinsockOnce() {
  static bool inited = false;
  static bool ok = false;
  if (inited) {
    return ok;
  }
  inited = true;
  WSADATA wsa;
  ok = (WSAStartup(MAKEWORD(2, 2), &wsa) == 0);
  return ok;
}

// 排除 IPv4 回环 127.0.0.0/8
bool IsIpv4Loopback(const IN_ADDR* addr) {
  const uint8_t* b = reinterpret_cast<const uint8_t*>(&addr->S_un);
  return b[0] == 127;
}

std::string WideToUtf8(const wchar_t* wstr) {
  if (!wstr || !*wstr) {
    return std::string();
  }
  int need = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
  if (need <= 0) {
    return std::string();
  }
  std::string out(static_cast<size_t>(need - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &out[0], need, nullptr, nullptr);
  return out;
}

bool ShouldSkipWindowsAdapter(const IP_ADAPTER_ADDRESSES* a) {
  if (!a) {
    return true;
  }
  if (a->OperStatus != IfOperStatusUp) {
    return true;
  }
  if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK) {
    return true;
  }
  // Teredo / 6to4 等隧道，一般不作为本机“真实网卡”展示
  if (a->IfType == IF_TYPE_TUNNEL) {
    return true;
  }
  return false;
}

#endif  // _WIN32

#if !defined(_WIN32)

bool IsIpv4LoopbackAddr(const struct sockaddr_in* sin) {
  const uint8_t* b = reinterpret_cast<const uint8_t*>(&sin->sin_addr);
  return b[0] == 127;
}

#endif

}  // namespace

bool EnumerateLocalNetworkInterfaces(std::vector<LocalNetworkInterfaceEntry>* out) {
  if (!out) {
    return false;
  }
  out->clear();

#if defined(_WIN32)
  if (!InitWinsockOnce()) {
    return false;
  }

  ULONG buf_len = 15000;
  std::vector<uint8_t> buffer(buf_len);
  PIP_ADAPTER_ADDRESSES addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

  ULONG ret = GetAdaptersAddresses(AF_UNSPEC,
                                   GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                       GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_PREFIX,
                                   nullptr, addrs, &buf_len);
  if (ret == ERROR_BUFFER_OVERFLOW) {
    buffer.resize(buf_len);
    addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    ret = GetAdaptersAddresses(AF_UNSPEC,
                               GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                   GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_PREFIX,
                               nullptr, addrs, &buf_len);
  }
  if (ret != NO_ERROR) {
    return false;
  }

  for (PIP_ADAPTER_ADDRESSES a = addrs; a != nullptr; a = a->Next) {
    if (ShouldSkipWindowsAdapter(a)) {
      continue;
    }
    std::string adapter_name = WideToUtf8(a->FriendlyName);
    if (adapter_name.empty()) {
      adapter_name = a->AdapterName ? a->AdapterName : "";
    }

    for (PIP_ADAPTER_UNICAST_ADDRESS ua = a->FirstUnicastAddress; ua != nullptr;
         ua = ua->Next) {
      if (!ua->Address.lpSockaddr) {
        continue;
      }
      const SOCKET_ADDRESS* sa = &ua->Address;
      if (sa->lpSockaddr->sa_family == AF_INET) {
        const SOCKADDR_IN* sin = reinterpret_cast<const SOCKADDR_IN*>(sa->lpSockaddr);
        if (IsIpv4Loopback(&sin->sin_addr)) {
          continue;
        }
        char ipbuf[INET_ADDRSTRLEN];
        if (!InetNtopA(AF_INET, &sin->sin_addr, ipbuf, INET_ADDRSTRLEN)) {
          continue;
        }
        LocalNetworkInterfaceEntry e;
        e.adapter_name = adapter_name;
        e.ip = ipbuf;
        e.address_family = AF_INET;
        out->push_back(std::move(e));
      } else if (sa->lpSockaddr->sa_family == AF_INET6) {
        const SOCKADDR_IN6* sin6 = reinterpret_cast<const SOCKADDR_IN6*>(sa->lpSockaddr);
        if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
          continue;
        }
        // 链路本地 fe80::/10，多为自动配置，业务上常需过滤；若需保留可删除此段
        if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
          continue;
        }
        char ipbuf[INET6_ADDRSTRLEN];
        if (!InetNtopA(AF_INET6, &sin6->sin6_addr, ipbuf, INET6_ADDRSTRLEN)) {
          continue;
        }
        LocalNetworkInterfaceEntry e;
        e.adapter_name = adapter_name;
        e.ip = ipbuf;
        e.address_family = AF_INET6;
        out->push_back(std::move(e));
      }
    }
  }
  return true;

#else  // Linux / POSIX

  struct ifaddrs* ifap = nullptr;
  if (getifaddrs(&ifap) != 0) {
    return false;
  }

  for (struct ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if ((ifa->ifa_flags & IFF_UP) == 0) {
      continue;
    }
    if (ifa->ifa_flags & IFF_LOOPBACK) {
      continue;
    }

    int family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      const struct sockaddr_in* sin = reinterpret_cast<const struct sockaddr_in*>(ifa->ifa_addr);
      if (IsIpv4LoopbackAddr(sin)) {
        continue;
      }
      char ipbuf[INET_ADDRSTRLEN];
      if (!inet_ntop(AF_INET, &sin->sin_addr, ipbuf, sizeof(ipbuf))) {
        continue;
      }
      LocalNetworkInterfaceEntry e;
      e.adapter_name = ifa->ifa_name ? ifa->ifa_name : "";
      e.ip = ipbuf;
      e.address_family = AF_INET;
      out->push_back(std::move(e));
    } else if (family == AF_INET6) {
      const struct sockaddr_in6* sin6 =
          reinterpret_cast<const struct sockaddr_in6*>(ifa->ifa_addr);
      if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr)) {
        continue;
      }
      if (IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
        continue;
      }
      char ipbuf[INET6_ADDRSTRLEN];
      if (!inet_ntop(AF_INET6, &sin6->sin6_addr, ipbuf, sizeof(ipbuf))) {
        continue;
      }
      LocalNetworkInterfaceEntry e;
      e.adapter_name = ifa->ifa_name ? ifa->ifa_name : "";
      e.ip = ipbuf;
      e.address_family = AF_INET6;
      out->push_back(std::move(e));
    }
  }

  freeifaddrs(ifap);
  return true;
#endif
}

bool EnumeratePrivateLanIpv4Interfaces(std::vector<LocalNetworkInterfaceEntry>* out) {
  if (!out) {
    return false;
  }
  out->clear();

#if defined(_WIN32)
  if (!InitWinsockOnce()) {
    return false;
  }

  ULONG buf_len = 15000;
  std::vector<uint8_t> buffer(buf_len);
  PIP_ADAPTER_ADDRESSES addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());

  ULONG ret = GetAdaptersAddresses(AF_UNSPEC,
                                   GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                       GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_PREFIX,
                                   nullptr, addrs, &buf_len);
  if (ret == ERROR_BUFFER_OVERFLOW) {
    buffer.resize(buf_len);
    addrs = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(buffer.data());
    ret = GetAdaptersAddresses(AF_UNSPEC,
                               GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                                   GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_INCLUDE_PREFIX,
                               nullptr, addrs, &buf_len);
  }
  if (ret != NO_ERROR) {
    return false;
  }

  for (PIP_ADAPTER_ADDRESSES a = addrs; a != nullptr; a = a->Next) {
    if (ShouldSkipWindowsAdapter(a)) {
      continue;
    }
    std::string adapter_name = WideToUtf8(a->FriendlyName);
    if (adapter_name.empty()) {
      adapter_name = a->AdapterName ? a->AdapterName : "";
    }
    if (IsLikelyVirtualAdapterByName(adapter_name)) {
      continue;
    }

    for (PIP_ADAPTER_UNICAST_ADDRESS ua = a->FirstUnicastAddress; ua != nullptr;
         ua = ua->Next) {
      if (!ua->Address.lpSockaddr) {
        continue;
      }
      const SOCKET_ADDRESS* sa = &ua->Address;
      if (sa->lpSockaddr->sa_family != AF_INET) {
        continue;
      }
      const SOCKADDR_IN* sin = reinterpret_cast<const SOCKADDR_IN*>(sa->lpSockaddr);
      if (IsIpv4Loopback(&sin->sin_addr)) {
        continue;
      }
      const uint8_t* b = reinterpret_cast<const uint8_t*>(&sin->sin_addr);
      if (!IsRfc1918Ipv4Bytes(b)) {
        continue;
      }
      char ipbuf[INET_ADDRSTRLEN];
      if (!InetNtopA(AF_INET, &sin->sin_addr, ipbuf, INET_ADDRSTRLEN)) {
        continue;
      }
      LocalNetworkInterfaceEntry e;
      e.adapter_name = adapter_name;
      e.ip = ipbuf;
      e.address_family = AF_INET;
      out->push_back(std::move(e));
    }
  }
  return true;

#else

  struct ifaddrs* ifap = nullptr;
  if (getifaddrs(&ifap) != 0) {
    return false;
  }

  for (struct ifaddrs* ifa = ifap; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if ((ifa->ifa_flags & IFF_UP) == 0) {
      continue;
    }
    if (ifa->ifa_flags & IFF_LOOPBACK) {
      continue;
    }
    std::string adapter_name = ifa->ifa_name ? ifa->ifa_name : "";
    if (IsLikelyVirtualAdapterByName(adapter_name)) {
      continue;
    }

    if (ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    const struct sockaddr_in* sin = reinterpret_cast<const struct sockaddr_in*>(ifa->ifa_addr);
    if (IsIpv4LoopbackAddr(sin)) {
      continue;
    }
    const uint8_t* b = reinterpret_cast<const uint8_t*>(&sin->sin_addr);
    if (!IsRfc1918Ipv4Bytes(b)) {
      continue;
    }
    char ipbuf[INET_ADDRSTRLEN];
    if (!inet_ntop(AF_INET, &sin->sin_addr, ipbuf, sizeof(ipbuf))) {
      continue;
    }
    LocalNetworkInterfaceEntry e;
    e.adapter_name = adapter_name;
    e.ip = ipbuf;
    e.address_family = AF_INET;
    out->push_back(std::move(e));
  }

  freeifaddrs(ifap);
  return true;
#endif
}

}  // namespace libnetwork
}  // namespace libmedia_transfer_protocol
