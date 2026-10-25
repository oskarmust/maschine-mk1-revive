// Replay compact EP4 evidence through the real dispatcher, without USB hardware.
// Build: clang -O0 -framework CoreFoundation -framework IOKit
//        tests/replay-pad-capture.c -o build/replay-pad-capture
#define main regression_test_main
#include "pad_release_test.c"
#undef main

int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    FILE *input = fopen(argv[1], "rb");
    if (!input) return 2;
    mk1_device_t dev = { .pad_cb = record_pads };
    mk1_pipe_reader_t reader = { .device = &dev, .endpoint_number = 4 };
    uint8_t record[72];
    puts("timestamp_ns,phase,callback,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15");
    while (fread(record, sizeof(record), 1, input) == 1) {
        uint64_t timestamp = 0;
        for (unsigned i = 0; i < 8; i++) timestamp |= (uint64_t)record[i] << (8 * i);
        unsigned before = callbacks;
        mk1_device_dispatch_input_report(&reader, record + 8, 64);
        printf("%llu,%u,%u", (unsigned long long)timestamp,
               mk1_scan_phase(record + 8, 64), callbacks != before);
        for (unsigned i = 0; i < 16; i++) printf(",%u", last_pressure[i]);
        puts("");
    }
    fclose(input);
    return 0;
}
