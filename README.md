# 网络数据可视化客户端

## 一、项目简介

本项目是基于 Qt Widgets、QTcpSocket 和 Qt Charts 开发的 TCP 网络数据可视化客户端。客户端可以连接数据服务器，实时接收 JSON 数据流，解析温度、湿度和状态字段，并通过折线图、表格和状态指示灯进行展示。

系统会自动将接收到的数据保存为 CSV 文件，文件按日期命名，例如 `data_20260518.csv`。

## 二、功能说明

1. 网络连接
   - 可配置服务器 IP 和端口
   - 支持连接和断开
   - 显示连接状态
   - 支持断线自动重连

2. 数据接收与解析
   - 使用 QTcpSocket 接收 TCP 数据
   - 每条数据使用 JSON 格式
   - 解析 timestamp、value1、value2、status 字段

3. 数据可视化
   - 使用 Qt Charts 实时折线图展示 value1 和 value2
   - 使用表格显示最新 20 条数据
   - 使用状态指示灯显示 normal、warning、error

4. 数据存储
   - 自动保存到 CSV 文件
   - 支持手动导出当前数据

5. 控制功能
   - 开始接收
   - 暂停接收
   - 清空显示
   - 设置刷新频率 0.5 秒到 5 秒

## 三、运行环境

- Windows 10 / Windows 11
- Qt Creator
- Qt 5.15 或 Qt 6.x
- 需要 Qt Charts 模块
- Python 3.x 用于运行测试服务器

## 四、编译说明

确保 `.pro` 文件包含：

```pro
QT += core gui widgets network charts
```

然后使用 Qt Creator 打开项目，构建并运行。

## 五、测试服务器使用方法

在项目目录打开命令行，运行：

```bash
python test_server.py
```

看到：

```text
测试服务器已启动：0.0.0.0:8080
等待客户端连接...
```

然后在客户端中填写：

```text
服务器地址：127.0.0.1
端口：8080
```

点击“连接”即可。

## 六、JSON 数据格式

```json
{
  "timestamp": 1779080000.123,
  "value1": 25.5,
  "value2": 65.2,
  "status": "normal"
}
```

## 七、CSV 文件说明

自动保存路径：

```text
程序运行目录/data/data_yyyyMMdd.csv
```

字段：

```text
timestamp,value1,value2,status
```

## 八、核心类说明

- DataRecord：数据记录结构，负责 JSON 转换和 CSV 行生成
- MainWindow：主窗口，负责网络连接、数据解析、图表刷新、表格显示和文件保存
- QTcpSocket：负责 TCP 网络通信
- QChart / QLineSeries：负责实时折线图显示
