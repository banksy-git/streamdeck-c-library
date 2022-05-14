#ifndef STREAMDECK_H
#define STREAMDECK_H

#include <hidapi/hidapi.h>

struct streamdeck
{
    hid_device *dev;
};

int streamdeck_open(
    struct streamdeck **dev,
    uint16_t vid,
    uint16_t pid,
    const wchar_t *serial_number);

int streamdeck_close(struct streamdeck *dev);

int streamdeck_reset(struct streamdeck *dev);

int streamdeck_set_brightness(
    struct streamdeck *dev,
    uint8_t percent);

int streamdeck_set_key_image(
    struct streamdeck *dev,
    uint8_t key,
    const uint8_t *data,
    size_t length);

int streamdeck_read_keys(
    struct streamdeck *dev,
    char *keystate,
    size_t length);

#endif // Header guard