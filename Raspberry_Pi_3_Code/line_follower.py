import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)

short_line = 5
med_line = 10
long_line = 20
reload_time = 2

left_motor_1 = 2  # Left motor 1, pin 3
left_motor_2 = 3  # Left motor 2, pin 5
right_motor_1 = 17  # Right motor 1, pin 11
right_motor_2 = 27  # Right motor 2, pin 13
button = 23  # Button input, pin 16

servo_pin = 10  # Servo motor, pin 19
servo_closed = 10  # Servo frequencies (min 2.5, max 12.5)
servo_open = 6
servo = GPIO.PWM


def motors_stop():
    GPIO.output(left_motor_1, GPIO.LOW)
    GPIO.output(left_motor_2, GPIO.LOW)
    GPIO.output(right_motor_1, GPIO.LOW)
    GPIO.output(right_motor_2, GPIO.LOW)


def drive(drive_time):
    # Set motors to move forwards
    GPIO.output(left_motor_1, GPIO.HIGH)
    GPIO.output(left_motor_2, GPIO.LOW)
    GPIO.output(right_motor_1, GPIO.HIGH)
    GPIO.output(right_motor_2, GPIO.LOW)
    # Wait for a time
    time.sleep(drive_time)
    # Stop the motors
    motors_stop()


def turn_90(direction):
    # Set the motor direction
    if direction == 'L':
        GPIO.output(left_motor_1, GPIO.LOW)
        GPIO.output(left_motor_2, GPIO.HIGH)
        GPIO.output(right_motor_1, GPIO.HIGH)
        GPIO.output(right_motor_2, GPIO.LOW)
    else:
        GPIO.output(left_motor_1, GPIO.HIGH)
        GPIO.output(left_motor_2, GPIO.LOW)
        GPIO.output(right_motor_1, GPIO.LOW)
        GPIO.output(right_motor_2, GPIO.HIGH)
    # Wait until the turn is complete
    time.sleep(3)
    # Stop the motors
    motors_stop()


def turn_180():
    # Set the motor direction (turn left)
    GPIO.output(left_motor_1, GPIO.LOW)
    GPIO.output(left_motor_2, GPIO.HIGH)
    GPIO.output(right_motor_1, GPIO.HIGH)
    GPIO.output(right_motor_2, GPIO.LOW)
    # Wait until the turn is complete
    time.sleep(6)
    # Stop the motors
    motors_stop()


def fire():
    servo.ChangeDutyCycle(servo_open)
    time.sleep(2)
    servo.ChangeDutyCycle(servo_closed)


if __name__ == "__main__":
    print("Setting up devices")
    # Set up motors
    GPIO.setup(left_motor_1, GPIO.OUT, initial=GPIO.LOW)  # Left motor 1, pin 3
    GPIO.setup(left_motor_2, GPIO.OUT, initial=GPIO.LOW)  # Left motor 2, pin 5
    GPIO.setup(right_motor_1, GPIO.OUT, initial=GPIO.LOW)  # Right motor 1, pin 11
    GPIO.setup(right_motor_2, GPIO.OUT, initial=GPIO.LOW)  # Right motor 2, pin 13
    GPIO.setup(button, GPIO.IN)  # Button input, pin 16
    GPIO.setup(servo_pin, GPIO.OUT)  # Servo motor control, pin 19
    servo = GPIO.PWM(servo_pin, 50)  # Servo motor control, pin 19, 50Hz
    servo.start(servo_closed)

    # Waiting for button
    print("Waiting for button press")
    while GPIO.input(2) == 0:
        continue
    time.sleep(0.5)

    # Run through routine until the time limit is reached
    # Move to first loader (initial movement)
    print("Moving to first loader")
    drive(short_line)
    for i in range(3):
        # Reload
        print("Loading")
        time.sleep(reload_time)
        # Fire
        print("Firing")
        fire()
        # Turn around
        print("Turning around")
        turn_180()
        # Move to the T
        print("Moving to T")
        move(short_line)
        # Turn left (to the middle of the field)
        print("Turning left")
        turn_90('L')
        # Move to opposite side of field
        print("Moving to opposite side of field")
        drive(long_line)
        # Right right
        print("Turning right")
        turn_90('R')
        # Move to next loader
        print("Moving to next loader")
        drive(short_line)
        # Reload
        print("Loading")
        time.sleep(reload_time)
        # Turn around
        print("Turnign around")
        turn_180()
        # Move to next loader
        print("Moving to next loader")
        drive(med_line)
        # Reload
        print("Loading")
        time.sleep(reload_time)
        # Fire
        print("Firing")
        fire()
        # Turn around
        print("Turning around")
        turn_180()
        # Move to T
        print("Moving to T")
        drive(short_line)
        # Turn right (to the middle of the field)
        print("Turning right")
        turn_90('R')
        # Drive to the opposite side of the field
        print("Moving to opposite side of field")
        drive(long_line)
        # Turn left
        print("Turning left")
        turn_90('L')
        # Move to the next loader
        print("Moving to next loader")
        drive(short_line)
        # Reload
        print("Loading")
        time.sleep(reload_time)
        # Turn around
        print("Turning around")
        turn_180()
        # Move to next loader
        print("Moving to next loader")
        drive(med_line)

    GPIO.cleanup()
    servo.stop()
