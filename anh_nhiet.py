import serial
import numpy as np
import cv2

# ================= CẤU HÌNH =================
SERIAL_PORT = 'COM4' 
BAUD_RATE = 921600
# ============================================

# Mở cổng Serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    print(f"Đã kết nối thành công tới {SERIAL_PORT} ở tốc độ {BAUD_RATE}")
except Exception as e:
    print(f"Lỗi mở cổng COM: {e}")
    exit()

print("Đang chờ dữ liệu từ STM32... Nhấn 'q' trên cửa sổ ảnh để thoát.")

frame_data = [] # Mảng tạm để hứng 768 con số

while True:
    try:
        # Đọc từng dòng từ mạch STM32 gửi lên
        line = ser.readline().decode('utf-8').strip()

        # Bỏ qua dòng trống
        if not line:
            continue

        # Nếu dòng bắt đầu bằng dấu gạch ngang (Tín hiệu kết thúc Frame)
        if line.startswith('---'):
            
            # Kiểm tra xem có gom đủ 768 điểm ảnh (24x32) không
            if len(frame_data) == 768:
                # 1. Chuyển thành ma trận 24 dòng x 32 cột
                temp_array = np.array(frame_data).reshape((24, 32))

                # 2. Lọc nhiễu và khóa dải nhiệt (Giới hạn hiển thị từ 20 đến 40 độ C)
                T_MIN = 20.0
                T_MAX = 40.0
                temp_array = np.clip(temp_array, T_MIN, T_MAX)

                # 3. Chuẩn hóa về 0-255
                norm_img = ((temp_array - T_MIN) / (T_MAX - T_MIN) * 255).astype(np.uint8)
                
                # SỬA Ở ĐÂY: Đổi cv2.INTER_CUBIC thành cv2.INTER_NEAREST để giữ nguyên dạng pixel thô
                img_resized = cv2.resize(norm_img, (640, 480), interpolation=cv2.INTER_NEAREST)

                # 4. Phủ màu Heatmap (COLORMAP_JET: Xanh -> Vàng -> Đỏ)
                heatmap = cv2.applyColorMap(img_resized, cv2.COLORMAP_JET)

                # 5. Viết chữ hiển thị nhiệt độ lớn nhất / nhỏ nhất
                max_temp = np.max(frame_data)
                min_temp = np.min(frame_data)
                cv2.putText(heatmap, f"Max: {max_temp:.1f} C", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
                cv2.putText(heatmap, f"Min: {min_temp:.1f} C", (10, 70), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

                # 6. Show ảnh lên màn hình
                cv2.imshow('MLX90640 Raw Pixels', heatmap)

                # Nhấn 'q' để thoát
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
            else:
                if len(frame_data) > 0:
                    print(f"Bị thiếu/lỗi dữ liệu: Chỉ nhận được {len(frame_data)}/768 điểm. Đang chờ khung hình tiếp theo...")

            # Xóa mảng cũ đi để chuẩn bị hứng khung hình mới
            frame_data = []

        else:
            # Nếu KHÔNG PHẢI dòng gạch ngang -> Đây là dòng chứa 32 giá trị nhiệt độ
            values = [float(x) for x in line.split() if x.strip()]
            frame_data.extend(values)

    except ValueError:
        pass
    except UnicodeDecodeError:
        pass

ser.close()
cv2.destroyAllWindows()