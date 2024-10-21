"""
ASGI config for myproject project.

It exposes the ASGI callable as a module-level variable named ``application``.

For more information on this file, see
https://docs.djangoproject.com/en/5.1/howto/deployment/asgi/
"""

import os  # นำเข้าโมดูล os เพื่อจัดการการตั้งค่าสภาพแวดล้อม

from django.core.asgi import get_asgi_application  # นำเข้า get_asgi_application จาก Django เพื่อสร้าง ASGI application

# ตั้งค่าตัวแปรสภาพแวดล้อม 'DJANGO_SETTINGS_MODULE' ให้ชี้ไปที่การตั้งค่าในโปรเจกต์ 'myproject'
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'myproject.settings')

# เรียก get_asgi_application() เพื่อสร้าง ASGI application callable ที่จะใช้โดยเซิร์ฟเวอร์ ASGI (เช่น Daphne, Uvicorn)
application = get_asgi_application()
