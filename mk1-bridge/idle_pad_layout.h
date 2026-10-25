#ifndef MK1_IDLE_PAD_LAYOUT_H
#define MK1_IDLE_PAD_LAYOUT_H
#include <stdint.h>
/* Physical 1..16 calibration, 2026-10-25:
 * USB IDs: 12 13 14 15 / 8 9 10 11 / 4 5 6 7 / 0 1 2 3.
 * USB rows start at the top; printed pad numbers start at the bottom.
 * Only idle games use this conversion. Maschine receives its native USB IDs.
 */
static inline uint32_t idle_pad_physical_index(uint32_t usb_index)
{
    return usb_index < 16 ? usb_index ^ 12u : 16u;
}
#endif
