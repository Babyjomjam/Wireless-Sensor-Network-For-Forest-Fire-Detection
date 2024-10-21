from django.contrib import admin  # นำเข้าโมดูลสำหรับการใช้งานหน้า admin ของ Django
from django.urls import include, path  # นำเข้าโมดูลสำหรับการกำหนดเส้นทาง URL

urlpatterns = [
    path('admin/', admin.site.urls),  # เมื่อเข้าถึง '/admin/', จะส่งผู้ใช้ไปที่หน้า Django admin
    path('', include('sensorapp.urls')),  # เส้นทางหลัก ('/') จะเชื่อมต่อกับ urls.py ของแอป sensorapp
]
