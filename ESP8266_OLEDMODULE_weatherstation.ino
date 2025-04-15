#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Thông tin WiFi tạm thời
const char* ssid = "WS_DR Config";  // Mạng WiFi AP
const char* password = "00000000";   // Mật khẩu cho mạng AP

// Thông tin WeatherAPI
String apiKey = "aa1239d7e849462abde120222241312";
String city = "Hanoi";  // Mặc định là Hanoi, nhưng sẽ thay đổi khi người dùng nhập
String apiUrl = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + city + "&aqi=no";

// Cấu hình OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 14
#define SCL_PIN 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WiFiClient client;

float temperature;
String weatherDescription;
float humidity;
float windSpeed;
String currentDateTime;

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1800000; // 30 phút

ESP8266WebServer server(80);  // Khởi tạo Web Server trên port 80

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // Bật chế độ Access Point để nhập WiFi
  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Khởi tạo Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/setWiFi", HTTP_POST, handleWiFi);

  server.begin();
  
  // Khởi tạo màn hình OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);  // Giảm kích thước văn bản
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("DR.NGOMINHTU'sPROJECT");
  display.println("WEATHER STATION");
  display.print("Wifi: ");
  display.println(ssid);  // Hiển thị SSID

  display.print("Password: ");
  display.println(password);  // Hiển thị mật khẩu
  display.println("IP:192.168.4.1");
  display.display();
  delay(2000);
}

void loop() {
  server.handleClient();  // Xử lý các yêu cầu từ client (trình duyệt)

  // Hiển thị trạng thái Wi-Fi trên màn hình OLED
  display.clearDisplay();
  display.setTextSize(1);  // Giảm kích thước văn bản
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (WiFi.status() == WL_CONNECTED) {
    display.println("WiFi: Connected");
  } else {
    display.println("WiFi: Disconnected");
  }

  // Hiển thị thông tin thời tiết trên màn hình OLED
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - lastUpdate > updateInterval || lastUpdate == 0) {
      getWeatherData();
      lastUpdate = millis();
    }
    display.println("Weather in " + city);
    display.println(currentDateTime);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Cond: ");
    display.println(weatherDescription);

    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    display.print("Wind: ");
    display.print(windSpeed);
    display.println(" km/h");

    display.display();
  }
}

// Trang chủ của Web Server, nơi người dùng có thể nhập Wi-Fi và thành phố
void handleRoot() {
  String html = "<html><body>";
  html += "<h2>Enter Wi-Fi Credentials</h2>";
  html += "<form action='/setWiFi' method='POST'>";
  html += "<label>SSID:</label><input type='text' name='ssid'><br>";
  html += "<label>Password:</label><input type='password' name='password'><br>";
  html += "<label>City:</label><input type='text' name='city' value='" + city + "'><br>";  // Thêm trường thành phố
  html += "<input type='submit' value='Submit'>";
  html += "</form>";
  html += "<h6>Dr.ngominhtu@gmail.com</h6>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Lấy thông tin Wi-Fi và thành phố từ form và kết nối đến Wi-Fi
void handleWiFi() {
  String newSsid = server.arg("ssid");
  String newPassword = server.arg("password");
  city = server.arg("city");  // Cập nhật thành phố từ form

  apiUrl = "http://api.weatherapi.com/v1/current.json?key=" + apiKey + "&q=" + city + "&aqi=no";  // Cập nhật URL API với thành phố mới

  WiFi.begin(newSsid.c_str(), newPassword.c_str());

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.send(200, "text/html", "<h2>Wi-Fi Connected Successfully!</h2><p>IP: " + WiFi.localIP().toString() + "</p>");
    Serial.println("Connected to Wi-Fi!");
    Serial.println("IP Address: " + WiFi.localIP().toString());
  } else {
    server.send(200, "text/html", "<h2>Failed to Connect to Wi-Fi</h2>");
  }
}

// Lấy dữ liệu thời tiết từ API
void getWeatherData() {
  HTTPClient http;
  http.begin(client, apiUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);

    temperature = doc["current"]["temp_c"].as<float>();
    weatherDescription = doc["current"]["condition"]["text"].as<String>();
    humidity = doc["current"]["humidity"].as<float>();
    windSpeed = doc["current"]["wind_kph"].as<float>();
    
    // Lấy thông tin ngày tháng năm
    String dateTimeRaw = doc["location"]["localtime"].as<String>();
    String dateOnly = dateTimeRaw.substring(0, 10); // Lấy phần ngày tháng năm (yyyy-MM-dd)

    // Tính thứ mấy
    String dayOfWeek = getDayOfWeek(dateOnly);

    // Hiển thị ngày tháng và thứ
    currentDateTime = dateOnly + " " + dayOfWeek;

    Serial.println("Updated weather info:");
    Serial.println("Temp: " + String(temperature) + " C");
    Serial.println("Cond: " + weatherDescription);
    Serial.println("Humidity: " + String(humidity) + " %");
    Serial.println("Wind: " + String(windSpeed) + " km/h");
    Serial.println("Time: " + currentDateTime);
  } else {
    Serial.println("Failed to get data!");
  }

  http.end();
}

// Hàm tính thứ mấy từ chuỗi ngày tháng (yyyy-MM-dd)
String getDayOfWeek(String date) {
  // date có dạng yyyy-MM-dd
  int year = date.substring(0, 4).toInt();
  int month = date.substring(5, 7).toInt();
  int day = date.substring(8, 10).toInt();
  
  // Công thức Zeller để tính thứ
  if (month <= 2) {
    month += 12;
    year--;
  }
  int k = year % 100;
  int j = year / 100;
  
  int dayOfWeek = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
  
  // Thứ trong công thức Zeller trả về từ 0 (Thứ bảy) đến 6 (Thứ sáu)
  switch (dayOfWeek) {
    case 0: return "Sat";
    case 1: return "Sun";
    case 2: return "Mon";
    case 3: return "Tue";
    case 4: return "Wed";
    case 5: return "Thu";
    case 6: return "Fri";
    default: return "Unknown";
  }
}
