// Exercise the real USB report dispatcher without opening hardware.
#include <assert.h>
#include "../mk1-bridge/bridge.h"

// Keep synthetic test reports out of the live hardware logs.
static int test_fprintf(FILE *stream, const char *fmt, ...)
{
    (void)stream;
    (void)fmt;
    return 0;
}
#define mk1_bridge_fprintf test_fprintf
#ifndef MK1_DEVICE_TEST_SOURCE
#define MK1_DEVICE_TEST_SOURCE "../mk1-usb/mk1_device.c"
#endif
#include MK1_DEVICE_TEST_SOURCE

static unsigned callbacks;
static uint16_t last_pressure[16];

static void record_pads(const mk1_pad_event_t *pads, uint8_t count, void *ctx)
{
    (void)ctx;
    assert(count == 16);
    callbacks++;
    for (unsigned i = 0; i < count; i++) {
        assert(pads[i].index == i);
        last_pressure[i] = pads[i].pressure;
    }
}

static void put_word(uint8_t *p, uint16_t word)
{
    p[0] = word & 255;
    p[1] = word >> 8;
}

static void resting_frame(uint8_t frame[64], unsigned phase)
{
    for (unsigned i = 0; i < 16; i++) {
        uint16_t word = ((i + phase) & 15) << 12;
        put_word(frame + i * 2, word);
        put_word(frame + 32 + i * 2, word);
    }
}

int main(void)
{
    mk1_device_t dev = { .pad_cb = record_pads };
    mk1_pipe_reader_t reader = { .device = &dev, .endpoint_number = 4 };
    uint8_t rest[64], frame[64];

    // Every start ID, every physical pad, equal AND unequal ADC passes.
    // These are all real pad frames, including the all-zero release frames.
    for (unsigned phase = 0; phase < 16; phase++) {
        resting_frame(rest, phase);
        for (unsigned pad = 0; pad < 16; pad++) {
            for (unsigned equal = 0; equal < 2; equal++) {
                memcpy(frame, rest, sizeof(frame));
                unsigned slot = (pad + 16 - phase) & 15;
                put_word(frame + slot * 2, (pad << 12) + 1000);
                put_word(frame + 32 + slot * 2, (pad << 12) + 999 + equal);
                unsigned before = callbacks;
                mk1_device_dispatch_input_report(&reader, frame, sizeof(frame));
                assert(callbacks == before + 1);
                for (unsigned i = 0; i < 16; i++) {
                    assert(last_pressure[i] == (i == pad ? 1000 : 0));
                }
                mk1_device_dispatch_input_report(&reader, rest, sizeof(rest));
                assert(callbacks == before + 2);
                for (unsigned i = 0; i < 16; i++) assert(last_pressure[i] == 0);
            }
        }
    }

    // A malformed ID set or short report must not fabricate pad state.
    resting_frame(frame, 0);
    put_word(frame, 0x1000); // duplicate pad 1, missing pad 0
    unsigned before = callbacks;
    mk1_device_dispatch_input_report(&reader, frame, 64);
    assert(callbacks == before);
    mk1_device_dispatch_input_report(&reader, frame, 63);
    assert(callbacks == before);

    // Capture must retain rejected packets too, since those may explain a gap.
    dev.pad_capture = tmpfile();
    assert(dev.pad_capture);
    mk1_device_dispatch_input_report(&reader, frame, 64);
    assert(callbacks == before && dev.pad_capture_records == 1);
    rewind(dev.pad_capture);
    uint8_t captured[72];
    assert(fread(captured, sizeof(captured), 1, dev.pad_capture) == 1);
    assert(memcmp(captured + 8, frame, 64) == 0);
    fclose(dev.pad_capture);
    dev.pad_capture = NULL;

    puts("PASS: 512 strike/release cycles across all IDs and start positions; equal ADC passes; malformed packets; raw capture");
    return 0;
}
