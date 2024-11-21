#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "eecs388_lib.h"

#define SERVO_PULSE_MAX 2400 /* 2400 us */
#define SERVO_PULSE_MIN 544 /* 544 us */
#define SERVO_PERIOD 20000 /* 20000 us (20ms) */

void auto_brake(int devid)
{
    uint16_t dist = 0;
    if ('Y' == ser_read(devid) && 'Y' == ser_read(devid)) {
        uint8_t first_byte = ser_read(devid);
        uint8_t second_byte = ser_read(devid);
        dist = 256 * second_byte + first_byte;
        if (dist > 200) {
            gpio_write(GREEN_LED, ON);
            gpio_write(RED_LED, OFF);
        } else if (dist > 100) {
            gpio_write(GREEN_LED, ON);
            gpio_write(RED_LED, ON);
        } else if (dist > 60) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, OFF);
        } else if (dist >= 0) {
            gpio_write(RED_LED, ON);
            gpio_write(GREEN_LED, OFF);
            delay(100);
            gpio_write(RED_LED, OFF);
            delay(100);
        }
    }
    printf("\nAutobreak: %d", dist);
}

int read_from_pi(int devid) {
    char buffer[16];
    if (ser_isready(devid)) {
        ser_readline(devid, 16, buffer);
        int deg = atoi(buffer);
        printf("\nread_from_pi: %d", deg);
        return deg;
    }
    return -1000;
    
}

void steering(int gpio, int pos) {
    gpio_write(gpio, ON);
    printf("\nsteering: %d", pos);
    int time = SERVO_PULSE_MIN + (SERVO_PULSE_MAX - SERVO_PULSE_MIN)/180 * pos;
    delay_usec(time);
    gpio_write(gpio, OFF);
    delay_usec(SERVO_PERIOD - time);
}


int main()
{
    // initialize UART channels
    ser_setup(0); // uart0
    ser_setup(1); // uart1
    int pi_to_hifive = 1; //The connection with Pi uses uart 1
    int lidar_to_hifive = 0; //the lidar uses uart 0
    
    printf("\nUsing UART %d for Pi -> HiFive", pi_to_hifive);
    printf("\nUsing UART %d for Lidar -> HiFive", lidar_to_hifive);
    
    //Initializing PINs
    gpio_mode(PIN_19, OUTPUT);
    gpio_mode(RED_LED, OUTPUT);
    gpio_mode(BLUE_LED, OUTPUT);
    gpio_mode(GREEN_LED, OUTPUT);

    printf("Setup completed.\n");
    printf("Begin the main loop.\n");
    int gpio = PIN_19; 
    while (1) {
        auto_brake(lidar_to_hifive); // measuring distance using lidar and braking
        int angle = read_from_pi(pi_to_hifive); //getting turn direction from pi
        if (angle != -1000 ) {
            printf("\nangle=%d", angle);
            for (int i = 0; i < 10; i++) {
                // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
                // and 0 if the prediction is a negative angle.
                // This is so that it is easier to see the movement of the servo.
                // You are welcome to pass the angle values directly to the steering function.
                // If the servo function is written correctly, it should still work,
                // only the movements of the servo will be more subtle
                if(angle>0) {
                    steering(gpio, 180);
                } else {
                    steering(gpio, 0);
                }
            }
            
                // Uncomment the line below to see the actual angles on the servo.
                // Remember to comment out the if-else statement above!
                // steering(gpio, angle);
        }

    }
    return 0;
}
