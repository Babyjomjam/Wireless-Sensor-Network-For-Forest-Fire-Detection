import pyrebase
from django.shortcuts import render

# Firebase configuration
firebaseConfig = {
    "apiKey": "AIzaSyD0mHuBU_T0DZiE2EEYWBH0JngLrMOOlKs",
    "authDomain": "esp32-bme680-422ef.firebaseapp.com",
    "databaseURL": "https://esp32-bme680-422ef-default-rtdb.asia-southeast1.firebasedatabase.app",
    "projectId": "esp32-bme680-422ef",
    "storageBucket": "esp32-bme680-422ef.appspot.com",
    "messagingSenderId": "307866788331",
    "appId": "1:307866788331:web:249fc26bed7b7073cff9ae",
    "measurementId": "G-BDYD3BVMXT"
}

# Initialize Firebase
firebase = pyrebase.initialize_app(firebaseConfig)
db = firebase.database()

# ฟังก์ชันแสดงหน้า welcome.html
def home_view(request):
    return render(request, 'welcome.html')

# ฟังก์ชันแสดงข้อมูลจาก Firebase
def sensor_data_view(request):
    try:
        # ดึงข้อมูลจาก Firebase Realtime Database จากโหนด 'Test1'
        data = db.child("Test1").get().val()

        if data:
            sensor_list = []
            for sensor_id, sensor in data.items():
                sensor_list.append({
                    'tem': sensor.get('temperature', 'N/A'),
                    'humi': sensor.get('humidity', 'N/A'),
                    'pressure': sensor.get('pressure', 'N/A'),
                    'gas': sensor.get('gas', 'N/A'),
                    'altitude': sensor.get('altitude', 'N/A'),
                    'lat': sensor.get('latitude', 'N/A'),  # แก้ไขให้ตรงกับ Firebase
                    'lng': sensor.get('longitude', 'N/A'),  # แก้ไขให้ตรงกับ Firebase
                    'timestamp': sensor.get('timestamp', 'N/A')
                })
                
            context = {'sensors': sensor_list}  # ย้าย context ออกมานอกลูป
        else:
            context = {'sensors': []}
        
    except Exception as e:
        # ถ้ามีข้อผิดพลาด จะส่งข้อความข้อผิดพลาดไปที่หน้าเว็บ
        context = {'error': str(e)}
        
    return render(request, 'sensorapp/index.html', context)

