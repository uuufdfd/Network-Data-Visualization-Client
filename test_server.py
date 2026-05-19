import socket
import json
import time
import random

HOST = "0.0.0.0"
PORT = 8080

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((HOST, PORT))
server.listen(1)

print(f"测试服务器已启动：{HOST}:{PORT}")
print("等待客户端连接...")

while True:
    conn, addr = server.accept()
    print("客户端已连接：", addr)

    try:
        while True:
            value1 = round(random.uniform(20, 35), 2)
            value2 = round(random.uniform(40, 85), 2)

            if value1 > 32 or value2 > 78:
                status = "warning"
            else:
                status = "normal"

            # 偶尔模拟错误状态
            if random.random() < 0.03:
                status = "error"

            data = {
                "timestamp": time.time(),
                "value1": value1,
                "value2": value2,
                "status": status
            }

            message = json.dumps(data, ensure_ascii=False) + "\n"
            conn.sendall(message.encode("utf-8"))
            print(message.strip())

            time.sleep(1)

    except (ConnectionResetError, BrokenPipeError):
        print("客户端断开，继续等待新连接...")
    finally:
        conn.close()
