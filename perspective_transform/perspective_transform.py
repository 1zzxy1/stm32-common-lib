from maix import image, display, app, time, camera
import cv2
import numpy as np
import math
from micu_uart_lib import (
    SimpleUART, micu_printf
)

# --------------------------- 紫色激光检测器类 ---------------------------
class PurpleLaserDetector:
    def __init__(self, pixel_radius=3):
        self.pixel_radius = pixel_radius
        self.kernel = np.ones((3, 3), np.uint8)
        
    def detect(self, img):
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        lower_purple = np.array([130, 80, 80])
        upper_purple = np.array([160, 255, 255])
        mask_purple = cv2.inRange(hsv, lower_purple, upper_purple)
        mask_purple = cv2.morphologyEx(mask_purple, cv2.MORPH_CLOSE, self.kernel)
        
        contours_purple, _ = cv2.findContours(mask_purple, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        laser_points = []
        
        for cnt in contours_purple:
            rect = cv2.minAreaRect(cnt)
            cx, cy = map(int, rect[0])
            laser_points.append((cx, cy))
            cv2.circle(img, (cx, cy), 3, (255, 0, 255), -1)
            cv2.putText(img, "Laser", (cx-20, cy-10),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.3, (255, 0, 255), 1)
        
        return img, laser_points

# --------------------------- 圆形轨迹点生成函数 ---------------------------
def generate_circle_points(center, radius, num_points):
    """在校正后的矩形内生成圆形轨迹点"""
    circle_points = []
    cx, cy = center
    for i in range(num_points):
        angle = 2 * math.pi * i / num_points
        x = int(cx + radius * math.cos(angle))
        y = int(cy + radius * math.sin(angle))
        circle_points.append((x, y))
    return circle_points

# --------------------------- 透视变换工具函数 ---------------------------
def perspective_transform(pts, target_width, target_height):
    """
    对四边形进行透视变换
    :param pts: 四边形顶点坐标 (4,2)
    :param target_width: 校正后宽度
    :param target_height: 校正后高度
    :return: 变换矩阵M和逆矩阵M_inv
    """
    # 顶点排序（左上→右上→右下→左下）
    s = pts.sum(axis=1)
    tl = pts[np.argmin(s)]
    br = pts[np.argmax(s)]
    diff = np.diff(pts, axis=1)
    tr = pts[np.argmin(diff)]
    bl = pts[np.argmax(diff)]
    src_pts = np.array([tl, tr, br, bl], dtype=np.float32)
    
    # 目标坐标
    dst_pts = np.array([
        [0, 0], [target_width-1, 0],
        [target_width-1, target_height-1], [0, target_height-1]
    ], dtype=np.float32)
    
    # 计算变换矩阵
    M = cv2.getPerspectiveTransform(src_pts, dst_pts)
    ret, M_inv = cv2.invert(M)  # 逆矩阵用于映射回原图
    return M, M_inv, src_pts

# --------------------------- 主程序 ---------------------------
if __name__ == "__main__":
    # 初始化设备
    disp = display.Display()
    cam = camera.Camera(320, 240, image.Format.FMT_BGR888)
    laser_detector = PurpleLaserDetector()
    
    # 初始化串口
    uart = SimpleUART()
    if uart.init("/dev/ttyS0", 115200, set_as_global=True):
        print("串口初始化成功")
        uart.set_frame("$$", "##", True)
    else:
        print("串口初始化失败")
        exit()

    # 核心参数
    min_contour_area = 3000
    max_contour_area = 40000
    target_sides = 4
    
    # 透视变换与圆形参数
    corrected_width = 200    # 校正后矩形宽度
    corrected_height = 150   # 校正后矩形高度
    circle_radius = 40       # 校正后矩形内圆的半径
    circle_num_points = 12   # 圆周点数量

    while not app.need_exit():
        # 读取图像
        img = cam.read()
        img_cv = image.image2cv(img, ensure_bgr=True, copy=False)
        output = img_cv.copy()

        # 1. 矩形检测
        gray = cv2.cvtColor(img_cv, cv2.COLOR_BGR2GRAY)
        _, binary = cv2.threshold(gray, 46, 255, cv2.THRESH_BINARY)
        contours, _ = cv2.findContours(binary, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        
        quads = []
        for cnt in contours:
            area = cv2.contourArea(cnt)
            if min_contour_area < area < max_contour_area:
                epsilon = 0.03 * cv2.arcLength(cnt, True)
                approx = cv2.approxPolyDP(cnt, epsilon, True)
                if len(approx) == target_sides:
                    quads.append((approx, area))

        # 按面积排序（外框在前，内框在后）
        quads.sort(key=lambda x: x[1], reverse=True)
        inner_quads = quads[1:]  # 只处理内框

        # 2. 处理内框：透视变换→画圆→映射回原图
        all_circle_points = []  # 存储所有映射回原图的圆轨迹点
        for approx, area in inner_quads:
            # 提取顶点
            pts = approx.reshape(4, 2).astype(np.float32)
            
            # 计算透视变换矩阵
            M, M_inv, src_pts = perspective_transform(
                pts, corrected_width, corrected_height
            )
            
            # 生成校正后矩形内的圆形轨迹（圆心为校正后矩形的中心）
            corrected_center = (corrected_width//2, corrected_height//2)
            corrected_circle = generate_circle_points(
                corrected_center, circle_radius, circle_num_points
            )
            
            # 将校正后的圆轨迹点映射回原图
            if M_inv is not None:
                # 格式转换为opencv需要的形状 (1, N, 2)
                corrected_points_np = np.array([corrected_circle], dtype=np.float32)
                original_points = cv2.perspectiveTransform(corrected_points_np, M_inv)[0]
                original_points = [(int(x), int(y)) for x, y in original_points]
                all_circle_points.extend(original_points)
                
                # 绘制映射回原图的轨迹点（红色）
                for (x, y) in original_points:
                    cv2.circle(output, (x, y), 2, (0, 0, 255), -1)
            
            # 绘制内框轮廓和中心点（调试用）
            cv2.drawContours(output, [approx], -1, (0, 255, 0), 2)
            M_moments = cv2.moments(approx)
            if M_moments["m00"] != 0:
                cx = int(M_moments["m10"] / M_moments["m00"])
                cy = int(M_moments["m01"] / M_moments["m00"])
                cv2.circle(output, (cx, cy), 5, (255, 0, 0), -1)

        # 3. 激光检测
        output, laser_points = laser_detector.detect(output)

        # 4. 串口发送数据
        # 发送内框圆轨迹点（格式：C,数量,x1,y1,x2,y2...）
        if all_circle_points:
            circle_data = f"C,{len(all_circle_points)}"
            for (x, y) in all_circle_points:
                circle_data += f",{x},{y}"
            micu_printf(circle_data)
        
        # 发送激光点
        if laser_points:
            laser_data = f"L,{len(laser_points)}"
            for (x, y) in laser_points:
                laser_data += f",{x},{y}"
            micu_printf(laser_data)

        # 显示图像
        img_show = image.cv2image(output, bgr=True, copy=False)
        disp.show(img_show)