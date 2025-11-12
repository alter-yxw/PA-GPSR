#!/bin/bash
# PA-GPSR 安装脚本 - 将模块复制到 ns-3.40

# 使用方法:
#   ./install-to-ns3.sh /path/to/ns-3.40
# 例如:
#   ./install-to-ns3.sh ~/ns-allinone-3.40/ns-3.40

if [ -z "$1" ]; then
    echo "错误: 请提供 ns-3.40 安装路径"
    echo "使用方法: $0 <ns-3.40路径>"
    echo "例如: $0 ~/ns-allinone-3.40/ns-3.40"
    exit 1
fi

NS3_DIR="$1"

# 检查 ns-3.40 目录是否存在
if [ ! -d "$NS3_DIR" ]; then
    echo "错误: ns-3.40 目录不存在: $NS3_DIR"
    exit 1
fi

# 检查 contrib 目录
if [ ! -d "$NS3_DIR/contrib" ]; then
    echo "错误: contrib 目录不存在: $NS3_DIR/contrib"
    echo "请确认这是一个有效的 ns-3.40 安装目录"
    exit 1
fi

REPO_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "======================================"
echo "PA-GPSR 安装到 ns-3.40"
echo "======================================"
echo "仓库目录: $REPO_DIR"
echo "ns-3目录: $NS3_DIR"
echo ""

# 复制模块
echo "正在复制模块..."

echo "  - location-service"
cp -r "$REPO_DIR/src/location-service" "$NS3_DIR/contrib/"

echo "  - pagpsr"
cp -r "$REPO_DIR/src/pagpsr" "$NS3_DIR/contrib/"

echo "  - gpsr"
cp -r "$REPO_DIR/src/gpsr" "$NS3_DIR/contrib/"

echo "  - mmgpsr"
cp -r "$REPO_DIR/src/mmgpsr" "$NS3_DIR/contrib/"

echo ""
echo "✓ 所有模块已复制到 $NS3_DIR/contrib/"
echo ""
echo "下一步:"
echo "  1. cd $NS3_DIR"
echo "  2. ./ns3 configure --enable-examples --enable-tests"
echo "  3. ./ns3 build"
echo ""
