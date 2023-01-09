
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

#define BTN_CENTER      0x1000

typedef struct {
  uint16_t value;
  char key[8];
  char str[10];
} Key;

Key button_map[11] = {
  {BTN_UP, "w", "UP"},
  {BTN_DOWN, "s", "DOWN"},
  {BTN_LEFT, "a", "LEFT"},
  {BTN_RIGHT, "d", "RIGHT"},
  {BTN_SQUARE, "1", "SQUARE"},
  {BTN_CROSS, "2", "CROSS"},
  {BTN_CIRCLE, "3", "CIRLCE"},
  {BTN_TRIANGLE, "4", "TRIANGLE"},
  {BTN_START, "x", "START"},
  {BTN_SELECT, "y", "SELECT"},
  {BTN_CENTER, "z", "CENTER"},
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
    printf("read() %d bytes\n", res);
    for (i = 0; i < res; i++)
        printf("%hhx ", buf[i]);
    printf("\n");
    for (i = 0; i < res; i++) {
        printf(" ");
        for (j = 0; j < 8; j++) {
            if (buf[i] & 1 << j) {
                printf("1");
            } else {
                printf("0");
            }
        }
        if ((i+1)%4 == 0) {
            printf("\n");
        }
    }
    puts("\n");
#endif

#ifdef DEBUG_BTN
    if (buf[4] & 128) puts("CENTER");
    if (buf[5] & 16) puts("LEFT");
    if (buf[5] & 32) puts("DOWN");
    if (buf[5] & 64) puts("UP");
    if (buf[5] & 128) puts("RIGHT");
    if (buf[6] & 1) puts("TRIANGLE");
    if (buf[6] & 2) puts("SQUARE");
    if (buf[6] & 4) puts("CROSS");
    if (buf[6] & 8) puts("CIRCLE");
    if (buf[6] & 16) puts("SELECT");
    if (buf[6] & 32) puts("START");
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

        if (buf[4] & 128)   pressed |= BTN_CENTER;
        if (buf[5] & 16)    pressed |= BTN_LEFT;
        if (buf[5] & 32)    pressed |= BTN_DOWN;
        if (buf[5] & 64)    pressed |= BTN_UP;
        if (buf[5] & 128)   pressed |= BTN_RIGHT;
        if (buf[6] & 1)     pressed |= BTN_TRIANGLE;
        if (buf[6] & 2)     pressed |= BTN_SQUARE;
        if (buf[6] & 4)     pressed |= BTN_CROSS;
        if (buf[6] & 8)     pressed |= BTN_CIRCLE;
        if (buf[6] & 16)    pressed |= BTN_SELECT;
        if (buf[6] & 32)    pressed |= BTN_START;
    }

    return pressed;
}


int main(int argc, char **argv)
{
    char *device = "/dev/hidraw0";

    xdo = xdo_new(NULL); 

    int fd, res;
    uint8_t buf[12] = {0};

    if (argc > 1)
        device = argv[1];

    fd = open(device, O_RDONLY);
    if (fd < 0) {
        printf("Unable to open device '%s':\n", device);
        perror("");
        return 1;
    }
    printf("Connected to device '%s'\n", device);

    while (1) {

        pressed_prev = pressed_new;
        pressed_new = ~read_pad(fd);
        modified = pressed_new ^ pressed_prev;
    
        Key pressed;
        for (unsigned i = 0; i < 11; i++) {
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
        sleep(0.01);
    }
    
    close(fd);
    return 0;
}

