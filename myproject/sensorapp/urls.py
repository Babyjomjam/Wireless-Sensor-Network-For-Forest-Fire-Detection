from django.urls import path
from . import views

urlpatterns = [
    path('', views.home_view, name='home'),  # หน้า welcome
    path('sensor/', views.sensor_data_view, name='sensor_data'),  # หน้าดูค่าจากเซ็นเซอร์
]
