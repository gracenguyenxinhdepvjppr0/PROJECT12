import serial
import numpy as np
import cv2
import base64
import json
import paho.mqtt.client as mqtt
import time

# ================= CẤU HÌNH =================
SERIAL_PORT = 'COM4'             # Thay đổi tên cổng (ttyUSB0, ttyACM0...)
BAUD_RATE = 115200                       # Khớp với code STM32 của bạn
THINGSBOARD_HOST = 'demo.thingsboard.io' # IP hoặc Domain của ThingsBoard
ACCESS_TOKEN = 'y5k92bwo22q59mgjdb46'       # Dán Access Token của thiết bị vào đây
# ============================================

# Khởi tạo MQTT Client
client = mqtt.Client()
client.username_pw_set(ACCESS_TOKEN)

def connect_mqtt():
    try:
        client.connect(THINGSBOARD_HOST, 1883, 60)
        client.loop_start()
        print(f"Đã kết nối thành công tới ThingsBoard: {THINGSBOARD_HOST}")
    except Exception as e:
        print(f"Lỗi kết nối MQTT: {e}")
        exit()

connect_mqtt()

# Mở cổng Serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f"Đã mở cổng {SERIAL_PORT}")
except Exception as e:
    print(f"Lỗi mở cổng COM: {e}")
    exit()

def read_thermal_frame():
    data_str = ""
    recording = False
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if line == "FRAME_START":
                recording = True
                data_str = ""
            elif line == "FRAME_END":
                if recording: return data_str
            elif recording:
                data_str += line
        except:
            pass # Bỏ qua nhiễu rác UART

print("Đang đọc dữ liệu từ STM32 và đẩy lên ThingsBoard...")

while True:
    frame_data = read_thermal_frame()
    
    if frame_data:
        try:
            # Tách chuỗi thành mảng float
            values = [float(x) for x in frame_data.split(',') if x.strip()]
            
            if len(values) == 768:
                # 1. Vẽ ảnh Heatmap bằng OpenCV
                temp_array = np.array(values).reshape((24, 32))
                
                # Giới hạn dải nhiệt độ (ví dụ từ 20 đến 40 độ C để màu sắc ổn định)
                T_MIN = 20.0
                T_MAX = 40.0
                temp_array = np.clip(temp_array, T_MIN, T_MAX)
                
                # Chuẩn hóa mảng về 0-255
                norm_img = ((temp_array - T_MIN) / (T_MAX - T_MIN) * 255).astype(np.uint8)
                
                # Phóng to làm mượt ảnh lên 640x480
                img_resized = cv2.resize(norm_img, (640, 480), interpolation=cv2.INTER_CUBIC)
                
                # Áp dụng bộ lọc màu Heatmap (Xanh -> Đỏ)
                heatmap = cv2.applyColorMap(img_resized, cv2.COLORMAP_JET)
                
                # 2. Mã hóa ảnh thành chuỗi Base64
                _, buffer = cv2.imencode('.jpg', heatmap)
                jpg_as_text = base64.b64encode(buffer).decode('utf-8')
                
                # Tiền tố này giúp HTML nhận diện đây là ảnh JPEG
                base64_string = "data:image/jpeg;base64," + jpg_as_text

                # 3. Đóng gói thành JSON và gửi lên ThingsBoard
                telemetry_data = {
                    "thermal_image": base64_string,
                    "max_temp": round(float(np.max(values)), 2),
                    "min_temp": round(float(np.min(values)), 2)
                }
                
                client.publish("v1/devices/me/telemetry", json.dumps(telemetry_data))
                print(f"Đã cập nhật khung hình mới! Nhiệt độ Max: {telemetry_data['max_temp']} °C")
                
            else:
                print(f"Khung hình bị thiếu dữ liệu (chỉ nhận được {len(values)}/768 điểm)")
                
        except ValueError:
            print("Lỗi chuyển đổi dữ liệu, có thể do nhiễu UART")
        except Exception as e:
            print("Lỗi hệ thống:", e)