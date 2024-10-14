import cv2
import numpy as np

# 定义颜色范围 (BGR)
lower_red = np.array([0, 0, 100])
upper_red = np.array([80, 80, 255])

lower_green = np.array([0, 100, 0])
upper_green = np.array([80, 255, 80])

lower_blue = np.array([100, 0, 0])
upper_blue = np.array([255, 80, 80])

# 定义腐蚀内核
kernel = np.ones((5, 5), np.uint8)

# 捕获摄像头内容
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # 转换为HSV图像
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # 红色遮罩
    mask_red = cv2.inRange(frame, lower_red, upper_red)
    mask_red = cv2.erode(mask_red, kernel, iterations=1)

    # 绿色遮罩
    mask_green = cv2.inRange(frame, lower_green, upper_green)
    mask_green = cv2.erode(mask_green, kernel, iterations=1)

    # 蓝色遮罩
    mask_blue = cv2.inRange(frame, lower_blue, upper_blue)
    mask_blue = cv2.erode(mask_blue, kernel, iterations=1)

    # 检测红色的圆形
    circles_red = cv2.HoughCircles(mask_red, cv2.HOUGH_GRADIENT, dp=1, minDist=50,
                                   param1=50, param2=30, minRadius=5, maxRadius=100)
    if circles_red is not None:
        circles_red = np.uint16(np.around(circles_red))
        for i in circles_red[0, :]:
            cv2.circle(frame, (i[0], i[1]), i[2], (0, 0, 255), 2)  # 画出外圆
            cv2.circle(frame, (i[0], i[1]), 2, (0, 0, 255), 3)  # 画出圆心

    # 检测绿色的圆形
    circles_green = cv2.HoughCircles(mask_green, cv2.HOUGH_GRADIENT, dp=1, minDist=50,
                                     param1=50, param2=30, minRadius=5, maxRadius=100)
    if circles_green is not None:
        circles_green = np.uint16(np.around(circles_green))
        for i in circles_green[0, :]:
            cv2.circle(frame, (i[0], i[1]), i[2], (0, 255, 0), 2)  # 画出外圆
            cv2.circle(frame, (i[0], i[1]), 2, (0, 255, 0), 3)  # 画出圆心

    # 检测蓝色的圆形
    circles_blue = cv2.HoughCircles(mask_blue, cv2.HOUGH_GRADIENT, dp=1, minDist=50,
                                    param1=50, param2=30, minRadius=5, maxRadius=100)
    if circles_blue is not None:
        circles_blue = np.uint16(np.around(circles_blue))
        for i in circles_blue[0, :]:
            cv2.circle(frame, (i[0], i[1]), i[2], (255, 0, 0), 2)  # 画出外圆
            cv2.circle(frame, (i[0], i[1]), 2, (255, 0, 0), 3)  # 画出圆心

    # 显示结果
    cv2.imshow("Original", frame)
    cv2.imshow("Red Mask", mask_red)
    cv2.imshow("Green Mask", mask_green)
    cv2.imshow("Blue Mask", mask_blue)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
