#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <hidapi/hidapi.h>

#include "streamdeck.h"

struct image_header
{
    uint8_t report_id;
    uint8_t seven;
    uint8_t key;
    uint8_t is_last;
    uint16_t length_le;
    uint16_t page_no_le;
} __attribute__((packed));

static int _send_feature_report(
    hid_device *dev,
    const uint8_t *data,
    size_t length)
{
    if (dev == NULL || data == NULL || length > 32)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }

    uint8_t buffer[33] = {0};
    memcpy(buffer + 1, data, length);
    return hid_send_feature_report(dev, buffer, 33);
}

int streamdeck_open(
    struct streamdeck **dev,
    uint16_t vid,
    uint16_t pid,
    const wchar_t *serial_number)
{
    if (dev == NULL)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }

    *dev = calloc(1, sizeof(**dev));
    if (*dev == NULL)
    {
        return STREAMDECK_ERROR;
    }
    (*dev)->dev = hid_open(0xfd9, 0x80, NULL);
    if ((*dev)->dev == NULL)
    {
        free(*dev);
        return STREAMDECK_ERROR;
    }
    return 0;
}

int streamdeck_close(struct streamdeck *dev)
{
    if (dev->dev != NULL)
    {
        hid_close(dev->dev);
        dev->dev = NULL;
        free(dev);
        return STREAMDECK_OK;
    }
    else
    {
        return STREAMDECK_ERROR;
    }
}

int streamdeck_reset(struct streamdeck *dev)
{
    if (dev == NULL)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }
    return _send_feature_report(dev->dev, "\x03\x02", 2);
}

int streamdeck_set_brightness(
    struct streamdeck *dev,
    uint8_t percent)
{
    if (dev == NULL)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }

    uint8_t buffer[3] = {0x03, 0x08, percent};
    return _send_feature_report(dev->dev, buffer, 3);
}

int streamdeck_set_key_image(
    struct streamdeck *dev,
    uint8_t key,
    const uint8_t *data,
    size_t length)
{
    if (dev == NULL || data == NULL || length <= 0)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }

    int error = STREAMDECK_OK;
    const size_t payload_size = 1024;
    struct image_header *payload = malloc(payload_size);
    if (payload == NULL)
    {
        return -1;
    }
    payload->report_id = 2;
    payload->seven = 7;
    payload->key = key;
    payload->is_last = 0;
    payload->page_no_le = 0;

    const size_t chunk_max = payload_size - sizeof(struct image_header);
    size_t pos = 0;
    while (payload->is_last == 0)
    {

        payload->length_le = length - pos;
        if (payload->length_le > chunk_max)
        {
            payload->length_le = chunk_max;
        }
        memcpy(payload + 1, data + pos, payload->length_le);
        pos += payload->length_le;
        if (pos >= length)
        {
            payload->is_last = 1;
        }

        if (hid_write(dev->dev, (uint8_t *)payload, payload_size) == -1)
        {
            error = STREAMDECK_ERROR;
            break;
        }

        payload->page_no_le += 1;
    }

    free(payload);
    return error;
}

int streamdeck_read_keys(
    struct streamdeck *dev,
    char *keystate,
    size_t length)
{
    char data[64] = {0};
    if (dev == NULL || length > 32)
    {
        errno = EINVAL;
        return STREAMDECK_ERROR;
    }
    if (hid_read(dev->dev, data, 4 + length) == -1)
    {
        return STREAMDECK_ERROR;
    }

    // More than two is probably invalid given how the hardware
    // is connected. (Matrix)
    int i;
    int pressed_count = 0;
    for (i = 0; i < 32; i++)
    {
        if (data[i + 4] != 0)
        {
            pressed_count += 1;
        }
    }

    memcpy(keystate, data + 4, length);

    return pressed_count;
}
