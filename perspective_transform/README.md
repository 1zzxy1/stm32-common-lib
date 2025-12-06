# 透视变换工具

> 📦 **Python工具库** - 图像透视变换

## 功能特性

- 四边形透视变换矩阵计算
- 支持正向和逆向变换
- 顶点自动排序

## 使用场景

✅ 图像矫正、视觉伺服、斜视校正、文档扫描
⚠️ 不适用STM32项目（Python库，用于上位机）

## API示例

```python
import cv2
import numpy as np
from perspective_transform import perspective_transform

# 定义四边形顶点（源图像）
src_points = np.array([
    [100, 100],
    [300, 120],
    [310, 280],
    [90, 260]
], dtype=np.float32)

# 定义目标矩形（校正后）
dst_points = np.array([
    [0, 0],
    [200, 0],
    [200, 200],
    [0, 200]
], dtype=np.float32)

# 执行透视变换
warped = perspective_transform(image, src_points, dst_points)

# 显示结果
cv2.imshow('Warped', warped)
cv2.waitKey(0)
```

## 依赖项

- Python 3.6+
- OpenCV (cv2)
- NumPy
