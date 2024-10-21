from flask import Flask, Response, render_template, request
import cv2
import numpy as np
import threading
import serial  # 导入串口通信库
import time  # 导入时间库
from queue import Queue
app = Flask(__name__)

# 初始化摄像头
camera = cv2.VideoCapture(0)  # 使用默认摄像头
camera.set(cv2.CAP_PROP_FRAME_WIDTH, 1024)
camera.set(cv2.CAP_PROP_FRAME_HEIGHT, 720)

camera2 = cv2.VideoCapture(2)  # 使用第二个摄像头
camera2.set(cv2.CAP_PROP_FRAME_WIDTH,  800)
camera2.set(cv2.CAP_PROP_FRAME_HEIGHT, 800)

# 定义颜色阈值（HSV格式）
thresholds = {
    'red': {'lower': np.array([0, 20, 20]), 'upper': np.array([40, 255, 234])},
    'green': {'lower': np.array([40,40,40]), 'upper': np.array([80,255,255])},
    'blue': {'lower': np.array([34, 90, 70]), 'upper': np.array([100, 255, 255])}
}

# 定义色块检测的阈值
block_thresholds = {
    'red': {'lower': np.array([0, 100, 100]), 'upper': np.array([10, 255, 255])},
    'green': {'lower': np.array([40, 100, 100]), 'upper': np.array([80, 255, 255])},
    'blue': {'lower': np.array([100, 150, 0]), 'upper': np.array([140, 255, 255])}
}

# 定义腐蚀内核
kernel = np.ones((1, 1), np.uint8)

# 存储视频帧的全局变量
output_frame = None
lock = threading.Lock()  # 线程锁

task = 'block'
data_queue = Queue()

def process_camera_feed():
    min_radius = 10
    max_radius = 200

    min_area = 500  # 最小面积阈值
    max_area = 50000  # 最大面积阈值

    global output_frame

    while True:
        success, frame = camera.read()  # 从摄像头读取帧
        if not success:
            break
        
        # 创建彩色图像的副本用于标注
        annotated_frame = frame.copy()
        mask_red = None
        mask_green = None
        mask_blue = None
        # 转换为HSV格式
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        if task == 'circle':
            # 使用动态阈值创建掩膜（色环检测）
            mask_red = cv2.inRange(hsv, thresholds['red']['lower'], thresholds['red']['upper'])
            mask_red = cv2.erode(mask_red, kernel, iterations=1)

            mask_green = cv2.inRange(hsv, thresholds['green']['lower'], thresholds['green']['upper'])
            mask_green = cv2.erode(mask_green, kernel, iterations=1)

            mask_blue = cv2.inRange(hsv, thresholds['blue']['lower'], thresholds['blue']['upper'])
            mask_blue = cv2.erode(mask_blue, kernel, iterations=1)

            # 对每个颜色进行圆形检测
            circles_red = cv2.HoughCircles(mask_red, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                           param1=50, param2=80, minRadius=10, maxRadius=200)
            circles_green = cv2.HoughCircles(mask_green, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                             param1=50, param2=80, minRadius=10, maxRadius=200)
            circles_blue = cv2.HoughCircles(mask_blue, cv2.HOUGH_GRADIENT, dp=1.2, minDist=30,
                                            param1=50, param2=80, minRadius=10, maxRadius=200)

            # 查找最大圆形的通用函数
            def find_largest_circle(circles):
                if circles is not None:
                    circles = np.round(circles[0, :]).astype("int")
                    largest_circle = max(circles, key=lambda x: x[2])  # 根据半径进行排序
                    return tuple(largest_circle)  # 返回 (x, y, r) 作为元组
                return None

            # 标注红色最大圆形并打包坐标和半径
            largest_red_circle = find_largest_circle(circles_red)
            if largest_red_circle is not None:
                (x, y, r) = largest_red_circle
                cv2.circle(annotated_frame, (x, y), r, (0, 0, 255), 4)
                cv2.circle(annotated_frame, (x, y), 2, (0, 0, 255), 3)  # 标记中心点
                print(f"Red Circle - {(x, y, r)}")
                data_queue.put(f"Red Circle - {(x, y, r)}")

            # 标注绿色最大圆形
            largest_green_circle = find_largest_circle(circles_green)
            if largest_green_circle is not None:
                (x, y, r) = largest_green_circle
                cv2.circle(annotated_frame, (x, y), r, (0, 255, 0), 4)
                cv2.circle(annotated_frame, (x, y), 2, (0, 255, 0), 3)  # 标记中心点
                print(f"Green Circle - {(x, y, r)}")
                data_queue.put(f"Green Circle - {(x, y, r)}")

            # 标注蓝色最大圆形
            largest_blue_circle = find_largest_circle(circles_blue)
            if largest_blue_circle is not None:
                (x, y, r) = largest_blue_circle
                cv2.circle(annotated_frame, (x, y), r, (255, 0, 0), 4)
                cv2.circle(annotated_frame, (x, y), 2, (255, 0, 0), 3)  # 标记中心点
                print(f"Blue Circle - {(x, y, r)}")
                data_queue.put(f"Blue Circle - {(x, y, r)}")


        elif task == 'block':

            # 使用动态阈值创建掩膜（色块检测）

            mask_red = cv2.inRange(hsv, block_thresholds['red']['lower'], block_thresholds['red']['upper'])

            mask_green = cv2.inRange(hsv, block_thresholds['green']['lower'], block_thresholds['green']['upper'])

            mask_blue = cv2.inRange(hsv, block_thresholds['blue']['lower'], block_thresholds['blue']['upper'])

            # 对每个颜色进行色块检测

            def find_largest_contour(mask, min_area, max_area):
                contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
                largest_contour = None
                max_area_found = 0  # 局部变量来存储最大面积
                for contour in contours:
                    area = cv2.contourArea(contour)
                    # 检查最小和最大面积阈值
                    if min_area < area < max_area:
                        if area > max_area_found:  # 更新最大轮廓
                            max_area_found = area
                            largest_contour = contour
                return largest_contour

            min_area = 500  # 定义最小面积阈值
            # 标注最大红色色块
            largest_red_contour = find_largest_contour(mask_red, min_area, max_area)
            if largest_red_contour is not None:
                x, y, w, h = cv2.boundingRect(largest_red_contour)
                cv2.rectangle(annotated_frame, (x, y), (x + w, y + h), (0, 0, 255), 2)
                center = (x + w // 2, y + h // 2)
                print(f"Red Block - Center: {center}, Width: {w}, Height: {h}")
                data_queue.put(f"Red Block - Center: {(x + w // 2, y + h // 2)}, Width: {w}, Height: {h}")
            # 标注最大绿色色块
            largest_green_contour = find_largest_contour(mask_green, min_area, max_area)
            if largest_green_contour is not None:
                x, y, w, h = cv2.boundingRect(largest_green_contour)
                cv2.rectangle(annotated_frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
                center = (x + w // 2, y + h // 2)
                print(f"Green Block - Center: {center}, Width: {w}, Height: {h}")
                data_queue.put(f"Green Block - Center: {(x + w // 2, y + h // 2)}, Width: {w}, Height: {h}")
            # 标注最大蓝色色块
            largest_blue_contour = find_largest_contour(mask_blue, min_area, max_area)
            if largest_blue_contour is not None:
                x, y, w, h = cv2.boundingRect(largest_blue_contour)
                cv2.rectangle(annotated_frame, (x, y), (x + w, y + h), (255, 0, 0), 2)
                center = (x + w // 2, y + h // 2)
                print(f"Blue Block - Center: {center}, Width: {w}, Height: {h}")
                data_queue.put(f"Blue Block - Center: {(x + w // 2, y + h // 2)}, Width: {w}, Height: {h}")

            # 更新全局帧变量（线程安全）
        with lock:
            output_frame = {
                'color': annotated_frame,
                'red': mask_red,
                'green': mask_green,
                'blue': mask_blue
            }
# 定义全局变量用于控制二维码扫描线程
qr_code_scanning_enabled = True
def process_qr_code():
    global qr_code_scanning_enabled  # 引用全局变量
    qr_detector = cv2.QRCodeDetector()  # 创建 QRCodeDetector 实例
    while True:
        if not qr_code_scanning_enabled:  # 检查是否允许扫描
            continue  # 如果不允许，继续下一次循环

        success, frame = camera2.read()  # 从第二个摄像头读取帧
        if not success:
            continue  # 如果读取失败，继续下一次循环

        # 使用 detectAndDecode 方法检测 QR 码
        data, bbox, _ = qr_detector.detectAndDecode(frame)

        if bbox is not None and data:
            # 确保 bbox 是整数并绘制多边形
            bbox = bbox.astype(int)
            cv2.polylines(frame, [bbox], isClosed=True, color=(0, 255, 0), thickness=2)
            print(f"Detected QR code: {data}")
            data_queue.put(f"Detected QR code: {data}")

        # 更新全局帧变量（线程安全）
        with lock:
            if output_frame is not None:  # 检查 output_frame 是否已初始化
                output_frame['qrcode'] = frame
            else:
                print("Warning: output_frame is None, skipping QR code update.")

def process_serial_communication():
    ser = serial.Serial('COM3', 9600)  # 根据需要修改串口号和波特率
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').rstrip()
            # 根据串口接收到的命令修改task和qr_code_scanning_enabled
            if line == 'switch_to_block':
                task = 'block'
            elif line == 'switch_to_circle':
                task = 'circle'
            elif line == 'enable_qr_scanning':
                qr_code_scanning_enabled = True  # 启用二维码扫描
            elif line == 'disable_qr_scanning':
                qr_code_scanning_enabled = False  # 禁用二维码扫描
            print(f"Received from serial: {line}")
        # 从队列中获取数据并通过串口发送
        if not data_queue.empty():
            data_to_send = data_queue.get()
            ser.write(data_to_send.encode('utf-8'))
            print(f"Sent to serial: {data_to_send}")
        time.sleep(0.1)  # 暂停以降低CPU使用率

def generate_frames(color):
    global output_frame
    while True:
        with lock:
            if output_frame is None or color not in output_frame:
                continue
            # 根据请求的颜色编码图像
            try:
                ret, buffer = cv2.imencode('.jpg', output_frame[color])
                frame = buffer.tobytes()
            except KeyError:
                continue  # 如果键不存在，继续下一次循环

        # 返回单帧
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/video_feed/color')
def video_feed_color():
    return Response(generate_frames('color'),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed/red')
def video_feed_red():
    return Response(generate_frames('red'),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed/green')
def video_feed_green():
    return Response(generate_frames('green'),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/video_feed/blue')
def video_feed_blue():
    return Response(generate_frames('blue'),
                    mimetype='multipart/x-mixed-replace; boundary=frame')
@app.route('/video_feed/qrcode')
def video_feed_qrcode():
    return Response(generate_frames('qrcode'),
                    mimetype='multipart/x-mixed-replace; boundary=frame')

@app.route('/update_thresholds', methods=['POST'])
def update_thresholds():
    global thresholds
    thresholds['red']['lower'] = np.array([int(request.form.get('red_lower_h')),
                                           int(request.form.get('red_lower_s')),
                                           int(request.form.get('red_lower_v'))])
    thresholds['red']['upper'] = np.array([int(request.form.get('red_upper_h')),
                                           int(request.form.get('red_upper_s')),
                                           int(request.form.get('red_upper_v'))])

    thresholds['green']['lower'] = np.array([int(request.form.get('green_lower_h')),
                                             int(request.form.get('green_lower_s')),
                                             int(request.form.get('green_lower_v'))])
    thresholds['green']['upper'] = np.array([int(request.form.get('green_upper_h')),
                                             int(request.form.get('green_upper_s')),
                                             int(request.form.get('green_upper_v'))])

    thresholds['blue']['lower'] = np.array([int(request.form.get('blue_lower_h')),
                                            int(request.form.get('blue_lower_s')),
                                            int(request.form.get('blue_lower_v'))])
    thresholds['blue']['upper'] = np.array([int(request.form.get('blue_upper_h')),
                                            int(request.form.get('blue_upper_s')),
                                            int(request.form.get('blue_upper_v'))])

    return 'Thresholds updated successfully!'

if __name__ == '__main__':
    # 启动一个线程处理色环和色块
    t = threading.Thread(target=process_camera_feed)
    t.daemon = True
    t.start()

    # 启动一个线程处理二维码    
    t2 = threading.Thread(target=process_qr_code)
    t2.daemon = True
    t2.start()

    # 启动一个线程处理串口数据
    # t3 = threading.Thread(target=process_serial_communication)
    # t3.daemon = True
    # t3.start()
    
    #启动Flask服务，访问 http://<服务器IP>:5000 查看识别结果
    app.run(host='0.0.0.0', port=5000)
