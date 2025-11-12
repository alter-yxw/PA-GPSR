# PA-GPSR 快速测试指南

## 快速验证步骤

### 1. 一键安装脚本

创建这个脚本来快速安装模块:

```bash
#!/bin/bash
# install-to-ns3.sh

# 设置路径
NS3_DIR="$HOME/ns-allinone-3.40/ns-3.40"  # 修改为你的 ns-3.40 路径
PA_GPSR_DIR="$(pwd)"

# 检查 ns-3 是否存在
if [ ! -d "$NS3_DIR" ]; then
    echo "错误: 找不到 ns-3.40 目录: $NS3_DIR"
    echo "请修改脚本中的 NS3_DIR 变量"
    exit 1
fi

echo "正在安装 PA-GPSR 模块到 $NS3_DIR ..."

# 复制模块
echo "复制模块..."
cp -r "$PA_GPSR_DIR/src/location-service" "$NS3_DIR/contrib/"
cp -r "$PA_GPSR_DIR/src/pagpsr" "$NS3_DIR/contrib/"
cp -r "$PA_GPSR_DIR/src/gpsr" "$NS3_DIR/contrib/"
cp -r "$PA_GPSR_DIR/src/mmgpsr" "$NS3_DIR/contrib/"

# 复制示例
echo "复制示例程序..."
cp "$PA_GPSR_DIR/examples/pagpsr-main.cc" "$NS3_DIR/scratch/"

# 构建
cd "$NS3_DIR"
echo "配置 ns-3..."
./ns3 configure --enable-examples --enable-tests

echo "构建 ns-3..."
./ns3 build

# 验证
echo ""
echo "验证安装..."
echo "已安装的模块:"
./ns3 show modules | grep -E "location-service|pagpsr|gpsr|mmgpsr"

echo ""
echo "安装完成!"
echo ""
echo "运行测试:"
echo "  cd $NS3_DIR"
echo "  ./ns3 run 'scratch/pagpsr-main --algorithm=pagpsr --size=10 --time=10'"
```

### 2. 快速功能测试

```bash
# 进入 ns-3 目录
cd /path/to/ns-3.40

# 测试 1: PA-GPSR 快速测试 (10节点, 10秒)
./ns3 run "scratch/pagpsr-main --algorithm=pagpsr --size=10 --time=10 --conn=3"

# 测试 2: GPSR 快速测试
./ns3 run "scratch/pagpsr-main --algorithm=gpsr --size=10 --time=10 --conn=3"

# 测试 3: MM-GPSR 快速测试
./ns3 run "scratch/pagpsr-main --algorithm=mmgpsr --size=10 --time=10 --conn=3"
```

### 3. 预期输出

成功运行时，您应该看到类似的输出:

```
Creating  10 nodes .. with 3 pairs...
Using PA-GPSR algorithm...
Starting simulation for 10 s ...
Starting simulation for speed 15 ms ...
The simulation is now at: 10 seconds
Output operation successfully performed1
```

### 4. 检查结果文件

```bash
# 检查生成的结果文件
ls -lh results/pagpsr_results/pairs3/
cat results/pagpsr_results/pairs3/pagpsr10_results.txt
```

## 常见错误及解决

### 错误 1: 找不到模块
```
Error: Module 'pagpsr' not found
```
**解决**: 确认模块已复制到 contrib/ 目录，并重新运行 `./ns3 configure`

### 错误 2: 编译错误
```
fatal error: ns3/pagpsr-module.h: No such file or directory
```
**解决**:
```bash
./ns3 clean
./ns3 configure --enable-examples
./ns3 build
```

### 错误 3: 链接错误
```
undefined reference to `ns3::pagpsr::RoutingProtocol::GetTypeId()'
```
**解决**: CMakeLists.txt 可能有问题，检查源文件列表是否完整

## 性能基准测试

如果要进行性能测试，推荐配置:

```bash
# 小规模 (10 节点)
./ns3 run "scratch/pagpsr-main --algorithm=pagpsr --size=10 --time=50 --conn=5"

# 中等规模 (30 节点)
./ns3 run "scratch/pagpsr-main --algorithm=pagpsr --size=30 --time=100 --conn=10"

# 大规模 (50 节点)
./ns3 run "scratch/pagpsr-main --algorithm=pagpsr --size=50 --time=200 --conn=15"
```

## 批量测试脚本

```bash
#!/bin/bash
# batch-test.sh - 批量测试所有协议

NS3_DIR="/path/to/ns-3.40"
cd "$NS3_DIR"

ALGORITHMS=("pagpsr" "gpsr" "mmgpsr")
SIZES=(10 20 30)
TIME=50
CONNECTIONS=10

for algo in "${ALGORITHMS[@]}"; do
    for size in "${SIZES[@]}"; do
        echo "Testing $algo with $size nodes..."
        ./ns3 run "scratch/pagpsr-main --algorithm=$algo --size=$size --time=$TIME --conn=$CONNECTIONS"
        echo "---"
    done
done

echo "All tests completed!"
```

## 下一步

如果基本测试通过，您可以:

1. ✅ 确认协议正常工作
2. 📝 创建自定义的 benchmarktest.cc
3. 📊 设置更复杂的测试场景
4. 📈 收集和分析性能数据
