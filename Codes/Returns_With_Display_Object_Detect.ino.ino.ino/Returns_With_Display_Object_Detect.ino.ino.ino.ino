#include <Arduino.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NewPing.h> // Include the NewPing library for the ultrasonic sensor

// Define the trigger and echo pins for the ultrasonic sensor
#define TRIGGER_PIN 13
#define ECHO_PIN 52

NewPing sonar(TRIGGER_PIN, ECHO_PIN); // Create a NewPing object with the defined pins

// Define a threshold distance to detect obstacles (in centimeters)
#define OBSTACLE_THRESHOLD 5
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {53, 51, 48, 47}; // Connect to the row pinouts of the keypad
byte colPins[COLS] = {45, 43, 41, 39}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int s1, s2, s3, s4, s5, s6, s7, s8;
const int ENA = 10; // PWM pin for Motor A speed control
const int IN1 = 22; // Motor A input 1
const int IN2 = 24; // Motor A input 2
const int ENB = 11; // PWM pin for Motor B speed control
const int IN3 = 26; // Motor B input 1
const int IN4 = 28; // Motor B input 2

int mode = 0; // Variable to store the selected mode

bool firstIntersectionPassed = false;
bool specialCaseActivated = false;
bool turned180 = false;

#define OLED_RESET 12
Adafruit_SSD1306 display(OLED_RESET);

void setup() {
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    Serial.begin(9600);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("FOOD SERVING ROBOT");
    display.display();
    delay(3000); // Delay for 3 seconds to display the greeting

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("CHOOSE A TABLE");
    display.display();
}

bool tableSelected = false; // Variable to indicate if a table number has been selected

int prevMode = 0; // Variable to store the previous mode

void loop() {
    char key = keypad.getKey();
  
    if (key != NO_KEY) {
        if (key == '1' || key == '2' || key == '3' || key == '4') {
            mode = key - '0'; // Convert char to int
            Serial.print("Selected Table: ");
            Serial.println(mode);
            specialCaseActivated = false; // Reset the special case flag
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Selected Table: ");
            display.println(mode);
            display.display();
        }
        else if (key == '0') {
            turn180Degrees();
            specialCaseActivated = true; // Set the special case flag
            Serial.println("Return Button Pressed");
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.println("Return Button Pressed");
            display.display();
        }
    }

    // If mode has been selected, start line following
    if (mode != 0) {
        // Read sensor values
        s1 = digitalRead(2);
        s2 = digitalRead(3);
        s3 = digitalRead(4);
        s4 = digitalRead(5);
        s5 = digitalRead(6);
        s6 = digitalRead(7);
        s7 = digitalRead(8);
        s8 = digitalRead(9);

        // Check for obstacles using ultrasonic sensor
        int distance = sonar.ping_cm();
        if (distance <= OBSTACLE_THRESHOLD && distance > 0) {
            // Stop the motors if an obstacle is detected
            motor_stop();
            // Print "OBSTACLE AHEAD"
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("OBSTACLE AHEAD");
            display.display();
            // Wait until the obstacle is cleared
            while (distance <= OBSTACLE_THRESHOLD && distance > 0) {
                distance = sonar.ping_cm();
                delay(50); // Adjust delay as needed
            }
            // Clear the display to revert back to normal
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Selected Table: ");
            display.println(mode);
            display.display();
        }

        // Line following
        lineFollow();
    }
}




void lineFollow() {
  // Black = 1, White = 0

  // Forward
   if (
    ((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 1) && (s5 == 1) && (s6 == 0) && (s7 == 0) && (s8 == 0)) ||
    ((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 1) && (s5 == 0) && (s6 == 0) && (s7 == 0) && (s8 == 0)) ||
    ((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 0) && (s5 == 1) && (s6 == 0) && (s7 == 0) && (s8 == 0)) ||
    ((s1 == 0) && (s2 == 0) && (s3 == 1) && (s4 == 1) && (s5 == 1) && (s6 == 1) && (s7 == 0) && (s8 == 0)) ||
    ((s1 == 0) && (s2 == 1) && (s3 == 1) && (s4 == 1) && (s5 == 1) && (s6 == 1) && (s7 == 1) && (s8 == 0))
  ) {
    forward();
  }
  // Right
  else if (
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==1)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==1)&&(s7==1)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==1)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==1)&&(s6==1)&&(s7==0)&&(s8==0)) ||

    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==1)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==1)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==1)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==1)&&(s7==1)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==1)&&(s6==1)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==0)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==0))
  ) {
    right();
  }
  // Left
  else if (
    ((s1==1)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==1)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||

    ((s1==1)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==1)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==1)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==1)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==1)&&(s2==0)&&(s3==0)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==0)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==1)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==0)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==0)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==0)&&(s7==0)&&(s8==0)) ||
    ((s1==0)&&(s2==1)&&(s3==1)&&(s4==1)&&(s5==1)&&(s6==1)&&(s7==0)&&(s8==0))
  ) {
    left();
  }
  // Stop
  else if ((s1 == 0) && (s2 == 0) && (s3 == 0) && (s4 == 0) && (s5 == 0) && (s6 == 0) && (s7 == 0) && (s8 == 0)) {
    motor_stop();
  }

  // If all sensors are on the line and a mode has been selected, turn left or right
  if ((s1 == 1) && (s2 == 1) && (s3 == 1) && (s4 == 1) && (s5 == 1) && (s6 == 1) && (s7 == 1) && (s8 == 1)) {
    
    if (specialCaseActivated && turned180) {
      
      if (mode == 1) {
        right(); // Change behavior for table 1
        Serial.println("Returning From Table 1");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Returning From Table 1");
            display.println(mode);
            display.display();
      } else if (mode == 2) {
        left(); // Change behavior for table 2
        Serial.println("Returning From Table 2");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Returning From Table 2");
            display.println(mode);
            display.display();
      }
      else if (mode == 3) {
        forward(); // Change behavior for table 3
        delay(1000);
        Serial.println("Returning From Table 3");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Returning From Table 3");
            display.println(mode);
            display.display();
      }
      else if (mode == 4) {
        forward(); // Change behavior for table 4
        delay(1000);
       Serial.println("Returning From Table 4");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Returning From Table 4");
            display.println(mode);
            display.display();
      }
    } else {
      if (mode == 1) {
        left();
        Serial.println("Table 1");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Table 1");
            display.println(mode);
            display.display();
      } else if (mode == 2) {
        right();
        Serial.println("Table 2");
         display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0,0);
            display.print("Table 2");
            display.println(mode);
            display.display();
      }
      else if (mode == 3){
        if (firstIntersectionPassed) {
        left(); // Turn left at the intersection
        Serial.println("Table 3: Turned left at intersection (subsequent)");
      } else {
        forward(); // Move forward without turning
        Serial.println("Table 3: Moved forward through intersection (first)");
        delay(500); // Delay to ensure the robot fully crosses the intersection
        firstIntersectionPassed = true; // Update the flag to indicate the first intersection is passed
      }
      } 
      else if (mode == 4) {
        if (firstIntersectionPassed) {
        right(); // Turn right at the intersection
        Serial.println("Table 4: Turned right at intersection (subsequent)");
      } 
      else {
        forward(); // Move forward without turning
        Serial.println("Table 4: Moved forward through intersection (first)");
        delay(500); // Delay to ensure the robot fully crosses the intersection
        firstIntersectionPassed = true; // Update the flag to indicate the first intersection is passed
      }
    }
    }
    
  }
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 110);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 110);
}

void right() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 170);
}

void left() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 170);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}

void motor_stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}

void turn180Degrees() {
  // Stop the robot
  motor_stop();
  delay(500); // Add a small delay for stability
  
  // Set the motors to rotate in opposite directions
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 150); // Adjust the speed as needed
  
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENB, 150); // Adjust the speed as needed
  
  // Delay for the time needed to complete the turn
  delay(1500); // Adjust the delay based on the robot's turning performance
  
  // Stop the motors
  motor_stop();
  turned180 = true; 
}

