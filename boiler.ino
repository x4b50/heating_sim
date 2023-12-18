#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// /* display
#define TFT_CS        10
#define TFT_RST        8 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         9

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// */

#define HLED 2
#define BLED 3
#define W_PIN 7
#define H_PIN A4
#define B_PIN A5

#define H_MAX 30
#define B_MAX 100

#define WOLTAIKA_ZAWSZE
#define DAY() (hour >= morning && hour <= evening)
#define NIGHT() (hour <= morning || hour > evening)
#define FRAME 330

int w_thr = 250;
int b_thr = 80;
int h_thr_h = 22;
int h_thr_l = 17;
int morning = 6 *60;
int evening = 22*60;

int wolt;
int b_temp;
int h_temp;
int hour;
unsigned long start;

unsigned long frame_start;
unsigned long now;

enum State {
    OFF,
    ON,
};

enum State boiler = OFF;
enum State heating = ON;

void setup() {
    Serial.begin(9600);
    pinMode(HLED, OUTPUT);
    pinMode(BLED, OUTPUT);
    pinMode(W_PIN, INPUT);

    tft.initR(INITR_BLACKTAB);
    tft.initR(INITR_MINI160x80);
    tft.setRotation(3);

    tft.fillScreen(ST77XX_WHITE);
    tft.setTextSize(2);

    start = millis()/FRAME;
    // start at five o'clock
    start = start - 300;
}

void loop() {
    frame_start = millis();
    now = millis();

    time_sim();
    get_measures();
    heating_check();
    boiler_check();

    if (heating == ON) { digitalWrite(HLED, HIGH); }
    else { digitalWrite(HLED, LOW); }
    if (boiler == ON) { digitalWrite(BLED, HIGH); }
    else { digitalWrite(BLED, LOW); }

    /* debug
    Serial.print("time:\t");
    Serial.print(hour/60);
    Serial.print(":");
    Serial.println(hour%60);
    Serial.print("Woltaika:\t");
    Serial.println(wolt);
    Serial.print("boiler:\t");
    Serial.println(b_temp);
    Serial.print("heating:\t");
    Serial.println(h_temp);
    Serial.println();
    // */

    tft.fillScreen(ST77XX_WHITE);
    tft.setCursor(0,2);
    tft.setTextColor(ST7735_CYAN);
    tft.print(hour/60);
    tft.print(":");
    if (hour%60 < 10) {
        tft.print("0");
    }
    tft.print(hour%60);
    if NIGHT() {
        tft.println(" NOC  ");
    } else {
        tft.println(" DZIEN");
    }

    tft.setTextColor(ST77XX_MAGENTA);
    tft.print("dom: ");
    if (h_temp < 10) {
        tft.print("0");
    }
    tft.println(h_temp);

    tft.setTextColor(ST7735_RED);
    tft.print("boiler: ");
    if (b_temp < 10) {
        tft.print("00");
    } else if (b_temp < 100) {
        tft.print("0");
    }
    tft.print(b_temp);
    while (now - frame_start < FRAME) {
        Serial.println(now-frame_start);
        now = millis();
        delay(10);
    }
}

inline void get_measures() {
    wolt = !digitalRead(W_PIN) * w_thr;
    // low resolutin 'cause uno is probably 16 bit (I think)
    // b_temp = analogRead(B_PIN) * (B_MAX/10)/1024*10;
    b_temp = (analogRead(B_PIN)+1) /(1024/B_MAX);
    h_temp = (analogRead(H_PIN) * H_MAX +1)/1024;
}

inline void time_sim() {
    unsigned long now = millis()/FRAME;
    hour = (now-start);
    if (hour >= 24 * 60) {
        hour = 0;
    }
}

void heating_check() {
    if (heating == OFF) {
        if (h_temp > h_thr_l) {
            return;
        }
        heating = ON;
    } else {
        if (h_temp < h_thr_h) {
            return;
        }
        heating = OFF;
    }
}

void boiler_check() {
    if (boiler == ON) {
        if NIGHT() {
            boiler = OFF;
            return;
        }
        if (b_temp >= b_thr) {
            boiler = OFF;
            return;
        }
        if (h_temp <= h_thr_l) {
            boiler = OFF;
            return;
        }
        if (heating == ON) {
            boiler = OFF;
            return;
        }
    } else {
        if (wolt >= w_thr) {
            boiler = ON;
            return;
        }
        if NIGHT() {
            return;
        }
        if (heating == ON) {
            return;
        }
        if (h_temp <= h_thr_l) {
            return;
        }
        if (b_temp >= b_thr) {
            return;
        }
        boiler = ON;
    }
}
