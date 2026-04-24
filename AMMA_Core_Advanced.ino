// =====================================================================
// System Architecture: Ahmed Mohamed Mohamed Ahmed's Smart Core V2.0
// Description: Asynchronous Server with RSSI & Uptime Telemetry
// Authorized Core ID: AMMA_SYS_2026
// =====================================================================

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

// [1] إعدادات الشبكة (Network Configuration)
const char *ssid = "Ahmedmx2";          // اسم شبكتك (تأكد من مطابقته للراوتر)
const char *password = "YOUR_PASSWORD"; // ضع الرقم السري هنا

// [2] إنشاء كائنات السيرفر وقناة الاتصال (Server & SSE Objects)
// نستخدم بورت 80 (المنفذ القياسي لبروتوكول HTTP)
AsyncWebServer Ahmed_Mohamed_Mohamed_Ahmed_Server(80);
// إنشاء قناة Server-Sent Events (SSE) لإرسال البيانات للمتصفح بدون Refresh
AsyncEventSource AMMA_Events("/events");

// [3] تعريف بنات الهاردوير (Hardware Pins Definition)
const int PIR_PIN = 27;  // بن حساس الحركة
const int LDR_PIN = 32;  // بن حساس الإضاءة

// [4] بناء هيكل البيانات للأجهزة (Data Structure for Devices)
// الـ Struct بيجمع كل خصائص الريلاي الواحد في "قالب" واحد لتقليل المتغيرات وتنظيف الكود
struct device {
  const int id;       // رقم تعريفي للريلاي في واجهة الويب
  const int pin;      // البن المتوصل عليه الريلاي في الشريحة
  const int btnPin;   // البن المخصص لزرار التحكم اليدوي الخارجي (إن وُجد)
  int prevBtnState;   // لتتبع حالة الزرار اليدوي السابقة (Debouncing logic)
  int status;         // حالة الريلاي الحالية (0: مطفي, 1: شغال)
};

// تهيئة مصفوفة الأجهزة (System Relays Mapping)
struct device d1 = { 1, 19, 12, 0, 0 }; // ريلاي 1 (مربوط بحساس الحركة)
struct device d2 = { 2, 21, 14, 0, 0 }; // ريلاي 2 (مربوط بحساس الإضاءة)
struct device d3 = { 3, 22, 25, 0, 0 }; // ريلاي 3 (تحكم حر)
struct device d4 = { 4, 23, 26, 0, 0 }; // ريلاي 4 (تحكم حر)

// [5] متغيرات تتبع حالة الحساسات (State Tracking Variables)
int pirState = LOW;             
int lastPirState = LOW;
unsigned long pirTimer = 0;           // لاستخدام millis() كتايمر بدل delay()
const unsigned long pirDelay = 10000; // تأخير 10 ثواني قبل إطفاء اللمبة بعد توقف الحركة

int ldrState = HIGH; 
int lastLdrState = HIGH;

// [6] معالج النصوص الديناميكي (Dynamic HTML Processor)
// الدالة دي بتستبدل المتغيرات بين علامات %% في كود الـ HTML بالحالة الحقيقية قبل إرسال الصفحة
String processor(const String &var) {
  if (var == "btn1txt") return d1.status == 0 ? "ON" : "OFF";
  if (var == "btn2txt") return d2.status == 0 ? "ON" : "OFF";
  if (var == "btn3txt") return d3.status == 0 ? "ON" : "OFF";
  if (var == "btn4txt") return d4.status == 0 ? "ON" : "OFF";
  if (var == "btn1class") return d1.status == 0 ? "button" : "button2";
  if (var == "btn2class") return d2.status == 0 ? "button" : "button2";
  if (var == "btn3class") return d3.status == 0 ? "button" : "button2";
  if (var == "btn4class") return d4.status == 0 ? "button" : "button2";
  if (var == "pirStatus") return pirState == HIGH ? "Motion Detected" : "Area Clear";
  if (var == "ldrStatus") return ldrState == LOW ? "Dark Mode Active" : "Daylight Mode";
  if (var == "sysUptime") return String(millis() / 60000) + " Mins"; // حساب وقت التشغيل بالدقائق
  if (var == "sysRSSI") return String(WiFi.RSSI()) + " dBm";         // قراءة قوة إشارة الواي فاي
  return String();
}

// [7] واجهة المستخدم (Web Interface - HTML/CSS/JS)
// مخزنة في الـ PROGMEM لتوفير مساحة الـ RAM في الشريحة
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <title>Ahmed Mohamed Mohamed Ahmed - Core System</title>
    <style>
    body { font-family: 'Consolas', 'Segoe UI', monospace; background-color: #0d1117; color: #c9d1d9; text-align: center; margin: 0; padding: 15px;}
    h1 { color: #58a6ff; font-size: 22px; text-transform: uppercase; letter-spacing: 2px;}
    .sys-header { background-color: #1f2428; border: 1px solid #30363d; padding: 10px; margin-bottom: 20px; border-radius: 8px; color: #79c0ff; font-size: 14px;}
    .card { background-color: #161b22; border: 1px solid #30363d; border-radius: 12px; padding: 20px; margin: 15px auto; max-width: 450px; box-shadow: 0 4px 12px rgba(0,0,0,0.5); }
    h3 { color: #8b949e; border-bottom: 1px dashed #30363d; padding-bottom: 8px; font-size: 18px;}
    .sensor-data { font-size: 16px; color: #ff7b72; margin-bottom: 15px;}
    .button { background-color: #238636; border: 1px solid rgba(240,246,252,0.1); color: white; padding: 12px 24px; border-radius: 6px; font-size: 16px; cursor: pointer; transition: 0.2s; width: 90%; font-weight: bold;}
    .button:hover { background-color: #2ea043; }
    .button2 { background-color: #21262d; border: 1px solid #363b42; color: #c9d1d9; padding: 12px 24px; border-radius: 6px; font-size: 16px; cursor: pointer; width: 90%;}
    .button3 { background-color: #da3633; border: none; color: white; padding: 10px 20px; border-radius: 6px; font-size: 14px; cursor: pointer; margin-top: 15px;}
    .footer { margin-top: 40px; font-size: 11px; color: #484f58; letter-spacing: 1px; }
    </style>
  </head>
  <body>
      <h1>Ahmed Mohamed Mohamed Ahmed</h1>
      
      <div class="sys-header">
        System Uptime: <span id="uptime">%sysUptime%</span> | Signal: <span id="rssi">%sysRSSI%</span>
      </div>

      <div class="card">
        <h3>Security Radar (PIR)</h3>
        <p class="sensor-data" id="pirTxt">Status: %pirStatus%</p>
        <p><a href='/set?button_id=1'><button id='btn1' class='%btn1class%'>Main Gate (Relay 1): %btn1txt%</button></a></p>
      </div>

      <div class="card">
        <h3>Environment Control (LDR)</h3>
        <p class="sensor-data" id="ldrTxt">Environment: %ldrStatus%</p>
        <p><a href='/set?button_id=2'><button id='btn2' class='%btn2class%'>Exterior Light (Relay 2): %btn2txt%</button></a></p>
      </div>

      <div class="card">
        <h3>Manual Overrides</h3>
        <p><a href='/set?button_id=3'><button id='btn3' class='%btn3class%'>System Relay 3: %btn3txt%</button></a></p>
        <p><a href='/set?button_id=4'><button id='btn4' class='%btn4class%'>System Relay 4: %btn4txt%</button></a></p>
      </div>

      <p><a href='/reset'><button class='button3'>Execute Master Reset</button></a></p>
      
      <div class="footer">
        Advanced Core V2.0 &copy; 2026<br>
        Engineered Exclusively by: Ahmed Mohamed Mohamed Ahmed
      </div>

      <script>
      // سكريبت الـ JavaScript لاستقبال البيانات الحية (SSE) من الشريحة
      if (!!window.EventSource) {
        var source = new EventSource('/events');
        
        // مستمع لحدث تغير الريلاي (تعديل لون الزرار والنص بدون عمل ريفريش)
        source.addEventListener('toggleState', function(e) {
          let jsonData = JSON.parse(e.data);
          const element = document.getElementById(jsonData.id);
          if(jsonData.status == 1){
            element.innerHTML = 'Relay ' + jsonData.id.replace('btn','') + ': OFF';
            element.className = "button2";    
          }else{
            element.innerHTML = 'Relay ' + jsonData.id.replace('btn','') + ': ON';
            element.className = "button";
          }
        }, false);
        
        // مستمع لحدث تغير الحساسات
        source.addEventListener('sensorUpdate', function(e) {
            let data = JSON.parse(e.data);
            document.getElementById('pirTxt').innerHTML = "Status: " + data.pir;
            document.getElementById('ldrTxt').innerHTML = "Environment: " + data.ldr;
        }, false);

        // مستمع لتحديث بيانات النظام (التليمتري)
        source.addEventListener('telemetryUpdate', function(e) {
            let data = JSON.parse(e.data);
            document.getElementById('uptime').innerHTML = data.up;
            document.getElementById('rssi').innerHTML = data.sig;
        }, false);
      }
      </script>
  </body>
  </html>
)rawliteral";

// [8] دالة التهيئة (Setup Routine)
void setup() {
  Serial.begin(115200);

  // إعداد بنات الريليهات كمخارج
  pinMode(d1.pin, OUTPUT); pinMode(d2.pin, OUTPUT);
  pinMode(d3.pin, OUTPUT); pinMode(d4.pin, OUTPUT);

  // إعداد بنات الأزرار الداخلية بمقاومة رفع (Pull-up)
  pinMode(d1.btnPin, INPUT_PULLUP); pinMode(d2.btnPin, INPUT_PULLUP);
  pinMode(d3.btnPin, INPUT_PULLUP); pinMode(d4.btnPin, INPUT_PULLUP);

  // إعداد بنات الحساسات كمداخل
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  // الاتصال بشبكة الواي فاي كـ Station (Client)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.println("\n[AMMA_CORE] Initializing Architecture...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n[AMMA_CORE] System Online.");
  Serial.print("[AMMA_CORE] Assigned IP: ");
  Serial.println(WiFi.localIP());

  // تفعيل خدمة mDNS للوصول للسيرفر بالاسم بدل الـ IP
  if (!MDNS.begin("amma-system")) { 
    Serial.println("DNS Initialization Failed!");
    while (1) { delay(1000); }
  }
  Serial.println("[AMMA_CORE] DNS Active: http://amma-system.local");
  MDNS.addService("http", "tcp", 80);

  // تعريف مسار الصفحة الرئيسية
  Ahmed_Mohamed_Mohamed_Ahmed_Server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // تعريف مسار استقبال أوامر التشغيل من الأزرار (/set?button_id=X)
  Ahmed_Mohamed_Mohamed_Ahmed_Server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("button_id")) {
      int idValue = request->getParam("button_id")->value().toInt();
      if (idValue == d1.id) togglePinState(&d1);
      else if (idValue == d2.id) togglePinState(&d2);
      else if (idValue == d3.id) togglePinState(&d3);
      else if (idValue == d4.id) togglePinState(&d4);
    }
    request->send_P(200, "text/html", index_html, processor);
  });

  // مسار إطفاء كل الأجهزة (Master Reset)
  Ahmed_Mohamed_Mohamed_Ahmed_Server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    resetAll();
    request->send_P(200, "text/html", index_html, processor);
  });

  // إعداد قناة الأحداث للعمل بمجرد اتصال المتصفح
  AMMA_Events.onConnect([](AsyncEventSourceClient *client) {
    client->send("Connected to AMMA_CORE", NULL, millis(), 10000);
  });
  Ahmed_Mohamed_Mohamed_Ahmed_Server.addHandler(&AMMA_Events);

  // إطلاق السيرفر
  Ahmed_Mohamed_Mohamed_Ahmed_Server.begin();
}

unsigned long lastTelemetryTime = 0;

// [9] دالة التشغيل المستمر (Main Loop) - تعتمد على Non-blocking Logic
void loop() {
  
  // 1. مراقبة الأزرار اليدوية (لو وُصلت مستقبلاً)
  int newBtn1State = digitalRead(d1.btnPin);
  int newBtn2State = digitalRead(d2.btnPin);
  int newBtn3State = digitalRead(d3.btnPin);
  int newBtn4State = digitalRead(d4.btnPin);

  // الكشف عن الضغطة (Falling Edge)
  if (d1.prevBtnState == 1 && newBtn1State == 0) togglePinState(&d1);
  if (d2.prevBtnState == 1 && newBtn2State == 0) togglePinState(&d2);
  if (d3.prevBtnState == 1 && newBtn3State == 0) togglePinState(&d3);
  if (d4.prevBtnState == 1 && newBtn4State == 0) togglePinState(&d4);

  d1.prevBtnState = newBtn1State;
  d2.prevBtnState = newBtn2State;
  d3.prevBtnState = newBtn3State;
  d4.prevBtnState = newBtn4State;

  // 2. منطق التحكم بحساس الإضاءة (LDR Logic)
  ldrState = digitalRead(LDR_PIN);
  if (ldrState != lastLdrState) {
    // إذا أظلمت البيئة والريلاي مطفي، شغله
    if (ldrState == LOW) { 
      if(d2.status == 0) togglePinState(&d2);
    } else {
    // إذا نورت البيئة والريلاي شغال، إطفيه
      if(d2.status == 1) togglePinState(&d2);
    }
    sendSensorUpdate(); // تحديث شاشة الموبايل فوراً
    lastLdrState = ldrState;
  }

  // 3. منطق التحكم بحساس الحركة (PIR Logic)
  pirState = digitalRead(PIR_PIN);
  if (pirState == HIGH) {
    pirTimer = millis(); // تصفير العداد طالما هناك حركة
    if (lastPirState == LOW) {
      if(d1.status == 0) togglePinState(&d1); 
      sendSensorUpdate();
      lastPirState = HIGH;
    }
  } else {
    // إذا توقفت الحركة، انتظر الوقت المحدد (pirDelay) قبل الإطفاء
    if (lastPirState == HIGH && (millis() - pirTimer > pirDelay)) {
      if(d1.status == 1) togglePinState(&d1); 
      sendSensorUpdate();
      lastPirState = LOW;
    }
  }

  // 4. بث بيانات التليمتري (Uptime & RSSI) كل 10 ثواني
  if(millis() - lastTelemetryTime > 10000) {
    sendTelemetryUpdate();
    lastTelemetryTime = millis();
  }

  delay(20); // تأخير بسيط لضمان استقرار الشريحة (Watchdog feeding)
}

// [10] الدوال المساعدة (Helper/Action Functions)

// دالة تغيير حالة الريلاي
void togglePinState(struct device *d) {
  if (d->status == 0) {
    digitalWrite(d->pin, HIGH); // تشغيل الريلاي
    d->status = 1;
  } else {
    digitalWrite(d->pin, LOW);  // إطفاء الريلاي
    d->status = 0;
  }
  // تطبيق Source Coding: ضغط حالة الريلاي في قالب JSON قبل الإرسال
  char AMMA_Data[500];
  snprintf(AMMA_Data, 500, "{\"id\":\"btn%d\", \"status\":%d}", d->id, d->status);
  AMMA_Events.send(AMMA_Data, "toggleState", millis());
}

// دالة إرسال حالة الحساسات للواجهة
void sendSensorUpdate() {
  char sensorData[200];
  const char* pStatus = pirState == HIGH ? "Motion Detected" : "Area Clear";
  const char* lStatus = ldrState == LOW ? "Dark Mode Active" : "Daylight Mode";
  snprintf(sensorData, 200, "{\"pir\":\"%s\", \"ldr\":\"%s\"}", pStatus, lStatus);
  AMMA_Events.send(sensorData, "sensorUpdate", millis());
}

// دالة إرسال بيانات النظام (وقت التشغيل وقوة الإشارة)
void sendTelemetryUpdate() {
  char telemetryData[150];
  // استخدام millis() وتحويلها لدقائق، وقراءة WiFi.RSSI()
  snprintf(telemetryData, 150, "{\"up\":\"%lu Mins\", \"sig\":\"%d dBm\"}", (millis() / 60000), WiFi.RSSI());
  AMMA_Events.send(telemetryData, "telemetryUpdate", millis());
}

// دالة الإطفاء الشامل (تُستدعى عند ضغط زر Master Reset)
void resetAll() {
  if (d1.status == 1) togglePinState(&d1);
  if (d2.status == 1) togglePinState(&d2);
  if (d3.status == 1) togglePinState(&d3);
  if (d4.status == 1) togglePinState(&d4);
}
