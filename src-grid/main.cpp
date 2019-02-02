/**********************************************************
 * Author: Nanne Huiges
 * Based on  ledring demo by Niels Maneschijn
 * https://github.com/nielsmaneschijn/lotlr
 * 
 * Wemos D1 mini module connection to Led strip/ring:
 *    5V -> +5 (red. power)
 *    G -> - (black. ground)
 *    RX -> DI (other. data)
 * 
 ************************************************************/
// Inbuild AP with webserver @ 192.168.4.1 if no network is found.
#include <WiFiManager.h>
// ws2812b ledstrip/ring
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
// this includes some form of time.h it seems
#include <NtpClientLib.h>
// http client
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// the 437 character definitions.
#include "font_8x8.h"
#include "font_4x7.h"

// todo: include this?


// 'brightness' of the led. 0 - 255 (?)
#define colorSaturation 32

const char *SSID = "ledgrid clock";

// set the matrix
typedef ColumnMajorAlternatingLayout MyPanelLayout;
const uint8_t PanelWidth = 16; // 8 pixel x 8 pixel matrix of leds
const uint8_t PanelHeight = 16;
const uint16_t PixelCount = PanelWidth * PanelHeight;
const uint8_t PixelPin = 2; // make sure to set this to the correct pin, ignored for Esp8266
NeoTopology<MyPanelLayout> topo(PanelWidth, PanelHeight);
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);
// 4 quadrants
const int lefttop = 0b00;
const int leftbot = 0b10;
const int righttop = 0b01;
const int rightbot = 0b11;

// 8 octants, from lefttop clockwise
const int oct1 = 0x00;
const int oct2 = 0x01;
const int oct3 = 0x02;
const int oct4 = 0x03;
const int oct5 = 0x10;
const int oct6 = 0x12;
const int oct7 = 0x13;
const int oct8 = 0x14;


// Define some colors for easy use later on
RgbColor red(colorSaturation, 0, 0);
RgbColor pink(colorSaturation, 0, colorSaturation / 8);
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation / 2, 0);
RgbColor purple(colorSaturation / 8, 0, colorSaturation);
RgbColor green(0, colorSaturation, 0);
RgbColor lgreen(colorSaturation / 4, colorSaturation / 2, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

// the 'current' leds to write to the matrix
RgbColor leds[PanelHeight][PanelWidth] = {green};

void setup_wifi()
{
    // maak een access point om je wifi netwerk in te kunnen stellen
    WiFiManager wifiManager;
    wifiManager.autoConnect(SSID);
    Serial.println(WiFi.localIP());
}

/**
 * Arduino convention: setup() gets called once
 * during start.
 */
void setup()
{
    // Initialize the BUILTIN_LED pin as an output, turn it on during setup
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(115200);
    while (!Serial)
        ; // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    setup_wifi();
    strip.Begin();
    strip.Show();
    NTP.begin("pool.ntp.org", 1, true, 0);
    NTP.setInterval(10, 3600);
    NTP.getTime();
    strip.ClearTo(black);
    strip.Show();
    Serial.println();
    Serial.println("Running...");
    digitalWrite(LED_BUILTIN, HIGH);
}

void show()
{
    for (int x = 0; x < PanelWidth; x++)
    {
        for (int y = 0; y < PanelHeight; y++)
        {
            strip.SetPixelColor(topo.Map(x, y), leds[y][x]);
        }
    }
    strip.Show();
}

void set_character_8x8(unsigned char letter, RgbColor color, int quadrant)
{
    bool quadrant_x = ((quadrant >> 0) & 0x01);
    bool quadrant_y = ((quadrant >> 1) & 0x01);
    for (byte x = 0; x < 8; x++)
    {
        byte row_byte = font_8x8[letter][x];
        for (byte y = 0; y < 8; y++)
        {
            bool led = ((row_byte >> y) & 0x01);
            if (led == 1)
            {
                leds[y + (8 * quadrant_y)][x + (8 * quadrant_x)] = color;
            }
            else
            {
                leds[y + (8 * quadrant_y)][x + (8 * quadrant_x)] = black;
            }
        }
    }
}

void set_character_4x7(unsigned char letter, RgbColor color, int octant)
{
    int octant_x = ((octant >> 0) & 0x01);
    int octant_y = ((octant >> 1) & 0x01);
    for (byte x = 0; x < 4; x++)
    {
        byte row_byte = font_4x7[letter][x];
        for (byte y = 0; y < 7; y++)
        {
            bool led = ((row_byte >> y) & 0x01);
            if (led == 1)
            {
                leds[y + (8 * octant_y)][x + (4 * octant_x)] = color;
            }
            else
            {
                leds[y + (8 * octant_y)][x + (4 * octant_x)] = black;
            }
        }
    }
}

void set_sec(int sec, RgbColor color)
{
    for (int y = 0; y < sec % 10; y++)
    {
        leds[y][PanelWidth - 1] = color;
    }
    for (int y = 0; y < sec / 10; y++)
    {
        leds[y][PanelWidth - 2] = color;
    }
}

bool should_display()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        HTTPClient http;
        String url = "http://192.168.178.2:8084/json.htm?type=devices&rid=158";
        http.begin(url);           
        int httpCode = http.GET(); 
        if (httpCode == 200)
        {                                      
            String payload = http.getString(); 
            DynamicJsonBuffer jsonBuffer;
            JsonObject &root = jsonBuffer.parseObject(payload);
            // Unfortunately, the following doesn't work (issue #118):
            // sensor = root["sensor"]; // <-  error "ambiguous overload for 'operator='"
            // As a workaround, you need to replace by
            String data = root["result"][0]["Data"].as<String>();
            return data == "On";
        }
        Serial.println("no status 200 from domoticz");
        return false;
    }
    Serial.println("no wifi connected");
    return false;
}

/**
 * Arduino convention: the loop gets called in a loop.
 */
void loop()
{

    if (should_display())
    {
        time_t t = now();

        int h = (t / 3600) % 24;
        char strh1[2];
        sprintf(strh1, "%d", h / 10);
        char strh2[2];
        sprintf(strh2, "%d", h % 10);

        int m = (t / 60) % 60;
        char strm1[2];
        sprintf(strm1, "%d", m / 10);
        char strm2[2];
        sprintf(strm2, "%d", m % 10);

        set_character_8x8(strh1[0], blue, lefttop);
        set_character_8x8(strh2[0], blue, righttop);
        set_character_8x8(strm1[0], purple, leftbot);
        set_character_8x8(strm2[0], purple, rightbot);

        // int s = t % 60;
        // set_sec(s, lgreen);

        show();
    } else {
        strip.ClearTo(black);
        strip.Show();
    }
    delay(1000);
}