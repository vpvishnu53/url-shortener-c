/* hash.c — FNV-1a hashing and Base62 encoding. */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "shortener.h"

static const char *BASE62 =
    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

uint64_t fnv1a(const char *s) {
    uint64_t h = 1469598103934665603ULL;
    while (*s) { h ^= (unsigned char)(*s++); h *= 1099511628211ULL; }
    return h;
}

size_t idx_short(const char *s) { return fnv1a(s) % SHORT_TABLE_SIZE; }
size_t idx_long (const char *s) { return fnv1a(s) % LONG_TABLE_SIZE;  }
size_t idx_user (const char *s) { return fnv1a(s) % USER_TABLE_SIZE;  }

void id_to_base62(uint64_t id, char *out, size_t out_sz) {
    if (out_sz == 0) return;
    if (id == 0) { out[0] = BASE62[0]; out[1] = '\0'; return; }
    char tmp[16]; int pos = 0;
    while (id > 0 && pos < (int)sizeof(tmp) - 1) {
        tmp[pos++] = BASE62[id % 62]; id /= 62;
    }
    if (pos >= (int)out_sz) pos = (int)out_sz - 1;
    for (int i = 0; i < pos; ++i) out[i] = tmp[pos - 1 - i];
    out[pos] = '\0';
}
