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
#ifndef _C_LIBNETWORK_LOCAL_NETWORK_INTERFACES_H_
#define _C_LIBNETWORK_LOCAL_NETWORK_INTERFACES_H_

#include <string>
#include <vector>

namespace libmedia_transfer_protocol {
namespace libnetwork {

// 单条本机网卡 IPv4/IPv6 单播地址（已排除回环、DOWN、常见虚拟隧道等）
struct LocalNetworkInterfaceEntry {
  std::string adapter_name;   // 适配器标识（Windows: FriendlyName；Linux: ifa_name）
  std::string ip;             // 文本形式地址
  int address_family;         // AF_INET 或 AF_INET6（数值与平台头文件一致）
};

// 枚举可用物理/真实网卡上的单播地址。成功返回 true；失败时 out 清空并返回 false。
bool EnumerateLocalNetworkInterfaces(std::vector<LocalNetworkInterfaceEntry>* out);

// 仅枚举「同局域网其它机器通常可访问」的 IPv4：RFC1918（10/8、172.16–31、192.168/16），
// 并排除 WSL、Hyper-V vEthernet、Docker、VirtualBox、VMware 等常见虚拟网卡名称。
// 适合 GB28181/流媒体绑定本机可达地址；结果中 address_family 均为 AF_INET。
bool EnumeratePrivateLanIpv4Interfaces(std::vector<LocalNetworkInterfaceEntry>* out);

}  // namespace libnetwork
}  // namespace libmedia_transfer_protocol

#endif  // _C_LIBNETWORK_LOCAL_NETWORK_INTERFACES_H_
