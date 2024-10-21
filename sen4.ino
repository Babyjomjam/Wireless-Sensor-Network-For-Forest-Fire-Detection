#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

//---------------------------------------- LINE Notify settings
String token = "IKWfn7WCRnawIPw3ncDs1D1fyUK5Nfuvap6WwWSm9aO";  // แทนด้วย token ของคุณ

void sendLineNotify(String message) {
  WiFiClientSecure client;
  client.setInsecure();  // ข้ามการตรวจสอบใบรับรองสำหรับทดสอบ
  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("การเชื่อมต่อล้มเหลว");
    return;
  }

  String lineNotifyMessage = "message=" + message;
  client.println("POST /api/notify HTTP/1.1");
  client.println("Host: notify-api.line.me");
  client.println("Authorization: Bearer " + token);
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Content-Length: " + String(lineNotifyMessage.length()));
  client.println();
  client.print(lineNotifyMessage);

  Serial.println("ส่งข้อความไปยัง LINE Notify แล้ว");
}

//---------------------------------------- Firebase และเซ็นเซอร์ BME680 settings
#define WIFI_SSID "OnePlus 11"
#define WIFI_PASSWORD "audir8v10+"
#define API_KEY "AIzaSyD0mHuBU_T0DZiE2EEYWBH0JngLrMOOlKs"
#define DATABASE_URL "https://esp32-bme680-422ef-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
Adafruit_BME680 bme;

unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 10000;

bool signupOK = false;

#define SEALEVELPRESSURE_HPA 1013.25  // ความดันอากาศที่ระดับน้ำทะเลมาตรฐาน

//

   s4 ทุ่งแสลงหลวง
float latitude = 16.577007;
float longitude =  100.881887;

//---------------------------------------- NTP Client Setup
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 7 * 3600;  // ชดเชยเวลาสำหรับประเทศไทย (+7 ชั่วโมง)
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds, 60000);  // ชดเชยเวลาสำหรับประเทศไทย (+7 ชั่วโมง)

// ฟังก์ชันแปลง epoch time เป็นวัน, เดือน, ปี, ชั่วโมง, นาที, วินาที
void getFormattedDateTime(unsigned long epochTime, int &day, int &month, int &year, int &hour, int &minute, int &second) {
  // ไม่ต้องชดเชยเวลาเพิ่มเติมแล้ว เนื่องจากใช้ utcOffsetInSeconds ใน NTPClient แล้ว
  second = epochTime % 60;
  epochTime /= 60;
  minute = epochTime % 60;
  epochTime /= 60;
  hour = epochTime % 24;
  epochTime /= 24;

  // คำนวณปี (เช็ค leap years)
  int days = epochTime;
  year = 1970;
  while ((days >= 365 && !(year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) || (days >= 366 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))) {
    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
      days -= 366; // Leap year
    } else {
      days -= 365;
    }
    year++;
  }

  // คำนวณเดือนและวัน
  int monthDays[] = { 31, (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  month = 0;
  while (days >= monthDays[month]) {
    days -= monthDays[month];
    month++;
  }
  day = days + 1;
  month += 1;
}

//---------------------------------------- Token Status Callback
void tokenStatusCallback(TokenInfo info) {
  switch (info.status) {
    case token_status_uninitialized:
      Serial.println("สถานะ Token: ยังไม่ถูกกำหนด");
      break;
    case token_status_on_signing:
      Serial.println("สถานะ Token: กำลังลงชื่อเข้าใช้");
      break;
    case token_status_ready:
      Serial.println("สถานะ Token: พร้อม");
      break;
    case token_status_on_refresh:
      Serial.println("สถานะ Token: กำลังรีเฟรช");
      break;
    case token_status_error:
      Serial.println("สถานะ Token: เกิดข้อผิดพลาด");
      break;
    default:
      Serial.println("สถานะ Token: ไม่ทราบ");
      break;
  }
}

//---------------------------------------- Setup
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // ไฟ LED บนบอร์ด

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("เชื่อมต่อ WiFi แล้ว");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ลงชื่อเข้าใช้สำเร็จ");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Callback ตรวจสอบสถานะ Token
  config.token_status_callback = tokenStatusCallback;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // เริ่มต้น NTPClient เพื่อซิงค์เวลากับเซิร์ฟเวอร์
  timeClient.begin();

  // เริ่มต้นเซ็นเซอร์ BME680 โดยใช้ที่อยู่ 0x77
  if (!bme.begin(0x77)) {  // ใช้ 0x77 ตามที่พบจาก I2C Scanner
    Serial.println("ไม่พบเซ็นเซอร์ BME680 กรุณาตรวจสอบการเชื่อมต่อสายไฟ!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320°C เป็นเวลา 150 ms
}

//---------------------------------------- Loop
void loop() {
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // อัปเดตเวลา
    timeClient.update();

    // ดึงค่า Epoch Time จาก timeClient
    unsigned long epochTime = timeClient.getEpochTime();
    
    // แปลงค่า Epoch Time เป็นวัน, เดือน, ปี, ชั่วโมง, นาที, วินาที
    int day, month, year, hour, minute, second;
    getFormattedDateTime(epochTime, day, month, year, hour, minute, second);

    // สร้าง timestamp ในรูปแบบ dd/mm/yyyy HH:MM:SS
    String timestamp = String(day) + "/" +
                       String(month) + "/" +
                       String(year) + " " +
                       String(hour) + ":" +
                       String(minute) + ":" +
                       String(second);

    // ใช้ค่าเซ็นเซอร์จริง (ถ้าใช้งานเซ็นเซอร์)
    if (!bme.performReading()) {
      Serial.println("ไม่สามารถอ่านค่าจากเซ็นเซอร์ได้");
      return;
    }
    float temperature = bme.temperature;
    float pressure = bme.pressure / 100.0;
    float humidity = bme.humidity;
    float gas = bme.gas_resistance / 1000.0;
    float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);

    // แสดงผลค่าที่กำหนด
    Serial.printf("Temperature: %.2f°C\nHumidity: %.2f%%\nGas: %.2f KOhms\nPressure: %.2f hPa\nAltitude: %.2f m\n", temperature, humidity, gas, pressure, altitude);
    Serial.println("Date and Time: " + timestamp);

    // บันทึกข้อมูลลง Firebase
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/humidity", humidity);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/gas", gas);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/pressure", pressure);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/altitude", altitude);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/latitude", latitude);
    Firebase.RTDB.setFloat(&fbdo, "Test1/sensor4/longitude", longitude);
    Firebase.RTDB.setString(&fbdo, "Test1/sensor4/timestamp", timestamp); // บันทึก timestamp ลงใน Firebase

    // ตรวจสอบเงื่อนไขไฟไหม้โดยใช้ค่าที่กำหนด
    checkFireAlert(temperature, gas, humidity, pressure, altitude, latitude, longitude, timestamp);
  }
}

//---------------------------------------- ฟังก์ชันแจ้งเตือนไฟไหม้
void checkFireAlert(float temperature, float gas, float humidity, float pressure, float altitude, float latitude, float longitude, String timestamp) {
  if (temperature > 30 && gas < 30 && humidity < 30) {
    String message = "🔥 ตรวจพบไฟไหม้!\n";
    message += "พิกัด: " + String(latitude, 6) + "," + String(longitude, 6) + "\n";
    message += "วันที่และเวลา: " + timestamp + "\n"; // เพิ่ม timestamp ที่มีทั้งวันที่และเวลา
    
    
    sendLineNotify(message);
  }
}
