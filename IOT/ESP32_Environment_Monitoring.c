/*
 * ESP32 Environmental Monitoring and Control System with PostgreSQL Integration
 * Arduino IDE Compatible Version
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ============ PIN DEFINITIONS ============
#define DHT22_PIN 4
#define DHT_TYPE DHT22
#define LCD_ADDRESS 0x27
#define LCD_COLS 16
#define LCD_ROWS 2
#define LIGHT_PIN 34
#define FAN_RELAY_PIN 26
#define LIGHT_RELAY_PIN 27
#define BUZZER_PIN 25
#define FAN_LED_PIN 32
#define LIGHT_LED_PIN 33
#define ALARM_LED_PIN 14

// ============ THRESHOLD VALUES ============
#define TEMP_HIGH 30.0
#define HUMIDITY_HIGH 70.0
#define LIGHT_LOW 500

// ============ TIMING CONFIGURATION ============
#define SENSOR_READ_INTERVAL 2000 // Read sensors every 2 seconds

// ============ GLOBAL OBJECTS ============
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);
DHT dht(DHT22_PIN, DHT_TYPE);

// ============ LED CONTROL FUNCTIONS ============
void setFanLED(bool state)
{
    digitalWrite(FAN_LED_PIN, state ? HIGH : LOW);
}

void setLightLED(bool state)
{
    digitalWrite(LIGHT_LED_PIN, state ? HIGH : LOW);
}

void setAlarmLED(bool state)
{
    digitalWrite(ALARM_LED_PIN, state ? HIGH : LOW);
}

// ============ RELAY & ACTUATOR CONTROL FUNCTIONS ============
void controlFan(bool state)
{
    digitalWrite(FAN_RELAY_PIN, state ? LOW : HIGH);
    setFanLED(state);
    Serial.print("FAN: ");
    Serial.println(state ? "ON" : "OFF");
}

void controlLight(bool state)
{
    digitalWrite(LIGHT_RELAY_PIN, state ? LOW : HIGH);
    setLightLED(state);
    Serial.print("LIGHT: ");
    Serial.println(state ? "ON" : "OFF");
}

void controlBuzzer(bool state)
{
    digitalWrite(BUZZER_PIN, state ? HIGH : LOW);
    setAlarmLED(state);
    Serial.print("BUZZER: ");
    Serial.println(state ? "ON" : "OFF");
}

// ============ SENSOR READING FUNCTIONS ============
bool readDHT22(float &temperature, float &humidity)
{
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("ERROR: Failed to read from DHT22 sensor!");
        return false;
    }
    return true;
}

int readLightLevel()
{
    return analogRead(LIGHT_PIN);
}

void writeALineOnLCD(const char *str)
{
    lcd.clear();
    lcd.print(str);
}
void displayOnLCD(float temp, float humidity, int light)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp, 1);
    lcd.print("C H:");
    lcd.print(humidity, 1);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print("Light: ");
    lcd.print(light);
}

// ============ SETUP FUNCTION ============
void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println(" Environmental Control System Started");
    Serial.println(" WITH PostgreSQL Integration");
    Serial.println("========================================");
    Serial.println("Hardware Configuration:");
    Serial.println("  DHT22: GPIO15");
    Serial.println("  Fan Relay: GPIO26 | Fan LED: GPIO32");
    Serial.println("  Light Relay: GPIO27 | Light LED: GPIO33");
    Serial.println("  Buzzer: GPIO25 | Alarm LED: GPIO14");
    Serial.println("  LDR: GPIO34 (ADC)");
    Serial.println("  LCD: I2C (SDA=21, SCL=22)");
    Serial.println("========================================");

    // Initialize I2C LCD
    // Wire.begin();
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    writeALineOnLCD("Environmental Control System Started");

    delay(1000);

    Serial.println("✓ LCD initialized");
    writeALineOnLCD("✓ LCD initialized");

    // Initialize DHT22 sensor
    dht.begin();
    Serial.println("✓ DHT22 initialized");
    writeALineOnLCD("✓ DHT22 initialized");
    delay(2000);

    // Initialize ADC
    analogReadResolution(12);
    Serial.println("✓ ADC initialized");
    writeALineOnLCD("✓ ADC initialized");

    // Initialize relay GPIO pins
    pinMode(FAN_RELAY_PIN, OUTPUT);
    pinMode(LIGHT_RELAY_PIN, OUTPUT);
    digitalWrite(FAN_RELAY_PIN, HIGH);
    digitalWrite(LIGHT_RELAY_PIN, HIGH);
    Serial.println("✓ Relays initialized (all OFF)");
    writeALineOnLCD("✓ Relays initialized (all OFF)");

    // Initialize buzzer GPIO
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    Serial.println("✓ Buzzer initialized (OFF)");
    writeALineOnLCD("✓ Buzzer initialized (OFF)");

    // Initialize LED indicator pins
    pinMode(FAN_LED_PIN, OUTPUT);
    pinMode(LIGHT_LED_PIN, OUTPUT);
    pinMode(ALARM_LED_PIN, OUTPUT);
    digitalWrite(FAN_LED_PIN, LOW);
    digitalWrite(LIGHT_LED_PIN, LOW);
    digitalWrite(ALARM_LED_PIN, LOW);
    Serial.println("✓ LED indicators initialized (all OFF)");
    writeALineOnLCD("✓ LED indicators initialized (all OFF)");

    Serial.println("Hardware initialization complete!");
    writeALineOnLCD("Hardware initialization complete!");

    // Connect to WiFi
    connectToWiFi();

    Serial.println("\nSystem Ready!\n");
    lcd.clear();
    writeALineOnLCD("System Ready");
    delay(1000);
}

// ============ MAIN LOOP ============
void loop()
{
    float temperature = 0.0;
    float humidity = 0.0;
    int lightLevel = 0;
    bool fanStatus = false;
    bool fanLedStatus = false;
    bool lightStatus = false;
    bool lightLedStatus = false;
    bool alarmLedStatus = false;
    bool buzzerStatus = false;

    // Read DHT22 sensor
    bool sensorSuccess = readDHT22(temperature, humidity);

    if (sensorSuccess)
    {
        // Read light level
        lightLevel = readLightLevel();

        // Display on LCD
        displayOnLCD(temperature, humidity, lightLevel);

        // Log to Serial Monitor
        Serial.print("Temp: ");
        Serial.print(temperature, 1);
        Serial.print("°C, Humidity: ");
        Serial.print(humidity, 1);
        Serial.print("%, Light: ");
        Serial.println(lightLevel);

        // ============ CONTROL LOGIC ============

        // Fan control
        if (temperature >= TEMP_HIGH)
        {
            controlFan(true);
            fanStatus = true;
            fanLedStatus = true;
        }
        else
        {
            if (temperature < TEMP_HIGH)
            {
                controlFan(false);
                fanStatus = false;
                fanLedStatus = false;
            }
        }

        // Light control
        if (lightLevel < LIGHT_LOW)
        {
            controlLight(true);
            lightStatus = true;
            lightLedStatus = true;
        }
        else
        {
            controlLight(false);
            lightStatus = false;
            lightLedStatus = false;
        }

        // Alarm control
        if ((temperature >= TEMP_HIGH || humidity >= HUMIDITY_HIGH) &&
            lightLevel < LIGHT_LOW)
        {
            controlBuzzer(true);
            buzzerStatus = true;
            alarmLedStatus = true;
        }
        else
        {
            controlBuzzer(false);
            buzzerStatus = false;
            alarmLedStatus = false;
        }
        // ============ SEND DATA TO SERVER ============
        // Only send data if:
        // 1. Sensors read successfully (we're here in this if block)
        // 2. Enough time has passed since last send
        unsigned long currentTime = millis();
        if (currentTime - lastSendTime >= DATA_SEND_INTERVAL)
        {
            bool sendSuccess = sendDataToServer(
                temperature,    // temperature (numeric 5,2)
                humidity,       // humidity (numeric 5,2)
                lightLevel,     // light_intensity (numeric 5,2)
                fanStatus,      // fan (boolean)
                fanLedStatus,   // fan_led (boolean)
                lightStatus,    // light (boolean)
                lightLedStatus, // light_led (boolean)
                alarmLedStatus, // alram_led (boolean) - matches your typo
                buzzerStatus    // buzzer (boolean)
            );

            // Brief visual feedback on LCD
            if (sendSuccess)
            {
                lcd.setCursor(15, 1);
                lcd.print("*"); // Success indicator
            }
            else
            {
                lcd.setCursor(15, 1);
                lcd.print("X"); // Failed indicator
            }

            lastSendTime = currentTime;
        }
    }
    else
    {
        // ============ SENSOR READ FAILED ============
        // DO NOT send data to server when sensor fails
        Serial.println("⚠ Sensor read failed. NOT sending data to server.");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor Error!");
        lcd.setCursor(0, 1);
        lcd.print("No data sent");
    }

    delay(SENSOR_READ_INTERVAL);
}