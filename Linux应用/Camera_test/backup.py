from flask import Flask, Response
import cv2
import numpy as np
import threading

app = Flask(__name__)

# 初始化摄像头
camera = cv2.VideoCapture(0)  # 使用默认摄像头

# 定义颜色阈值（HSV格式）
lower_red = np.array([0, 100, 100])
upper_red = np.array([10, 255, 255])

lower_green = np.array([40, 40, 40])
upper_green = np.array([80, 255, 255])

lower_blue = np.array([100, 150, 0])
upper_blue = np.array([140, 255, 255])

# 存储视频帧的全局变量
output_frame = None
lock = threading.Lock()  # 线程锁

def process_camera_feed():
    global output_frame

    while True:
        success, frame = camera.read()  # 从摄像头读取帧
        if not success:
            break

        # 存储圆心坐标的列表
        circle_centers = {'red': None, 'green': None, 'blue': None}

        # 转换为HSV格式
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        # 创建掩膜并检测红、绿、蓝色的圆环
        mask_red = cv2.inRange(hsv, lower_red, upper_red)
        mask_green = cv2.inRange(hsv, lower_green, upper_green)
        mask_blue = cv2.inRange(hsv, lower_blue, upper_blue)

        # 使用HoughCircles检测圆环
        circles_red = cv2.HoughCircles(mask_red, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                       param1=50, param2=30, minRadius=10, maxRadius=100)
        circles_green = cv2.HoughCircles(mask_green, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                         param1=50, param2=30, minRadius=10, maxRadius=100)
        circles_blue = cv2.HoughCircles(mask_blue, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                        param1=50, param2=30, minRadius=10, maxRadius=100)

        # 保存检测到的最大红色圆环坐标
        if circles_red is not None:
            circles_red = np.round(circles_red[0, :]).astype("int")
            max_red_circle = max(circles_red, key=lambda c: c[2])  # 根据半径c[2]找到最大的圆
            (x, y, r) = max_red_circle
            circle_centers['red'] = (x, y)
            cv2.circle(frame, (x, y), r, (0, 0, 255), 4)
            cv2.circle(frame, (x, y), 2, (0, 0, 255), 3)  # 标记中心点

        # 保存检测到的最大绿色圆环坐标
        if circles_green is not None:
            circles_green = np.round(circles_green[0, :]).astype("int")
            max_green_circle = max(circles_green, key=lambda c: c[2])  # 根据半径c[2]找到最大的圆
            (x, y, r) = max_green_circle
            circle_centers['green'] = (x, y)
            cv2.circle(frame, (x, y), r, (0, 255, 0), 4)
            cv2.circle(frame, (x, y), 2, (0, 255, 0), 3)  # 标记中心点

        # 保存检测到的最大蓝色圆环坐标
        if circles_blue is not None:
            circles_blue = np.round(circles_blue[0, :]).astype("int")
            max_blue_circle = max(circles_blue, key=lambda c: c[2])  # 根据半径c[2]找到最大的圆
            (x, y, r) = max_blue_circle
            circle_centers['blue'] = (x, y)
            cv2.circle(frame, (x, y), r, (255, 0, 0), 4)
            cv2.circle(frame, (x, y), 2, (255, 0, 0), 3)  # 标记中心点

        # 打印当前帧中检测到的最大圆心坐标
        # print(circle_centers)
        # cv2.imshow("biu", frame)
        # 更新全局帧变量（线程安全）
        with lock:
            output_frame = frame.copy()

def generate_frames():
    global output_frame
    while True:
        with lock:
            if output_frame is None:
                continue
            ret, buffer = cv2.imencode('.jpg', output_frame)
            frame = buffer.tobytes()
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')  # 返回JPEG帧

@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    # 启动一个线程处理摄像头
    t = threading.Thread(target=process_camera_feed)
    t.daemon = True
    t.start()

    # 启动Flask服务，访问 http://<服务器IP>:5000/video_feed 查看识别结果
    app.run(host='0.0.0.0', port=5000)