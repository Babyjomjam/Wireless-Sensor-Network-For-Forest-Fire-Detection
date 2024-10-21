"""
WSGI config for myproject project.

It exposes the WSGI callable as a module-level variable named ``application``.

For more information on this file, see
https://docs.djangoproject.com/en/5.1/howto/deployment/wsgi/
"""

import os  # นำเข้าโมดูล os เพื่อจัดการการตั้งค่าสภาพแวดล้อม

from django.core.wsgi import get_wsgi_application  # นำเข้า get_wsgi_application เพื่อสร้าง WSGI application สำหรับโปรเจกต์

# ตั้งค่าตัวแปรสภาพแวดล้อม 'DJANGO_SETTINGS_MODULE' ให้ชี้ไปที่การตั้งค่าในโปรเจกต์ 'myproject'
os.environ.setdefault('DJANGO_SETTINGS_MODULE', 'myproject.settings')

# เรียก get_wsgi_application() เพื่อสร้าง WSGI application ที่จะใช้โดยเซิร์ฟเวอร์ WSGI (เช่น Gunicorn, uWSGI)
application = get_wsgi_application()
