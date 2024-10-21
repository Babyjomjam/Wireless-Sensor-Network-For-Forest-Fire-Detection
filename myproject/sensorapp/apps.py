from django.apps import AppConfig  # นำเข้า AppConfig จาก django.apps เพื่อกำหนดการตั้งค่าของแอป

# สร้างคลาสการตั้งค่าของแอป sensorapp
class SensorappConfig(AppConfig):
    default_auto_field = 'django.db.models.BigAutoField'  # กำหนดฟิลด์ primary key เริ่มต้นสำหรับโมเดลที่ไม่มีการกำหนดค่า explicit
    name = 'sensorapp'  # กำหนดชื่อของแอปในโปรเจกต์
