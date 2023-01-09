
/* WIP
 * TODO: Map keys, Simulate keystrokes, maybe a signal handler for exit ? */


#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <xdo.h>

//#define DEBUG_COM
//#define DEBUG_BTN

#define BTN_UP          0x01
#define BTN_DOWN        0x02
#define BTN_LEFT        0x04
#define BTN_RIGHT       0x08

#define BTN_SQUARE      0x10
#define BTN_CROSS       0x20
#define BTN_CIRCLE      0x40
#define BTN_TRIANGLE    0x80

#define BTN_SELECT      0x100
#define BTN_START       0x200

typedef struct {
  uint16_t value;
  char key[8];
  char str[10];
} Key;

Key button_map[10] = {
  {BTN_UP, "w", "UP"},
  {BTN_DOWN, "s", "DOWN"},
  {BTN_LEFT, "a", "LEFT"},
  {BTN_RIGHT, "d", "RIGHT"},
  {BTN_SQUARE, "space", "SQUARE"},
  {BTN_CROSS, "space", "CROSS"},
  {BTN_CIRCLE, "space", "CIRLCE"},
  {BTN_TRIANGLE, "space", "TRIANGLE"},
  {BTN_START, "q", "START"},
  {BTN_SELECT, "e", "SELECT"},
};


xdo_t* xdo;

void key_down(char* key) {
    xdo_send_keysequence_window_down(xdo, CURRENTWINDOW, key, 0);
}

void key_up(char* key) {
    xdo_send_keysequence_window_up(xdo, CURRENTWINDOW, key, 0);
}


uint16_t pressed_new = 0;
uint16_t pressed_prev = 0;
uint16_t modified = 0;


void debug(int res, uint8_t* buf) {
#ifdef DEBUG_COM
    unsigned i, j;
    printf("read() read %d bytes, %d us interval:\n\t", res, sleep_us);
    for (i = 0; i < res; i++)
        printf("%hhx ", buf[i]);:
    printf("\n\t");
    for (i = 0; i < 2; i++) {
        printf("   ");
        for (j = 0; j < 8; j++) {
            if (buf[i] & 1 << j) {
                printf("1");
            } else {
                printf("0");
            }
        }
    }
    puts("\n");
#endif

#ifdef DEBUG_BTN
    uint8_t byte = 0x00;
    byte = buf[0];      
    if (byte & 0x01) puts("SQUARE");
    if (byte & 0x02) puts("CROSS");
    if (byte & 0x04) puts("CIRCLE");
    if (byte & 0x08) puts("TRIANGLE");

    byte = buf[1];
    if (byte & 0x01) puts("SELECT");
    if (byte & 0x02) puts("START");

    if (buf[7]) puts("RIGHT");
    if (buf[8]) puts("LEFT");
    if (buf[9]) puts("UP");
    if (buf[10]) puts("DOWN");
#endif
}


uint16_t read_pad(int fd) {
    uint8_t buf[12];
    uint16_t pressed = 0x00;
    memset(buf, 0x0, sizeof(buf));
    
    int res = read(fd, buf, 11);
    if (res < 0) {
        perror("read");
    } else {
        debug(res, buf);

        uint8_t byte = buf[0];

        if (byte & 0x01)  { pressed |= BTN_SQUARE; }
        if (byte & 0x02) { pressed |= BTN_CROSS; }
        if (byte & 0x04) { pressed |= BTN_CIRCLE; }
        if (byte & 0x08) { pressed |= BTN_TRIANGLE; }

        byte = buf[1];
        if (byte & 0x01) { pressed |= BTN_SELECT; }
        if (byte & 0x02) { pressed |= BTN_START; }

        if (buf[7]) { pressed |= BTN_RIGHT; }
        if (buf[8]) { pressed |= BTN_LEFT; }
        if (buf[9]) { pressed |= BTN_UP; }
        if (buf[10]) { pressed |= BTN_DOWN; }
    
    }

    return pressed;
}


int main(int argc, char **argv)
{
    char *device = "/dev/hidraw0";

    xdo = xdo_new(NULL); 

    int fd, res;
    unsigned sleep_us = 0, run = 0, cal_runs = 1000;
    uint8_t buf[12];
    memset(buf, 0x0, sizeof(buf));
    

    if (argc > 1)
        device = argv[1];

    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open device '%s':\n", device);
        perror("");
        return 1;
    }
    printf("Connected to device '%s'\n", device);
    sleep(2);
    /*
    puts("Calibrating ...");
    while (run++ < cal_runs) {
        printf("\r%0.0f%% ", ((double) run/ (double) cal_runs) * 100);
        fflush(stdout);
        res = read(fd, buf, 11);
        if (res < 0) {
            sleep_us += 100;
        }
        usleep(sleep_us);
    }
    */
    printf("\rDone! Read interval is %d us\n", sleep_us);

    while (1) {

        pressed_prev = pressed_new;
        pressed_new = ~read_pad(fd);
        modified = pressed_new ^ pressed_prev;
    
        Key pressed;
        for (unsigned i = 0; i < 10; i++) {
            pressed = button_map[i];
            if (pressed.value & modified) {
                if (pressed.value & pressed_new) {
                    key_up(pressed.key);
                }
                else  {
                    key_down(pressed.key);
                }
            }
        }   
        usleep(sleep_us);
    }
    
    close(fd);
    return 0;
}

