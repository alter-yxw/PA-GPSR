# PA-GPSR 在 benchmarktest.cc 中的测试指南

## ✅ 验证清单

### 1. 当前状态确认

✅ **已完成**:
- PA-GPSR、GPSR、MM-GPSR 已适配 ns-3.40
- WiFi API 已更新 (AdhocWifiMac → WifiMac)
- CMakeLists.txt 已创建
- 所有模块理论上可以编译

⚠️ **需要修改**:
- benchmarktest.cc 需要添加 Install() 调用

## 🔧 必需的修改

### 修改 benchmarktest.cc

在 `SimulationTest::Run()` 方法中，`InstallApplications()` 之后添加以下代码：

```cpp
void SimulationTest::Run ()
{
  SeedManager::SetSeed (seed);
  CreateNodes();
  CreateDevices();
  InstallEnergyModel();
  InstallInternetStack ();

  if(appOn) { InstallApplications (); }

  // ========== 在这里添加以下代码 ==========
  // PA-GPSR/GPSR/MM-GPSR 需要额外的 Install 步骤来设置 UDP 回调
  if (algorithm == "pagpsr") {
    PAGpsrHelper pagpsrHelper;
    pagpsrHelper.Install();
    std::cout << "PA-GPSR Install() completed\n";
  } else if (algorithm == "gpsr") {
    GpsrHelper gpsrHelper;
    gpsrHelper.Install();
    std::cout << "GPSR Install() completed\n";
  } else if (algorithm == "mmgpsr") {
    MMGpsrHelper mmgpsrHelper;
    mmgpsrHelper.Install();
    std::cout << "MM-GPSR Install() completed\n";
  }
  // ========== 添加结束 ==========

  m_metrics =  CreateObject<PerformanceMetrics>();
  m_metrics -> SetNodes(nodes);
  // ... 其余代码保持不变
}
```

### 为什么需要这个修改？

PA-GPSR/GPSR/MM-GPSR 使用地理位置路由，需要在 UDP 数据包中添加位置信息头部。`Install()` 方法的作用是：

1. 获取每个节点的 UDP 协议对象
2. 获取路由协议对象
3. 将路由协议插入到 UDP 和 IP 层之间
4. 拦截数据包，添加地理位置信息

**代码实现** (src/pagpsr/helper/pagpsr-helper.cc):
```cpp
void PAGpsrHelper::Install (void) const {
  NodeContainer c = NodeContainer::GetGlobal ();
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i) {
      Ptr<Node> node = (*i);
      Ptr<UdpL4Protocol> udp = node->GetObject<UdpL4Protocol> ();
      Ptr<pagpsr::RoutingProtocol> pagpsr = node->GetObject<pagpsr::RoutingProtocol> ();

      // 保存原始的下行目标，然后用路由协议替换
      pagpsr->SetDownTarget (udp->GetDownTarget ());
      udp->SetDownTarget (MakeCallback(&pagpsr::RoutingProtocol::AddHeaders, pagpsr));
  }
}
```

## 📥 安装步骤

### 步骤 1: 准备 ns-3.40 环境

```bash
# 假设 ns-3.40 已安装在 ~/ns-allinone-3.40/ns-3.40
NS3_DIR=~/ns-allinone-3.40/ns-3.40
cd $NS3_DIR
```

### 步骤 2: 安装 PA-GPSR 模块

```bash
# 从 PA-GPSR 仓库复制模块
PA_GPSR_DIR=/path/to/PA-GPSR

cp -r $PA_GPSR_DIR/src/location-service contrib/
cp -r $PA_GPSR_DIR/src/pagpsr contrib/
cp -r $PA_GPSR_DIR/src/gpsr contrib/
cp -r $PA_GPSR_DIR/src/mmgpsr contrib/
```

### 步骤 3: 复制并修改 benchmarktest.cc

```bash
# 复制到 scratch 目录
cp /path/to/benchmarktest.cc scratch/

# 编辑文件，添加上述的 Install() 调用
# 使用你喜欢的编辑器: vim, nano, 或 VS Code
vim scratch/benchmarktest.cc
```

或者应用补丁：
```bash
cd scratch
patch benchmarktest.cc < $PA_GPSR_DIR/benchmarktest_pagpsr_fix.patch
```

### 步骤 4: 配置和编译

```bash
cd $NS3_DIR

# 配置 (CMake 会自动检测 contrib 目录中的模块)
./ns3 configure --enable-examples --enable-tests

# 编译
./ns3 build

# 验证模块已安装
./ns3 show modules | grep -E "location-service|pagpsr|gpsr|mmgpsr"
```

**预期输出**:
```
location-service
pagpsr
gpsr
mmgpsr
```

## 🧪 测试运行

### 快速功能测试 (小规模)

```bash
cd $NS3_DIR

# 测试 PA-GPSR (20节点, 30秒)
./ns3 run "scratch/benchmarktest --algorithm=pagpsr --size=20 --time=30 --AppsOn=true"

# 测试 GPSR
./ns3 run "scratch/benchmarktest --algorithm=gpsr --size=20 --time=30 --AppsOn=true"

# 测试 MM-GPSR
./ns3 run "scratch/benchmarktest --algorithm=mmgpsr --size=20 --time=30 --AppsOn=true"
```

### 标准性能测试 (中等规模)

```bash
# PA-GPSR 性能测试 (50节点, 200秒)
./ns3 run "scratch/benchmarktest --algorithm=pagpsr --size=50 --time=200 --MinSpeed=20 --MaxSpeed=60"
```

### 批量对比测试

创建测试脚本 `test_all_protocols.sh`:

```bash
#!/bin/bash
NS3_DIR=~/ns-allinone-3.40/ns-3.40
cd $NS3_DIR

PROTOCOLS=("pagpsr" "gpsr" "mmgpsr" "aodv" "olsr")
SIZES=(20 30 50)
TIME=100

for proto in "${PROTOCOLS[@]}"; do
    for size in "${SIZES[@]}"; do
        echo "========================================"
        echo "Testing: $proto with $size nodes"
        echo "========================================"
        ./ns3 run "scratch/benchmarktest --algorithm=$proto --size=$size --time=$TIME --seed=2025"
        echo ""
    done
done

echo "All tests completed!"
echo "Results saved in: benchmark_tests_*nodes.csv"
```

运行：
```bash
chmod +x test_all_protocols.sh
./test_all_protocols.sh
```

## 🔍 预期输出

### 成功的运行输出

```
Creating 20 nodes, Algorithm: pagpsr
PA-GPSR Install() completed
Simulation Starts, TotalTime: 30 s ...
[仿真运行...]
性能指标已保存至: benchmark_tests_20nodes.csv
```

### 性能指标文件

生成的 CSV 文件应该包含：

| 指标 | 说明 |
|------|------|
| PDR | 数据包投递率 (%) |
| avgDelay | 平均端到端延迟 (ms) |
| avgHops | 平均跳数 |
| throughput | 吞吐量 (kbps) |
| controlOverheadRatio | 控制开销比 |
| avgEnergyPerNode | 平均每节点能耗 (J) |
| deadNodes | 能量耗尽节点数 |

## ❌ 常见错误及解决

### 错误 1: 找不到模块

**症状**:
```
fatal error: ns3/pagpsr-module.h: No such file or directory
```

**解决**:
```bash
# 确认模块已复制
ls contrib/pagpsr/CMakeLists.txt
ls contrib/gpsr/CMakeLists.txt
ls contrib/mmgpsr/CMakeLists.txt
ls contrib/location-service/CMakeLists.txt

# 重新配置和编译
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

### 错误 2: Segmentation fault

**症状**:
```
Segmentation fault (core dumped)
```

**可能原因**:
1. 忘记调用 Install()
2. Install() 调用顺序错误

**解决**:
- 确认在 Run() 方法中添加了 Install() 调用
- 确保 Install() 在 InstallInternetStack() **之后**调用

### 错误 3: TypeId 未注册

**症状**:
```
Aborted (TypeId not found: ns3::pagpsr::RoutingProtocol)
```

**解决**:
```bash
# 检查模块是否正确编译
./ns3 show modules | grep pagpsr

# 如果没有显示，重新编译
./ns3 build
```

### 错误 4: 没有数据传输

**症状**:
- 仿真运行完成
- 但 PDR = 0%, RxPackets = 0

**可能原因**:
- **最常见**: 忘记调用 Install()，导致 UDP 回调未设置
- 节点间距离过大，超出通信范围
- 应用程序未启动

**解决**:
1. 检查是否添加了 Install() 调用
2. 检查 MaxRange 设置 (默认500m)
3. 确认 `--AppsOn=true`

### 错误 5: 编译错误 - WifiMac 相关

**症状**:
```
error: 'AdhocWifiMac' was not declared in this scope
```

**解决**:
这是 ns-3.40 的正常行为。benchmarktest.cc 中的用法是正确的：
```cpp
wifiMac.SetType ("ns3::AdhocWifiMac");  // 这是正确的
```

确保不要在代码中显式使用 `Ptr<AdhocWifiMac>`。

## 📊 性能对比参考

### 预期性能范围 (50节点, 200秒)

| 协议 | PDR (%) | 平均延迟 (ms) | 平均跳数 | 控制开销 |
|------|---------|---------------|----------|----------|
| PA-GPSR | 80-95 | 50-150 | 3-5 | 低 |
| GPSR | 75-90 | 60-180 | 3-5 | 低 |
| MM-GPSR | 78-92 | 55-170 | 3-5 | 低-中 |
| AODV | 70-85 | 80-200 | 3-6 | 中 |
| OLSR | 75-88 | 100-250 | 3-5 | 高 |

**注意**: 实际值取决于：
- 节点密度
- 移动速度
- 流量负载
- 信道模型

## 📝 下一步建议

### 1. 验证基本功能
先用小规模 (10-20节点) 短时间 (30-50秒) 测试，确保协议正常工作。

### 2. 性能基准测试
使用标准配置 (50节点, 200秒) 收集性能数据。

### 3. 参数敏感性分析
- 节点数量: 20, 30, 40, 50, 100
- 移动速度: 低速(10-20 m/s), 中速(20-40 m/s), 高速(40-60 m/s)
- 流量负载: 不同的连接对数

### 4. 协议对比分析
在相同条件下比较 PA-GPSR、GPSR、MM-GPSR、AODV、OLSR 的性能。

## 🎯 成功标准

PA-GPSR 成功集成的标志：

✅ 编译无错误
✅ 运行时无 segfault
✅ 能看到 "PA-GPSR Install() completed" 消息
✅ 有数据包成功传输 (RxPackets > 0)
✅ PDR > 0% (理想情况 > 70%)
✅ 生成了性能指标 CSV 文件

## 📞 遇到问题？

如果遇到问题，请检查：

1. ✅ 是否按照 MIGRATION_TO_NS3.40.md 正确安装了模块？
2. ✅ 是否在 Run() 中添加了 Install() 调用？
3. ✅ benchmarktest.cc 是否包含了必要的头文件？
4. ✅ ns-3.40 是否正确配置和编译？
5. ✅ 是否使用了正确的命令行参数？

**调试提示**:
- 启用日志: `NS_LOG="PAGpsrRoutingProtocol=level_all" ./ns3 run ...`
- 生成 PCAP: `--pcap=true`
- 打印路由表: `--printRoutes=true`
