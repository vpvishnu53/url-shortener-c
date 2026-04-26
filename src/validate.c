/* validate.c — URL and alias input validation. */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "shortener.h"

int validate_url(const char *url) {
    if (!url) return 0;
    size_t len = strlen(url);
    if (len < MIN_URL_LEN || len > MAX_URL_LEN) {
        printf("Error: URL length must be %d-%d chars.\n", MIN_URL_LEN, MAX_URL_LEN);
        return 0;
    }
    int is_https = (strncmp(url, "https://", 8) == 0);
    int is_http  = (strncmp(url, "http://",  7) == 0);
    if (!is_http && !is_https) {
        printf("Error: URL must start with http:// or https://\n"); return 0;
    }
    if (strchr(url, ' ')) {
        printf("Error: URL must not contain spaces.\n"); return 0;
    }
    if (!strchr(url + (is_https ? 8 : 7), '.')) {
        printf("Error: URL missing domain dot.\n"); return 0;
    }
    return 1;
}

int validate_alias(const char *alias) {
    if (!alias) return 0;
    size_t len = strlen(alias);
    if (len < 1 || len >= MAX_ALIAS_LEN) {
        printf("Error: Alias must be 1-%d chars.\n", MAX_ALIAS_LEN - 1); return 0;
    }
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)alias[i];
        if (!isalnum(c) && c != '-' && c != '_') {
            printf("Error: Alias may only contain [a-zA-Z0-9_-].\n"); return 0;
        }
    }
    return 1;
}
