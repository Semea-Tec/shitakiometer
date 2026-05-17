#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebServer.h>

// Pins for Heltec LoRa V3 (ESP32-S3)
#define SCK_LORA 9
#define MISO_LORA 11
#define MOSI_LORA 10
#define SS_LORA 8
#define RST_LORA 12
#define BUSY_LORA 13
#define DIO1_LORA 14

#define OLED_SDA 17
#define OLED_SCL 18
#define OLED_RST 21
#define VEXT_PIN 36 // GPIO 36 controls Vext on V3
#define LED_PIN 35  // GPIO 35 is the white LED

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BAND 915.0 // MHz

// WiFi Configuration
const char *ssid = ".....";
const char *password = ".....";

// Web Server (HTTP) on port 80
WebServer webServer(80);

// Data storage for latest sensor reading (typed)
struct SensorData
{
    float temperature; // degrees C
    float humidity;    // percent
    int co2;           // ppm
    int rssi;          // dBm
    long timestamp;    // millis
} latestData = {0.0, 0.0, 0, 0, 0};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);
SX1262 radio = new Module(SS_LORA, DIO1_LORA, RST_LORA, BUSY_LORA);

// MOCK MODE: enable to periodically generate fake data for testing the HTTP API
// Set to false to restore normal operation (only real LoRa data)
#define MOCK_MODE false
const unsigned long MOCK_INTERVAL = 5000; // ms between mock samples
unsigned long lastMockMillis = 0;

// Helper: parse incoming LoRa string into `latestData`.
// Expected formats: "temperature:25.3,humidity:40.2,co2:412" or any subset.
void parseSensorString(const String &s, SensorData &out)
{
    int start = 0;
    while (start < s.length())
    {
        int comma = s.indexOf(',', start);
        String token;
        if (comma == -1)
        {
            token = s.substring(start);
            start = s.length();
        }
        else
        {
            token = s.substring(start, comma);
            start = comma + 1;
        }
        token.trim();
        int colon = token.indexOf(':');
        if (colon == -1)
            continue;
        String key = token.substring(0, colon);
        String val = token.substring(colon + 1);
        key.toLowerCase();
        val.trim();
        if (key.indexOf("temp") != -1 || key == "t")
        {
            out.temperature = val.toFloat();
        }
        else if (key.indexOf("hum") != -1 || key == "h")
        {
            out.humidity = val.toFloat();
        }
        else if (key.indexOf("co2") != -1)
        {
            out.co2 = val.toInt();
        }
    }
}

// HTTP Request Handlers
void handleGetData()
{
    // Returns the latest sensor data as JSON
    String json = "{";
    json += "\"temperature\":" + String(latestData.temperature, 1) + ",";
    json += "\"humidity\":" + String(latestData.humidity, 1) + ",";
    json += "\"co2\":" + String(latestData.co2) + ",";
    json += "\"rssi\":" + String(latestData.rssi) + ",";
    json += "\"timestamp\":" + String(latestData.timestamp);
    json += "}";

    webServer.send(200, "application/json", json);
    Serial.println("[HTTP] GET /data - Data sent to client");
}

void handleRoot()
{
    // Returns a simple HTML page with connection info
    String html = "<!DOCTYPE html>";
    html += "<html><head><meta name='viewport' content='width=device-width'>";
    html += "<title>LoRa Receiver</title>";
    html += "<style>body{font-family:Arial;margin:20px;}";
    html += ".container{background:#f0f0f0;padding:20px;border-radius:5px;}";
    html += ".data{margin:10px 0;font-size:16px;}";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h1>LoRa Receiver V3</h1>";
    html += "<p>WiFi IP: " + WiFi.localIP().toString() + "</p>";
    html += "<div class='data'><strong>Temperature:</strong> " + String(latestData.temperature, 1) + " &deg;C</div>";
    html += "<div class='data'><strong>Humidity:</strong> " + String(latestData.humidity, 1) + " %</div>";
    html += "<div class='data'><strong>CO2:</strong> " + String(latestData.co2) + " ppm</div>";
    html += "<div class='data'><strong>RSSI:</strong> " + String(latestData.rssi) + " dBm</div>";
    html += "<p><a href='/data'>/data - Get JSON data</a></p>";
    html += "</div></body></html>";

    webServer.send(200, "text/html", html);
    Serial.println("[HTTP] GET / - Home page sent to client");
}

void handleNotFound()
{
    webServer.send(404, "text/plain", "Not Found");
}

// Flag para indicar que um pacote foi recebido via interrupção
volatile bool receivedFlag = false;

// Função de interrupção (ISR)
void setFlag(void)
{
    receivedFlag = true;
}

void setup_wifi()
{
    delay(10);
    Serial.println();

    // Garante que o WiFi inicie em um estado limpo (Station Mode)
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.println("Scanning for WiFi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        Serial.println("No networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.println(")");
            delay(10);
        }
    }
    Serial.println("");

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// Update OLED to show only the device title and the IP address
void showTitleAndIP()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("LoRa");
    display.setCursor(0, 16);
    display.println("Receiver");
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
}

void setup()
{
    Serial.begin(115200);

    // Turn on Vext power (V3 uses GPIO 36)
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW);
    delay(100);

    // Turn on built-in LED to show it's alive
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    // Manual OLED Reset
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(20);
    digitalWrite(OLED_RST, HIGH);

    // Initialize OLED
    Wire.begin(OLED_SDA, OLED_SCL);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        for (;;)
            ;
    }

    display.clearDisplay();
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(0xFF); // Max brightness
    display.dim(false);
    display.setTextColor(WHITE);
    // initial display will be updated to show title+IP after WiFi connects
    
    // Show title and IP on the OLED so user can connect
    showTitleAndIP();
    
    // Setup WiFi
    setup_wifi();

    // Initialize LoRa
    Serial.print(F("[LoRa] Initializing ... "));
    int state = radio.begin(BAND);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }

    // Configura a interrupção para quando receber um pacote
    radio.setDio1Action(setFlag);

    // Começa a escutar (modo não bloqueante)
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("[LoRa] Listening for packets..."));
    }
    else
    {
        Serial.print(F("[LoRa] Failed to start receive, code "));
        Serial.println(state);
    }

    // Setup HTTP Web Server
    webServer.on("/", handleRoot);
    webServer.on("/data", handleGetData);
    webServer.onNotFound(handleNotFound);
    webServer.begin();
    Serial.print("[HTTP] Web server started on http://");
    Serial.print(WiFi.localIP());
    Serial.println(":80");
    
}

void loop()
{
    // No MQTT in this build: just ensure LoRa listening stays active
    // (radio.startReceive will be called again at end of packet handling)

    // Handle incoming HTTP requests
    webServer.handleClient();

    // If MOCK_MODE is enabled, periodically generate fake data so the
    // `/data` endpoint can be tested without real LoRa packets.
    if (MOCK_MODE)
    {
        unsigned long now = millis();
        if (now - lastMockMillis >= MOCK_INTERVAL)
        {
            lastMockMillis = now;
            float mt = random(200, 350) / 10.0; // 20.0 - 34.9 C
            float mh = random(300, 700) / 10.0; // 30.0 - 70.0 %
            int mco2 = random(350, 800);        // ppm
            Serial.printf("[MOCK] Generating mock data: T=%.1f H=%.1f CO2=%d\n", mt, mh, mco2);

            // Store mock data
            latestData.temperature = mt;
            latestData.humidity = mh;
            latestData.co2 = mco2;
            latestData.rssi = -50; // reasonable fixed RSSI for mock
            latestData.timestamp = now;

            // Keep OLED showing only title+IP (do not display sensor values)
            showTitleAndIP();
        }
    }

    // Verifica se a flag de interrupção foi ativada
    if (receivedFlag)
    {
        // Reseta a flag
        receivedFlag = false;

        String str;
        // Lê os dados recebidos
        int state = radio.readData(str);

        if (state == RADIOLIB_ERR_NONE)
        {
            Serial.println(F("[LoRa] Data received!"));
            Serial.print(F("[LoRa] Data:\t\t"));
            Serial.println(str);

            // Parse and store data for HTTP requests
            parseSensorString(str, latestData);
            latestData.rssi = radio.getRSSI();
            latestData.timestamp = millis();

            // // Keep OLED showing only title+IP (do not display sensor values)
            // showTitleAndIP();

            // 5. Atualiza o Display OLED
            display.clearDisplay();
            display.setTextSize(1);
            display.setCursor(0, 0);
            display.println("SENDER V3");

            display.setCursor(0, 12);
            display.print("Temp: ");
            if (!isnan(latestData.temperature))
            {
                display.print(latestData.temperature);
                display.print(" C");
            }
            else
            {
                display.print("ERRO");
            }

            display.setCursor(0, 24);
            display.print("Umid: ");
            if (!isnan(latestData.humidity))
            {
                display.print(latestData.humidity);
                display.print(" %");
            }
            else
            {
                display.print("ERRO");
            }

            display.setCursor(0, 36);
            display.print("CO2:  ");
            if (latestData.co2 != -1)
            {
                display.print(latestData.co2);
                display.print(" ppm");
            }
            else
            {
                display.print("ERRO");
            }

            display.setCursor(0, 50);
            display.print("LoRa: ");
            display.println(state == RADIOLIB_ERR_NONE ? "OK" : "FALHA");

            display.display();
        }

        // Volta a escutar novos pacotes
        radio.startReceive();
    }
}
