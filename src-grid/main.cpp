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
#include <WiFiManager.h>
// ws2812b ledstrip/ring
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>

// 'brightness' of the led. 0 - 255 (?)
#define colorSaturation 32

typedef ColumnMajorAlternatingLayout MyPanelLayout;

const uint8_t PanelWidth = 16; // 8 pixel x 8 pixel matrix of leds
const uint8_t PanelHeight = 16;
const uint16_t PixelCount = PanelWidth * PanelHeight;
const uint8_t PixelPin = 2; // make sure to set this to the correct pin, ignored for Esp8266

NeoTopology<MyPanelLayout> topo(PanelWidth, PanelHeight);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount, PixelPin);

// Define some colors for easy use later on
RgbColor red(colorSaturation, 0, 0);
RgbColor pink(colorSaturation, 0, colorSaturation / 8);
RgbColor yellow(colorSaturation, colorSaturation, 0);
RgbColor orange(colorSaturation, colorSaturation / 2, 0);
RgbColor purple(colorSaturation / 8, 0, colorSaturation);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

const uint16_t left = 0;
const uint16_t right = PanelWidth - 1;
const uint16_t top = 0;
const uint16_t bottom = PanelHeight - 1;

#define H                         \
    {                             \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}  \
    }

#define N \
    { \
        {0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}, \
        {0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0},  \
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0}  \
    }

const int numPatterns = 2;
byte patterns[numPatterns][16][16] = {N,H};

int pattern = 0;


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

    strip.Begin();
    strip.Show();

    Serial.println();
    Serial.println("Running...");

    digitalWrite(LED_BUILTIN, HIGH);
}

void setPattern(int pattern)
{
    for (int x = 0; x < PanelWidth; x++)
    {
        for (int y = 0; y < PanelHeight; y++)
        {
            if(patterns[pattern][y][x] == 1){
                strip.SetPixelColor(topo.Map(x, y), blue);
            }else{
                strip.SetPixelColor(topo.Map(x, y), black);
            }
        }
    }
}

/**
 * Arduino convention: the loop gets called in a loop.
 */
void loop()
{
    strip.ClearTo(black);
    strip.Show();
    setPattern(0);
    strip.Show();
    delay(2500);

    strip.ClearTo(black);
    strip.Show();
    setPattern(1);
    strip.Show();
    delay(2500);

    // strip.ClearTo(black);
    // strip.Show();

    // for (int x = 0; x < PanelWidth; x++)
    // {
    //     for (int y = 0; y < PanelHeight; y++)
    //     {
    //         Serial.print(x);
    //         Serial.print(',');
    //         Serial.print(y);
    //         Serial.println();
    //         strip.SetPixelColor(topo.Map(x, y), red);
    //         strip.Show();
    //         delay(100);
    //     }
    // }   
    // delay(2500);
 
}