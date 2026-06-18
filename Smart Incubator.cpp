#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <ESP32Servo.h>

#define DHTPIN 23
#define DHTTYPE DHT22

#define BTN_UP 5
#define BTN_DOWN 18
#define BTN_SET 19

#define RELAY_HEATER 26
#define RELAY_WATER 25
#define RELAY_COOL 33
#define RELAY_GAS 32

#define GAS_PIN 34

#define SERVO_PIN 12

#define BUZZER 4


LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);
Servo servo;


int temp = 0, humi = 0;

int setTemp = 25;
int setHumi = 30;
int servoInterval = 10;

int mode = 0;

unsigned long lastSensor = 0;
unsigned long lastServo = 0;

int servoPos = 40;
bool servoState = false;

void setup()
{
    Serial.begin(115200);

    lcd.init();
    lcd.backlight();

    dht.begin();

    pinMode(BTN_UP, INPUT_PULLDOWN);
    pinMode(BTN_DOWN, INPUT_PULLDOWN);
    pinMode(BTN_SET, INPUT_PULLDOWN);

    pinMode(RELAY_HEATER, OUTPUT);
    pinMode(RELAY_WATER, OUTPUT);
    pinMode(RELAY_COOL, OUTPUT);
    pinMode(RELAY_GAS, OUTPUT);

    pinMode(BUZZER, OUTPUT);

    servo.attach(SERVO_PIN);
    servo.write(70);

    lcd.setCursor(0, 0);
    lcd.print("Incubator Ready");
    delay(2000);
    lcd.clear();

    setTemp = dht.readTemperature();
    setHumi = dht.readHumidity();

    delay(1000);
}

void loop()
{

    //BUTTON
    if (digitalRead(BTN_SET) == HIGH)
    {
        digitalWrite(BUZZER, HIGH);
        delay(50);
        digitalWrite(BUZZER, LOW);
        mode++;
        if (mode > 2) mode = 0;
        delay(250);
    }

    if (digitalRead(BTN_UP) == HIGH)
    {
        digitalWrite(BUZZER, HIGH);
        delay(50);
        digitalWrite(BUZZER, LOW);
        if (mode == 0) setTemp++;
        if (mode == 1) setHumi++;
        if (mode == 2) servoInterval++;
        delay(150);
    }

    if (digitalRead(BTN_DOWN) == HIGH)
    {
        digitalWrite(BUZZER, HIGH);
        delay(50);
        digitalWrite(BUZZER, LOW);
        if (mode == 0) setTemp--;
        if (mode == 1) setHumi--;
        if (mode == 2) servoInterval--;
        if (servoInterval < 5) servoInterval = 5;
        delay(150);
    }

    //READ SENSORS
    if (millis() - lastSensor > 1500)
    {
        lastSensor = millis();
        temp = dht.readTemperature();
        humi = dht.readHumidity();

        // int gasValue = analogRead(GAS_PIN);

        // // Gas control
        // if (gasValue > 1500) {
        //   digitalWrite(RELAY_GAS, LOW);
        // } else {
        //   digitalWrite(RELAY_GAS, HIGH);
        // }

        static bool gasDetected = false;
        static unsigned long gasStartTime = 0;

        int gasValue = analogRead(GAS_PIN);

        if (gasValue > 1500 && !gasDetected)
        {
            gasDetected = true;
            gasStartTime = millis();


            digitalWrite(RELAY_GAS, LOW);


            for (int i = 0; i < 5; i++)
            {
                digitalWrite(BUZZER, HIGH);
                delay(100);
                digitalWrite(BUZZER, LOW);
                delay(100);
            }

            Serial.println("GAS DETECTED! Fan ON for 10 seconds");
        }

        if (gasDetected)
        {
            if (millis() - gasStartTime >= 10000)
            {
                gasDetected = false;

                digitalWrite(RELAY_GAS, HIGH);

                Serial.println("Gas cleared. Fan OFF");
            }
        }


        if (!gasDetected && gasValue <= 1500)
        {
            digitalWrite(RELAY_GAS, HIGH);
        }


        if (temp < setTemp) digitalWrite(RELAY_HEATER, LOW);
        else digitalWrite(RELAY_HEATER, HIGH);


        if (humi < setHumi) digitalWrite(RELAY_WATER, LOW);
        else digitalWrite(RELAY_WATER, HIGH);


        if (temp > setTemp) digitalWrite(RELAY_COOL, LOW);
        else digitalWrite(RELAY_COOL, HIGH);


        Serial.print("Temp: ");
        Serial.print(temp);
        Serial.print("  Humi: ");
        Serial.print(humi);
        Serial.print("  Gas: ");
        Serial.println(gasValue);
        Serial.print("Set Temp: ");
        Serial.print(setTemp);
        Serial.print("  Set Humi: ");
        Serial.println(setHumi);
        Serial.print("Servo Interval: ");
        Serial.println(servoInterval);
        Serial.println("----------------------");
    }

    //SERVO MOVEMENT
    if (millis() - lastServo > servoInterval * 1000)
    {
        lastServo = millis();

        if (servoState == false)
        {
            slowMove(70, 120, 20);
            servoState = true;
        }
        else
        {
            slowMove(120, 70, 20);
            servoState = false;
        }
    }


    //LCD UPDATE
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temp);
    lcd.print(char(223));
    lcd.print("C");

    lcd.print(" H:");
    lcd.print(humi);
    lcd.print("%");


    int remaining = (servoInterval * 1000 - (millis() - lastServo)) / 1000;
    if (remaining < 0) remaining = 0;

    lcd.print(" ");
    lcd.print(remaining);
    lcd.print("s ");


    lcd.setCursor(0, 1);
    if (mode == 0) lcd.print("Set Tem:");
    else if (mode == 1) lcd.print("Set Humi:");
    else lcd.print("Set Time:");

    if (mode == 0)
    {
        lcd.print(setTemp);
        lcd.print(char(223));
        lcd.print("C");
    }
    else if (mode == 1)
    {
        lcd.print(setHumi);
        lcd.print("%");
    }
    else if (mode == 2)
    {
        lcd.print(servoInterval);
        lcd.print("s");
    }


    lcd.print("     ");
}


void slowMove(int startAngle, int endAngle, int stepDelay)
{
    if (startAngle < endAngle)
    {
        for (int pos = startAngle; pos <= endAngle; pos++)
        {
            servo.write(pos);
            delay(stepDelay);
        }
    }
    else
    {
        for (int pos = startAngle; pos >= endAngle; pos--)
        {
            servo.write(pos);
            delay(stepDelay);
        }
    }
}