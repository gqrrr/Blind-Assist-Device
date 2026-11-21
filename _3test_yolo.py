#导包
from ultralytics import YOLO
import cv2
import serial
import time

#串口配置
SERIAL_PORT = 'COM5'
SERIAL_BAUDRATE = 115200
#异常处理
try:
    #创建串口对象
    ser=serial.Serial(SERIAL_PORT,SERIAL_BAUDRATE,timeout=0.1)
    print(f"成功打开串口{SERIAL_PORT}")
except Exception as e:
    print(f"无法打开串口{SERIAL_PORT}: {e}")
    ser=None

#创建模型
model=YOLO('yolov8n.pt')

#调用摄像头
cap=cv2.VideoCapture(1)
if not cap.isOpened():
    print("无法打开摄像头")
    exit()
print("摄像头已打开")

#设立 ‘危险’--障碍物 名单（先从80个模型中挑取）
danger_list=['person','chair','car','bottle']

#主循环
while True:
    ret, frame = cap.read()
    if not ret:
        print("无法获取画面")
        break
    #获取画面的高度和宽度--算面积
    h,w=frame.shape[:2]
    frame_area=h*w
    #识别画面
    result=model(frame,verbose=False)
    #默认画面安全
    is_danger=False
    danger_name=''
    #遍历当前帧识别的物品
    for box in result[0].boxes:
        #获取物品的名字
        name=model.names[int(box.cls[0])]
        #判断是否是危险物品
        if name in danger_list:
            #获取物品框的坐标
            x1,y1,x2,y2=box.xyxy[0].cpu().numpy()
            #获取物品框的面积
            box_area=(x2-x1)*(y2-y1)
            #算物品所占比例
            ratio=box_area/frame_area
            #判断是否危险(物品所占比例在15%以上)->调至20%->25%
            if ratio>0.25:
                is_danger=True
                danger_name=name
                print(f"检测到危险物品：{danger_name}")
                break
    #绘制框架
    boder=result[0].plot()
    #根据是否安全输出提示--发送串口数据
    if is_danger:
        cv2.putText(boder,f"Danger! {danger_name} is too close!",(20,50),cv2.FONT_HERSHEY_SIMPLEX,0.8,(0,0,255),2)
        if ser:
            ser.write(b'1')
    else:
        cv2.putText(boder,"Safe",(20,50),cv2.FONT_HERSHEY_SIMPLEX,1,(0,255,0),2)
        if ser:
            ser.write(b'0')
    #显示画面
    cv2.imshow('TEST_YOLO',boder)
    #按q键退出
    if cv2.waitKey(1)&0xFF==ord('q'):
        break

#释放资源
cap.release()
cv2.destroyAllWindows()
if ser:
    ser.close()

