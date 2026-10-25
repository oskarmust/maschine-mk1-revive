// Exercise the actual LED emitter with USB writes captured in memory.
#include <assert.h>
#include "../mk1-usb/mk1_device.h"

static uint8_t packets[2][33];
static unsigned writes;
static bool test_usb_write(mk1_device_t *dev, uint8_t endpoint,
                           const void *data, size_t len);
#define mk1_device_write_endpoint test_usb_write
#define main bridge_application_main
#include "../mk1-bridge/main.c"
#undef main
#undef mk1_device_write_endpoint

static bool test_usb_write(mk1_device_t *dev, uint8_t endpoint,
                           const void *data, size_t len)
{
    assert(dev && endpoint == 1 && len == 33 && writes < 2);
    memcpy(packets[writes++], data, len);
    return true;
}

static void emit(const uint8_t logical[57])
{
    bridge_t bridge = { .usb = (mk1_device_t *)(uintptr_t)1 };
    g_quiet = true;
    writes = 0;
    emit_led_state(&bridge, logical, 57);
    assert(writes == 2);
    assert(packets[0][0] == 0x0c && packets[0][1] == 0);
    assert(packets[1][0] == 0x0c && packets[1][1] == 0x1e);
}

int main(void)
{
    uint8_t logical[57] = {0};
    uint8_t baseline_b[33];
    logical[39] = 0x32; // selected Group A stays lit
    emit(logical);
    memcpy(baseline_b, packets[1], 33);

    // A pad-only change must not change any Packet B controller LED.
    for (unsigned pad_slot = 0; pad_slot < 16; pad_slot++) {
        logical[pad_slot] = 0x3f;
        emit(logical);
        assert(packets[0][2 + pad_slot] == 0x3f);
        assert(memcmp(packets[1], baseline_b, 33) == 0);
        logical[pad_slot] = 0;
    }

    // Explicit zeros from Maschine turn off controller LEDs, even if a pad
    // value would have lit the same slot through the obsolete remap.
    logical[2] = 0x3f;
    logical[32] = 0x32;
    emit(logical);
    assert(packets[1][4] == 0x32); // Group H, device slot 33
    logical[32] = 0;
    emit(logical);
    assert(packets[1][4] == 0);

    // Preserve software-requested controller flashes, including active Group A.
    // The correction removes accidental legacy routing, not real LED feedback.
    for (unsigned li = 32; li <= 56; li++) {
        logical[li] = 0x3f;
        emit(logical);
        assert(packets[1][li - 28] == 0x3f);
        logical[li] = 0;
        emit(logical);
        assert(packets[1][li - 28] == 0);
    }

    puts("PASS: all 16 pad LEDs isolated; controller OFF and genuine flashes preserved");
    return 0;
}
