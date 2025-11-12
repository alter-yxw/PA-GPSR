# PA-GPSR Migration to ns-3.40

本文档说明如何将 PA-GPSR、GPSR、MM-GPSR 和 Location Service 模块从 ns-3.27 迁移到 ns-3.40。

## 主要变化总结

### 1. 构建系统变化
- **ns-3.27**: 使用 waf 构建系统和 wscript 文件
- **ns-3.40**: 使用 CMake 构建系统和 CMakeLists.txt 文件

### 2. WiFi API 变化
- **AdhocWifiMac**: 在 ns-3.40 中,不再需要显式转换为 `AdhocWifiMac`,直接使用 `WifiMac` 基类即可
- **NqosWaveMacHelper**: 已被移除,改用 `WifiMacHelper` 并配置为 `OcbWifiMac` 类型
- **WiFi PHY 配置**: `CcaMode1Threshold` 属性更名为 `CcaEdThreshold`

### 3. Ipv4Address API 变化
- **Ipv4Address::IsEqual()**: 此方法已被移除，使用 `==` 运算符代替
  - 旧代码: `if (addr1.IsEqual(addr2))`
  - 新代码: `if (addr1 == addr2)`

### 4. Ipv4RoutingProtocol 接口变化
- **RouteInput 回调参数**: 从值传递改为常量引用传递
  - 旧代码: `bool RouteInput(..., UnicastForwardCallback ucb, ...)`
  - 新代码: `bool RouteInput(..., const UnicastForwardCallback& ucb, ...)`
- **PrintRoutingTable**: 添加了第二个参数 `Time::Unit`
  - 旧代码: `void PrintRoutingTable(Ptr<OutputStreamWrapper>) const`
  - 新代码: `void PrintRoutingTable(Ptr<OutputStreamWrapper>, Time::Unit = Time::S) const`

### 5. 头文件变化
- `#include "ns3/adhoc-wifi-mac.h"` → `#include "ns3/wifi-mac.h"`
- 移除了 `ocb-wifi-mac.h` 的单独包含(已合并到 wave-mac-helper.h)

## 安装步骤

### 前提条件
确保您已经安装了 ns-3.40。如果还没有安装,请从官方网站下载并安装。

### 步骤 1: 复制模块到 contrib 目录

将四个模块复制到您的 ns-3.40 安装目录的 `contrib/` 目录下:

```bash
# 假设您的 ns-3.40 安装在 ~/ns-allinone-3.40/ns-3.40/
NS3_DIR=~/ns-allinone-3.40/ns-3.40
cd /path/to/PA-GPSR

# 复制所有模块到 contrib 目录
cp -r src/location-service $NS3_DIR/contrib/
cp -r src/pagpsr $NS3_DIR/contrib/
cp -r src/gpsr $NS3_DIR/contrib/
cp -r src/mmgpsr $NS3_DIR/contrib/
```

### 步骤 2: 复制示例程序

```bash
# 复制示例程序到 scratch 或 examples 目录
cp examples/pagpsr-main.cc $NS3_DIR/scratch/
```

### 步骤 3: 配置和构建

```bash
cd $NS3_DIR

# 配置 ns-3(CMake 会自动检测 contrib 目录中的模块)
./ns3 configure --enable-examples --enable-tests

# 构建 ns-3
./ns3 build
```

### 步骤 4: 验证安装

检查模块是否正确安装:

```bash
./ns3 show modules | grep -E "location-service|pagpsr|gpsr|mmgpsr"
```

您应该能看到以下输出:
```
location-service
pagpsr
gpsr
mmgpsr
```

### 步骤 5: 运行示例程序

```bash
# 运行 PA-GPSR 示例
./ns3 run scratch/pagpsr-main -- --size=30 --time=200
```

## 详细代码变化

### 1. CMakeLists.txt 文件

每个模块都添加了 CMakeLists.txt 文件来替代原来的 wscript 文件。

**示例 (pagpsr/CMakeLists.txt)**:
```cmake
set(source_files
    model/pagpsr-rtable.cc
    model/pagpsr-rst-table.cc
    model/pagpsr-ptable.cc
    model/pagpsr-rqueue.cc
    model/pagpsr-packet.cc
    model/pagpsr.cc
    helper/pagpsr-helper.cc
)

set(header_files
    model/pagpsr-rtable.h
    model/pagpsr-rst-table.h
    model/pagpsr-ptable.h
    model/pagpsr-rqueue.h
    model/pagpsr-packet.h
    model/pagpsr.h
    helper/pagpsr-helper.h
)

build_lib(
  LIBNAME pagpsr
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${liblocation-service}
    ${libinternet}
    ${libwifi}
    ${libapplications}
    ${libmesh}
    ${libpoint-to-point}
    ${libvirtual-net-device}
)
```

### 2. WiFi MAC 配置变化

**原代码 (ns-3.27)**:
```cpp
#include "ns3/adhoc-wifi-mac.h"

// ...
Ptr<WifiMac> mac = wifi->GetMac()->GetObject<AdhocWifiMac>();
if (mac != 0) {
    mac->TraceDisconnectWithoutContext("TxErrHeader",
                                      m_neighbors.GetTxErrorCallback());
}
```

**新代码 (ns-3.40)**:
```cpp
#include "ns3/wifi-mac.h"

// ...
Ptr<WifiMac> mac = wifi->GetMac();
if (mac != 0) {
    mac->TraceDisconnectWithoutContext("TxErrHeader",
                                      m_neighbors.GetTxErrorCallback());
}
```

### 3. 示例程序中的 WiFi 配置

**原代码 (ns-3.27)**:
```cpp
NqosWaveMacHelper wifiMac = NqosWaveMacHelper::Default();
wifiPhy.Set("CcaMode1Threshold", DoubleValue(-64.8));
```

**新代码 (ns-3.40)**:
```cpp
WifiMacHelper wifiMac;
wifiMac.SetType("ns3::OcbWifiMac");
wifiPhy.Set("CcaEdThreshold", DoubleValue(-64.8));
```

### 4. Ipv4Address API 变化

**原代码 (ns-3.27)**:
```cpp
// 在 pagpsr-ptable.cc, gpsr-ptable.cc 等文件中
std::map<Ipv4Address, ...>::iterator i = m_table.find(id);
if (i != m_table.end() || id.IsEqual(i->first)) {
    // ...
}
```

**新代码 (ns-3.40)**:
```cpp
// IsEqual() 方法已被移除，使用 == 运算符
std::map<Ipv4Address, ...>::iterator i = m_table.find(id);
if (i != m_table.end() || id == i->first) {
    // ...
}
```

**影响的文件**:
- `src/pagpsr/model/pagpsr-rtable.cc` (2处)
- `src/pagpsr/model/pagpsr-rst-table.cc` (2处)
- `src/pagpsr/model/pagpsr-ptable.cc` (2处)
- `src/gpsr/model/gpsr-ptable.cc` (2处)
- `src/mmgpsr/model/mmgpsr-ptable.cc` (2处)
- `src/mmgpsr/model/mmgpsr-Ttable.cc` (1处)

### 5. Ipv4RoutingProtocol 接口变化

ns-3.40 中 `Ipv4RoutingProtocol` 接口发生了变化，导致所有派生类都需要更新。

**问题**: 如果不更新会导致编译错误:
```
error: invalid new-expression of abstract class type 'ns3::pagpsr::RoutingProtocol'
note: because the following virtual functions are pure within 'ns3::pagpsr::RoutingProtocol':
note: 'virtual bool ns3::Ipv4RoutingProtocol::RouteInput(...)'
note: 'virtual void ns3::Ipv4RoutingProtocol::PrintRoutingTable(...)'
```

#### 5.1 RouteInput 方法签名变化

**原代码 (ns-3.27)** - 头文件 (*.h):
```cpp
bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                 UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                 LocalDeliverCallback lcb, ErrorCallback ecb);
```

**新代码 (ns-3.40)** - 头文件 (*.h):
```cpp
bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                 const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb,
                 const LocalDeliverCallback& lcb, const ErrorCallback& ecb);
```

**关键变化**: 所有回调参数从值传递改为常量引用传递 (添加 `const` 和 `&`)

**原代码 (ns-3.27)** - 实现文件 (*.cc):
```cpp
bool RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                  UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                  LocalDeliverCallback lcb, ErrorCallback ecb)
{
    // 实现...
}
```

**新代码 (ns-3.40)** - 实现文件 (*.cc):
```cpp
bool RoutingProtocol::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev,
                                  const UnicastForwardCallback& ucb, const MulticastForwardCallback& mcb,
                                  const LocalDeliverCallback& lcb, const ErrorCallback& ecb)
{
    // 实现...
}
```

#### 5.2 PrintRoutingTable 方法签名变化

**原代码 (ns-3.27)**:
```cpp
virtual void PrintRoutingTable (ns3::Ptr<ns3::OutputStreamWrapper>) const
{
  return;
}
```

**新代码 (ns-3.40)**:
```cpp
virtual void PrintRoutingTable (ns3::Ptr<ns3::OutputStreamWrapper>, ns3::Time::Unit = ns3::Time::S) const
{
  return;
}
```

**关键变化**: 添加了第二个参数 `Time::Unit`，默认值为 `Time::S`

**影响的文件**:
- `src/pagpsr/model/pagpsr.h` (RouteInput 声明, PrintRoutingTable 声明)
- `src/pagpsr/model/pagpsr.cc` (RouteInput 实现)
- `src/gpsr/model/gpsr.h` (RouteInput 声明, PrintRoutingTable 声明)
- `src/gpsr/model/gpsr.cc` (RouteInput 实现)
- `src/mmgpsr/model/mmgpsr.h` (RouteInput 声明, PrintRoutingTable 声明)
- `src/mmgpsr/model/mmgpsr.cc` (RouteInput 实现)

**注意**: 所有三个路由协议 (PA-GPSR, GPSR, MM-GPSR) 都需要同样的修改。

## 模块依赖关系

```
location-service (基础模块)
    └── network

pagpsr, gpsr, mmgpsr
    ├── location-service
    ├── internet
    ├── wifi
    ├── applications
    ├── mesh
    ├── point-to-point
    └── virtual-net-device
```

## 常见问题

### 问题 1: 编译时找不到头文件

**解决方案**: 确保所有模块都已正确复制到 `contrib/` 目录,并且运行了 `./ns3 configure`。

### 问题 2: 链接错误,找不到符号

**解决方案**: 检查 CMakeLists.txt 中的 `LIBRARIES_TO_LINK` 部分是否包含了所有必要的依赖。

### 问题 3: WiFi 配置不工作

**解决方案**: 确保使用了 `WifiMacHelper` 并正确设置为 `OcbWifiMac` 类型:
```cpp
WifiMacHelper wifiMac;
wifiMac.SetType("ns3::OcbWifiMac");
```

### 问题 4: 运行时出现 TypeId 错误

**解决方案**: 这通常意味着模块没有正确注册。确保:
1. 模块的 CMakeLists.txt 正确配置
2. 运行了 `./ns3 build` 重新构建
3. 所有依赖模块都已安装

## 兼容性说明

### 已测试的 ns-3 版本
- ns-3.40 ✓

### 已知限制
1. 本迁移基于 WAVE/802.11p 配置,如果您使用其他 WiFi 标准,可能需要额外调整
2. 某些高级 WiFi 功能可能需要根据具体使用场景进行调整

## 性能注意事项

迁移到 ns-3.40 后,协议的功能应该保持不变。但是:
- ns-3.40 的 WiFi 模型可能有性能改进
- CMake 构建系统通常比 waf 更快
- 确保在相同的配置下比较性能结果

## 进一步优化建议

1. **使用新的 ns-3.40 特性**: 考虑使用 ns-3.40 中新增的网络功能
2. **代码现代化**: 考虑使用 C++14/17 特性(ns-3.40 支持)
3. **日志优化**: 利用 ns-3.40 改进的日志系统
4. **测试**: 添加单元测试以确保迁移后功能正确

## 参考资源

- [ns-3.40 官方文档](https://www.nsnam.org/documentation/)
- [ns-3 WiFi 模块文档](https://www.nsnam.org/docs/release/3.40/models/html/wifi.html)
- [ns-3 WAVE 模块文档](https://www.nsnam.org/docs/release/3.40/models/html/wave.html)

## 贡献

如果您在迁移过程中发现问题或有改进建议,请提交 issue 或 pull request。

## 版本历史

- **v1.0** (2025-11-12): 初始迁移到 ns-3.40
  - 添加 CMakeLists.txt 支持
  - 更新 WiFi API
  - 修复所有已知的兼容性问题
