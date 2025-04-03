#include <U8g2lib.h>         // U8g2 Library Reference: https://github.com/olikraus/u8g2/wiki
#include "MPU6050.h"         // https://github.com/ElectronicCats/mpu6050
#include <Leanbot.h>          // use Leanbot library

MPU6050 sensor;

int ax, ay, az; // Get values from sensor
int a, b;        // Converted values for display
int x, y;        // Random coordinates for food symbol
int score = 0;    // Game score
int timer = 30;   // Countdown timer in seconds
unsigned long previousMillis = 0;  // Used to track the time for the timer
bool gameOver = false;  // Flag to track if the game is over

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0);   // SH1106 for 1.3" OLED module

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize MPU6050 sensor
  sensor.initialize();
  if (sensor.testConnection()) {
    Serial.println("Init MPU6050 ok.");
  } else {
    Serial.println("Init MPU6050 error.");
    while (1);  // Stop here if sensor fails to initialize
  }

  // Start OLED
  u8g2.setBusClock(400000);  // I2C speed 400kHz
  u8g2.begin();

  // Initialize the first location of the food symbol
  x = 25;
  y = 25;
}

void loop() {
  if (gameOver) {
    // If the game is over, display the Game Over screen and reset after a small delay
    u8g2.firstPage();
    do {
      u8g2.setFont(u8g2_font_ncenB08_tr); // Use a simple font for game over
      u8g2.setCursor(30, 50);
      u8g2.print("Game Over");
      u8g2.setCursor(30, 60);
      u8g2.print("Final Score: ");
      u8g2.print(score);
    } while (u8g2.nextPage());
    LbDelay(2000); // Show the game over screen for 2 seconds
    resetGame();   // Reset the game after showing the game over screen
    gameOver = false; // Reset game over flag
    return; // Skip the rest of the loop
  }

  // Get acceleration data from MPU6050 sensor
  sensor.getAcceleration(&ax, &ay, &az);
  a = map(ax, -2000, 3200, 0, 128); // Map accelerometer data to x-coordinate
  b = map(ay, -5400, 2000, 0, 64); // Map accelerometer data to y-coordinate

  // Ensure player stays within bounds
  if (a > 56 && a < 68) a = 56;
  if (b > 30 && b < 42) b = 40;

  // Check if player symbol (fish) has eaten the food
  if ((a + 2) == x || (b + 2) == y) {
    x = random(6, 122);  // New random x-coordinate for food
    y = random(6, 58);   // New random y-coordinate for food
    score++;             // Increase score
    Leanbot.tone(500, 1000); // Play buzzer sound (500Hz for 1000ms)
  }

  // Timer logic
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= 1000 && timer > 0) {
    previousMillis = currentMillis;  // Save the last time update
    timer--;  // Decrease the timer every second
  }

  // OLED buffer drawing loop
  u8g2.firstPage();
  do {
    // Display player symbol (fish)
    u8g2.setFont(u8g2_font_unifont_t_animals);   // Use animal symbols font
    u8g2.drawGlyph(a, b, 65); // Draw fish symbol

    // Display food symbol
    u8g2.setFont(u8g2_font_7x13_t_symbols);
    u8g2.drawGlyph(x, y, 9728); // Draw food symbol (e.g., a star)

    // Display score and timer
    u8g2.setFont(u8g2_font_ncenB08_tr); // Use a simple font for score and timer
    u8g2.setCursor(0, 10);
    u8g2.print("Score: ");
    u8g2.print(score);
    
    u8g2.setCursor(0, 20);
    u8g2.print("Time: ");
    u8g2.print(timer);

    // Display Game Over message if time runs out
    if (timer == 0) {
      gameOver = true; // Set game over flag when the timer runs out
    }
  } while (u8g2.nextPage());

  LbDelay(50); // Small delay to avoid overwhelming the loop
}

// Reset game when time is over
void resetGame() {
  score = 0;     // Reset score
  timer = 30;    // Reset timer to 30 seconds
  x = random(6, 122);  // Reset food position
  y = random(6, 58);
}
