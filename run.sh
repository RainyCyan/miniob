#!/bin/bash

# 定义颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 获取当前目录
CURRENT_DIR=$(basename "$PWD")

# 检查是否在正确的目录下执行
# if [[ "$CURRENT_DIR" != "build" || "$CURRENT_DIR" != "build_debug" || "$CURRENT_DIR" != "build_release" ]]; then
#     echo -e "${RED}错误: 请在 build_debug 或 build_release 目录下执行此脚本${NC}"
#     exit 1
# fi

# 显示用法信息
usage() {
    echo -e "${YELLOW}用法: $0 [选项]${NC}"
    echo "选项:"
    echo "  -c, --cli      使用CLI模式启动 (方式1)"
    echo "  -p, --port     使用端口模式启动 (方式2)"
    echo "  -s, --socket   使用socket模式启动 (方式3)"
    echo "  -h, --help     显示帮助信息"
    exit 1
}

# 检查参数数量
if [ $# -ne 1 ]; then
    usage
fi

# 处理参数
case "$1" in
    -c|--cli)
        echo -e "${GREEN}启动CLI模式...${NC}"
        ./bin/observer -f ../etc/observer.ini -P cli
        ;;
    -p|--port)
        echo -e "${GREEN}启动端口模式...${NC}"
        # 启动observer
        ./bin/observer -f ../etc/observer.ini -p 6789 &
        OBSERVER_PID=$!
        echo "Observer 启动成功，PID: $OBSERVER_PID"
        
        # 等待一下让observer完全启动
        sleep 2
        
        # 启动obclient
        echo "启动obclient连接..."
        ./bin/obclient -p 6789
        
        # 当obclient退出时，终止observer
        kill $OBSERVER_PID
        echo "Observer 已停止"
        ;;
    -s|--socket)
        echo -e "${GREEN}启动socket模式...${NC}"
        # 启动observer
        ./bin/observer -f ../etc/observer.ini -s miniob.sock &
        OBSERVER_PID=$!
        echo "Observer 启动成功，PID: $OBSERVER_PID"
        
        # 等待一下让observer完全启动
        sleep 2
        
        # 启动obclient
        echo "启动obclient连接..."
        ./bin/obclient -s miniob.sock
        
        # 当obclient退出时，终止observer
        kill $OBSERVER_PID
        echo "Observer 已停止"
        ;;
    -h|--help)
        usage
        ;;
    *)
        echo -e "${RED}错误: 未知选项 $1${NC}"
        usage
        ;;
esac