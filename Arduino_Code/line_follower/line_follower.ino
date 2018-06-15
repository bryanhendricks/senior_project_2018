#include <Servo.h>
#include <AFMotor.h> // Uses digital pins 4, 7, 8, and 12

// Line sensor pin numbers
#define LEFT_SENSOR 0
#define MIDDLE_SENSOR 1
#define RIGHT_SENSOR 2

// Line sensor data values
int left_sensor_val = 0;
int middle_sensor_val = 0;
int right_sensor_val = 0;
bool left_sensor_line = false;
bool middle_sensor_line = false;
bool right_sensor_line = false;

// Robot position indicators
#define POS_LEFT_3 -3  // Far enough left that no sensors see the line
#define POS_LEFT_2 -2  // Only the right sensor can see the line
#define POS_LEFT_1 -1  // The middle and right sensors can see the line
#define POS_MID 0     // Only the middle sensor can see the line
#define POS_RIGHT_1 1 // The middle and left sensors can see the line
#define POS_RIGHT_2 2 // Only the left sensor can see the line
#define POS_RIGHT_3 3 // Far enough right that no sensors see the line
#define POS_T 10

// Keep track of the position indicators
char pos_state = POS_MID;

// Line sensor color thresholds
int color_thresh = 1000;

// Set up DC motor on M1
AF_DCMotor motor_left(1); // Uses digital pin 11

// Set up DC motor on M2
AF_DCMotor motor_right(2); // Uses digital pin 3

// Set up DC motor on M3
AF_DCMotor motor_fire(3); // Uses digital pin 5
#define FIRING_MOTOR_SPEED 200

// Set up servo motor
Servo firing_servo;
#define FIRING_SERVO_CLOSED 45
#define FIRING_SERVO_OPEN 120

// Max speed (up to 255)
unsigned char max_speed = 150;

// List of turns to take
#define TURN_LEFT 0
#define TURN_RIGHT 1
#define TURN_180 2
#define FIRE 3
#define RELOAD 4
#define SHORT_LINE 5
#define MED_LINE 6
#define LONG_LINE 7
#define RELOAD_TIME 1000 // Ms the robot waits to reload
#define SHORT_LINE_TIME 1500 // Ms as the robot moves along the line for
#define MED_LINE_TIME 3000
char move_list[] = {SHORT_LINE, RELOAD,
TURN_180, MED_LINE, RELOAD, FIRE,
TURN_180, SHORT_LINE, TURN_LEFT,
LONG_LINE, TURN_LEFT,
SHORT_LINE, RELOAD,
TURN_180, MED_LINE, RELOAD,
TURN_180, SHORT_LINE, TURN_LEFT,
LONG_LINE, TURN_RIGHT, FIRE,
TURN_180};
int cur_move_num = 0, move_count = sizeof(move_list);

// Keep track of time
unsigned long prev_time;

// Start button
#define START_BUTTON_PIN 6 // Digital pin 6

void setup() {
  // Set up serial comms
  Serial.begin(9600);
  Serial.println("Beginning setup");

  // Initialize the motors as stopped
  motor_left.setSpeed(0); // Set the motor speed to 0 (max 255)
  motor_right.setSpeed(0);
  motor_fire.setSpeed(0);

  // Enable the motors
  motor_left.run(FORWARD); // Set the motor direction
  motor_right.run(FORWARD);
  motor_fire.run(FORWARD);

  // Initialize the servo as closed
  firing_servo.attach(9); // Pin 9 (SER1 on the motor controller)
  firing_servo.write(FIRING_SERVO_CLOSED);

  // Set up start button
  pinMode(START_BUTTON_PIN, INPUT);

  // Wait until start button is pressed
  while (digitalRead(START_BUTTON_PIN) == LOW) {}

  // Wait for a half second
  delay(500);

  // Initialize the line sensors
  get_color_thresh();
}

void fire() {
  char count;

  // Stop the robot from moving
  motor_left.setSpeed(0);
  motor_right.setSpeed(0);
  delay(200);

  // Spin up the firing motor
  motor_fire.setSpeed(FIRING_MOTOR_SPEED);
  delay(200);

  // Open the firing servo 10 times
  for(count = 0; count < 10; count++){
    firing_servo.write(FIRING_SERVO_OPEN);
    delay(200);
    firing_servo.write(FIRING_SERVO_CLOSED);
    delay(100);
  }
}

void turn_90() {
  prev_time = millis();
}

void turn_180() {
  prev_time = millis();
}

void move_straight() {
  if (pos_state == POS_LEFT_3) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(BACKWARD);
    // Turn right
    motor_left.setSpeed(max_speed);
    motor_right.setSpeed(max_speed * 0.2);
  }
  else if (pos_state == POS_LEFT_2) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(FORWARD);
    // Turn right
    motor_left.setSpeed(max_speed);
    motor_right.setSpeed(max_speed * 0.2);
  }
  else if (pos_state == POS_LEFT_1) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(FORWARD);
    // Turn right
    motor_left.setSpeed(max_speed);
    motor_right.setSpeed(max_speed * 0.7);
  }
  else if (pos_state == POS_MID) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(FORWARD);
    // Move straight forward
    motor_left.setSpeed(max_speed);
    motor_right.setSpeed(max_speed);
  }
  else if (pos_state == POS_RIGHT_1) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(FORWARD);
    // Turn left
    motor_left.setSpeed(max_speed * 0.7);
    motor_right.setSpeed(max_speed);
  }
  else if (pos_state == POS_RIGHT_2) {
    // Enable the motors
    motor_left.run(FORWARD);
    motor_right.run(FORWARD);
    // Turn left
    motor_left.setSpeed(max_speed * 0.2);
    motor_right.setSpeed(max_speed);
  }
  else if (pos_state == POS_RIGHT_3) {
    // Enable the motors
    motor_left.run(BACKWARD);
    motor_right.run(FORWARD);
    // Turn left
    motor_left.setSpeed(max_speed * 0.2);
    motor_right.setSpeed(max_speed);
  }
}

// Get data from the line sensors based on the average of 3 independant sensor reads
void read_line_sensors() {
  // Get new sensor data
  left_sensor_val = analogRead(LEFT_SENSOR);
  middle_sensor_val = analogRead(MIDDLE_SENSOR);
  right_sensor_val = analogRead(RIGHT_SENSOR);

  // Get new sensor data, average it with the previous set
  left_sensor_val = (left_sensor_val + analogRead(LEFT_SENSOR)) / 2;
  middle_sensor_val = (middle_sensor_val + analogRead(MIDDLE_SENSOR)) / 2;
  right_sensor_val = (right_sensor_val + analogRead(RIGHT_SENSOR)) / 2;

  // Get new sensor data again, average it with the previous set
  left_sensor_val = (left_sensor_val / 1.5) + (analogRead(LEFT_SENSOR) / 3);
  middle_sensor_val = (middle_sensor_val / 1.5) + (analogRead(MIDDLE_SENSOR) / 3);
  right_sensor_val = (right_sensor_val / 1.5) + (analogRead(RIGHT_SENSOR) / 3);

  // Update the sensor booleans
  if (left_sensor_val > color_thresh) {
    left_sensor_line = true;
  }
  else {
    left_sensor_line = false;
  }
  if (middle_sensor_val > color_thresh) {
    middle_sensor_line = true;
  }
  else {
    middle_sensor_line = false;
  }
  if (right_sensor_val > color_thresh) {
    right_sensor_line = true;
  }
  else {
    right_sensor_line = false;
  }
}

// Update the position variable
char update_position() {
  // Read data from the color sensors
  read_line_sensors();

  // Switch case for detecting where the line is
  // Able to change states up to 2 transitions away from any other given state (more can be added if needed)
  switch (pos_state) {
    case POS_LEFT_3:
      // Far enough left that no sensors see the line
      if (!left_sensor_line && !middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_2;
      }
      else if (!left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_1;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      break;
    case POS_LEFT_2:
      // Only the right sensor can see the line
      if (!left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_LEFT_3;
      }
      else if (!left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_1;
      }
      else if (!left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_MID;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      break;
    case POS_LEFT_1:
      // The middle and right sensors can see the line
      if (!left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_LEFT_3;
      }
      else if (!left_sensor_line && !middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_2;
      }
      else if (!left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_MID;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      else if (left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_1;
      }
      break;
    case POS_MID:
      // Only the middle sensor can see the line
      if (!left_sensor_line && !middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_2;
      }
      else if (!left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_1;
      }
      else if (left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_1;
      }
      else if (left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_2;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      break;
    case POS_RIGHT_1:
      // The middle and left sensors can see the line
      if (!left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_LEFT_1;
      }
      else if (!left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_MID;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      else if (left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_2;
      }
      else if (!left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_3;
      }
      break;
    case POS_RIGHT_2:
      // Only the left sensor can see the line
      if (!left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_MID;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      else if (left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_1;
      }
      else if (!left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_3;
      }
      break;
    case POS_RIGHT_3:
      // Far enough right that no sensors see the line
      if (left_sensor_line && middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_1;
      }
      else if (left_sensor_line && !middle_sensor_line && !right_sensor_line) {
        pos_state = POS_RIGHT_2;
      }
      else if (left_sensor_line && middle_sensor_line && right_sensor_line) {
        pos_state = POS_T;
      }
      break;
    default:
      break;
  }
}

void get_color_thresh() {
  // Read data from the color sensors
  read_line_sensors();

  // Set the color threshold value to be the midpoint between the light and dark readings
  // Get the average light color (outer sensors), then average that with the dark color (middle sensor)
  color_thresh = (((left_sensor_val + right_sensor_val) / 2) + middle_sensor_val) / 2;

  // Read the line sensors again to get the correct boolean line states
  read_line_sensors();
}
