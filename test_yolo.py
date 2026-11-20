#导包
from ultralytics import YOLO
import cv2

#创建模型
model = YOLO('yolov8n.pt')

#调用摄像头
cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("无法打开摄像头")
    exit()
print("摄像头已打开")

#主循环--呈现画面
while True:
    ret, frame = cap.read() #一帧一帧读取
    if not ret:
        print("无法获取")
        break
    #识别画面
    results = model(frame,verbose=False)
    #绘制框架
    boder = results[0].plot()
    #显示画面
    cv2.imshow('TEST_YOLO', boder)
    #退出
    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("关闭摄像头")
        break

#释放资源
cap.release()
cv2.destroyAllWindows()
