/**
 * @brief Standalone debug tool to trace PlayerStats bit-level parsing.
 * Compile: g++ -o tools/trace_ps tools/trace_ps.cpp src/core/BinDataStream.cpp -I. -Isrc -Isrc/core -std=gnu++1z
 */
#include <cstdio>
#include <cstring>
#include <vector>
#include "core/BinDataStream.h"
#include "core/D2Types.h"

static const DWORD PLAYER_STATS_BITS_COUNT[16] = {
    10,10,10,10,10,8, 21,21,21,21,21,21, 7,32,25,25
};

int main(int argc, char* argv[]) {
    const char* path = argc > 1 ? argv[1] : "doc/杜五_v3.1.d2s";
    FILE* f = fopen(path, "rb");
    if (!f) { perror("fopen"); return 1; }
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<BYTE> buf(sz);
    fread(buf.data(), 1, sz, f);
    fclose(f);

    // Find 0x6667 PlayerStats marker
    for (size_t i = 0; i < sz - 1; i++) {
        if (*(WORD*)(&buf[i]) == 0x6667) {
            printf("Found 0x6667 at byte %zu\n", i);
            CInBitsStream bs(buf.data() + i, sz - i);
            WORD wMagic;
            bs >> wMagic;
            if (wMagic != 0x6667) { printf("Magic mismatch!\n"); continue; }

            WORD iEnd;
            printf("=== PlayerStats parse ===\n");
            for (bs >> bits(iEnd, 9); bs.Good() && iEnd < 16; bs >> bits(iEnd, 9)) {
                DWORD val = 0;
                DWORD nbits = PLAYER_STATS_BITS_COUNT[iEnd];
                printf("  Stat[%2u] @ stream_byte=%u bit=%u reading %u bits...",
                       (unsigned)iEnd, bs.BytePos(), bs.BitPos(), nbits);
                fflush(stdout);
                bs >> bits(val, nbits);
                printf(" val=%u\n", val);
            }
            printf("Loop exit: iEnd=0x%X Good=%d stream_byte=%u bit=%u\n",
                   (unsigned)iEnd, (int)bs.Good(), bs.BytePos(), bs.BitPos());
            bs.AlignByte();
            printf("After AlignByte: stream_byte=%u (file_byte=%zu)\n",
                   bs.BytePos(), i + bs.BytePos());
            break;
        }
    }
    return 0;
}
