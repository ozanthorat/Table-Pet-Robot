/*
 * Table Pet Robot - Direct Port Manipulation Version
 *
 * Hardware:
 * - Arduino Uno (Clone)
 * - OLED 128x64 I2C (SH110X Driver)
 * - Active Buzzer (Pin 2) - Needs only HIGH to beep
 * - Touch Sensor (Pin 9)
 * - Sound Sensor (Analog A0)
 * - Potentiometer (Analog A1)
 *
 * Libraries:
 * - FluxGarage_RoboEyes.h
 * - Adafruit_SH110X.h
 * - Wire.h
 */
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <FluxGarage_RoboEyes.h>
#include <Wire.h>
// --- Configuration ---
#define I2C_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Sound Threshold to trigger "Scared" face
#define SOUND_THRESHOLD 600
// Timings
#define SLEEP_TIMEOUT_MS 120000 // 2 Minutes
#define DOUBLE_TAP_MAX_GAP_MS 400
#define DEBOUNCE_MS 50
#define ALERT_TOTAL_DURATION 5000
#define ALERT_BUZZER_DURATION 3000 // 3 Seconds Beep
#define ALERT_DB_SWITCH_MS 3000    // Switch to dB text at 3 seconds
// --- Globals ---
Adafruit_SH1106G display =
    Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// Pass display reference
RoboEyes<Adafruit_SH1106G> eyes(display);
// State Variables
bool isScared = false;
bool isHappy = false;
bool isSleeping = false;
bool isShowingDB = false;
int measuredDB = 0;
// Beep Timer
unsigned long buzzerEndTime = 0;
// Mood Timer
unsigned long moodStartTime = 0;
const unsigned long MOOD_DURATION = 2000;
// Interaction & Sleep Timers
unsigned long lastInteractionTime = 0;
// Touch Logic Variables
bool lastTouchState = false;
unsigned long lastTouchTime = 0;
unsigned long lastTapTime = 0;
int tapCount = 0;
// Custom Idle Variables
unsigned long lastIdleMoveTime = 0;
unsigned long nextIdleInterval = 1000;
void setup() {
  // --- Direct Port Setup (No pinMode) ---
  // Pin 2 (PD2) Output (Buzzer)
  DDRD |= (1 << 2);
  // Pin 9 (PB1) Input (Touch)
  DDRB &= ~(1 << 1);
  // --- Display & Eyes ---
  display.begin(I2C_ADDRESS, true);
  eyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);
  // DISABLE Built-in Idle
  eyes.setIdleMode(false, 0, 0);
  // ENABLE Curiosity
  eyes.setCuriosity(true);
  // AUTO BLINKER: Blinks eyes.
  eyes.setAutoblinker(true, 3000, 1000);
  eyes.setMood(DEFAULT);
  lastInteractionTime = millis();
}
void loop() {
  unsigned long currentMillis = millis();
  // --- 1. Input ---
  // A1 Pot -> Contrast
  int potValue = analogRead(A1);
  display.setContrast(map(potValue, 0, 1023, 0, 255));
  // A0 Sound
  int soundValue = analogRead(A0);
  // Pin 9 Touch
  bool touchActive = (PINB & (1 << 1));
  // --- 2. Global Interaction Check ---
  if (soundValue > SOUND_THRESHOLD || touchActive) {
    lastInteractionTime = currentMillis;
    if (isSleeping) {
      isSleeping = false;
      // Wake up!
      eyes.setMood(DEFAULT);
      eyes.setAutoblinker(true, 3000, 1000); // Re-enable blink
    }
  }
  // Check for Sleep
  if (!isSleeping && (currentMillis - lastInteractionTime > SLEEP_TIMEOUT_MS)) {
    isSleeping = true;
    isScared = false;
    isHappy = false;
    isShowingDB = false;
    eyes.setMood(TIRED); // Closed eyes look
    eyes.setAutoblinker(false, 0, 0);
  }
  // --- 3. Logic (Only run if NOT sleeping) ---
  bool isBeeping = (currentMillis < buzzerEndTime);
  if (!isSleeping) {
    // SOUND TRIGGER (SCARED / ALERT)
    // Only trigger if not already in an alert sequence
    if (!isScared && soundValue > SOUND_THRESHOLD) {
      isScared = true;
      isHappy = false;
      isShowingDB = false;
      // Calculate fake DB for display later
      // Map Sound Value (Threshold to 1023) -> (60dB to 95dB)
      // This is an estimation since we don't have accurate calibration
      measuredDB = map(soundValue, SOUND_THRESHOLD, 1023, 60, 95);
      // 1. Center Eyes and Set ANGRY
      eyes.setMood(ANGRY);
      eyes.setPosition(DEFAULT); // Center position
      // 2. Start Buzzer (3 seconds)
      buzzerEndTime = currentMillis + ALERT_BUZZER_DURATION;
      PORTD |= (1 << 2); // Buzzer ON (High)
      // Start Sequence Timer
      moodStartTime = currentMillis;
    }
    // ALERT SEQUENCE HANDLING
    if (isScared) {
      unsigned long timeInAlert = currentMillis - moodStartTime;
      // Phase 1: 0 - 3 seconds (Angry + Buzzer)
      // Managed by buzzerEndTime logic.
      // Phase 2: 3 - 5 seconds (Show DB)
      // ALERT_DB_SWITCH_MS = 3000
      if (timeInAlert >= ALERT_DB_SWITCH_MS &&
          timeInAlert < ALERT_TOTAL_DURATION) {
        isShowingDB = true;
      } else {
        isShowingDB = false;
      }
      // End of Alert
      if (timeInAlert >= ALERT_TOTAL_DURATION) {
        isScared = false;
        eyes.setMood(DEFAULT);
        isShowingDB = false;
        display.clearDisplay();
        // Re-enable eyes drawing cleanly
      }
    }
    // TOUCH TRIGGER & DOUBLE TAP (Only if NOT Scared/Alert)
    if (!isScared) {
      if (touchActive != lastTouchState) {
        // Pin state changed
        if (currentMillis - lastTouchTime > DEBOUNCE_MS) {
          lastTouchTime = currentMillis;
          if (touchActive) {
            // RISING EDGE
            if (currentMillis - lastTapTime < DOUBLE_TAP_MAX_GAP_MS) {
              // DOUBLE TAP -> Laugh/Love
              tapCount = 2;
              if (!isScared) {
                isHappy = true;
                eyes.anim_laugh();
                moodStartTime = currentMillis + 1500;
              }
            } else {
              // Single Tap -> Happy
              tapCount = 1;
              if (!isScared) {
                isHappy = true;
                eyes.setMood(HAPPY);
                moodStartTime = currentMillis;
              }
            }
            lastTapTime = currentMillis;
          }
        }
        lastTouchState = touchActive;
      }
      // Happy Timeout
      if (isHappy && (currentMillis - moodStartTime > MOOD_DURATION)) {
        isHappy = false;
        eyes.setMood(DEFAULT);
      }
      // CUSTOM IDLE MOVEMENT
      if (!isHappy) {
        if (currentMillis - lastIdleMoveTime > nextIdleInterval) {
          byte randDir = random(1, 9);
          eyes.setPosition(randDir);
          lastIdleMoveTime = currentMillis;
          nextIdleInterval = random(1500, 4000);
        }
      }
    }
  }
  // --- 4. Output ---
  // Buzzer Control
  if (isBeeping) {
    PORTD |= (1 << 2);
  } else {
    PORTD &= ~(1 << 2);
  }
  // Display Control
  if (isShowingDB && !isSleeping) {
    // Custom Drawing: DB Value
    // Font size 3 is approx 18px wide per char.
    // "XX dB" = 5 chars = ~90px width.
    // Center X = (128 - 90) / 2 = 19
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(18, 20); // Centered X, Y
    display.print(measuredDB);
    display.print(" dB");
    display.display();
  } else {
    // Normal Eyes Drawing
    // Only update eyes if NOT showing DB text
    if (!isShowingDB) {
      eyes.update();
    }
  }
}
