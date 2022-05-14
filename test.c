#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "streamdeck.h"

#define NUMBER_OF_KEYS 15

int load_file(const char *filename, uint8_t **data, size_t *size)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        return -1;
    }
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    *data = malloc(*size);
    fread(*data, *size, 1, f);
    fclose(f);
    return 0;
}

void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    int res;
    struct streamdeck *sd;

    if (streamdeck_open(&sd, 0xfd9, 0x80, NULL) == -1)
    {
        error_exit("Failed to open device.");
    }

    streamdeck_reset(sd);
    streamdeck_set_brightness(sd, 50);

    // Load button pictures
    uint8_t *green;
    size_t green_length;
    if (load_file("green.jpg", &green, &green_length) != 0)
    {
        error_exit("Failed to read green image");
    }

    uint8_t *black;
    size_t black_length;
    if (load_file("black.jpg", &black, &black_length) != 0)
    {
        error_exit("Failed to read black image");
    }

    uint8_t *exit;
    size_t exit_length;
    if (load_file("exit.jpg", &exit, &exit_length) != 0)
    {
        error_exit("Failed to read exit image");
    }


    streamdeck_set_key_image(sd, 14, exit, exit_length);

    char last_keys[NUMBER_OF_KEYS] = {0};
    bool is_exit = false;
    while(!is_exit)
    {
        char keys[NUMBER_OF_KEYS];
        if (streamdeck_read_keys(sd, keys, NUMBER_OF_KEYS) == -1)
        {
            break;
        }

        int i;
        for (i = 0; i < NUMBER_OF_KEYS; i++)
        {
            if (keys[i] != last_keys[i])
            {
                if (keys[i])
                {
                    if (i==14) {
                        is_exit = true;
                    }
                    streamdeck_set_key_image(sd, i, green, green_length);
                }
                else
                {
                    streamdeck_set_key_image(sd, i, black, black_length);
                }
            }
        }

        memcpy(last_keys, keys, NUMBER_OF_KEYS);
    }

    streamdeck_set_brightness(sd, 25);
    streamdeck_reset(sd);

    streamdeck_close(sd);

    return 0;
}
