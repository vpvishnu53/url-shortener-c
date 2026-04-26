/* urls.c — URL shortening, lookup, search, and deletion. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shortener.h"

Entry *find_by_short(const char *sc) {
    for (Entry *e = short_table[idx_short(sc)]; e; e = e->next_short)
        if (strcmp(e->short_code, sc) == 0) return e;
    return NULL;
}

Entry *find_by_long(const char *url) {
    for (Entry *e = long_table[idx_long(url)]; e; e = e->next_long)
        if (strcmp(e->long_url, url) == 0) return e;
    return NULL;
}

void shorten_url(const char *url, const char *alias) {
    if (!validate_url(url)) return;

    if (alias && *alias) {
        if (!validate_alias(alias)) return;
        if (find_by_short(alias)) {
            printf("Error: alias '%s' is already taken.\n", alias); return;
        }
        Entry *e = create_entry(next_id++, url, alias, 1);
        if (!e) return;
        insert_entry(e); save_entry(e);
        printf("short: %s\n", e->short_code);
    } else {
        Entry *exist = find_by_long(url);
        if (exist) { printf("short: %s\n", exist->short_code); return; }
        Entry *e = create_entry(next_id++, url, NULL, 0);
        if (!e) return;
        insert_entry(e); save_entry(e);
        printf("short: %s\n", e->short_code);
    }
}

void resolve_short(const char *sc) {
    Entry *e = find_by_short(sc);
    printf(e ? "long: %s\n" : "not found\n", e ? e->long_url : "");
}

void lookup_long(const char *url) {
    if (!validate_url(url)) return;
    Entry *e = find_by_long(url);
    printf(e ? "short: %s\n" : "not found\n", e ? e->short_code : "");
}

void search_entries(const char *kw) {
    if (!kw || !*kw) { printf("Usage: search <keyword>\n"); return; }
    int n = 0;
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (strstr(e->long_url, kw) || strstr(e->short_code, kw)) {
                printf("[%s] %s  (owner: %s)\n",
                       e->short_code, e->long_url, e->owner);
                n++;
            }
    if (!n) printf("No results for '%s'.\n", kw);
}

void mylinks(void) {
    if (!current_user[0]) { printf("Not logged in.\n"); return; }
    int n = 0;
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (strcmp(e->owner, current_user) == 0) {
                printf("[%s] %s\n", e->short_code, e->long_url); n++;
            }
    if (!n) printf("No links for '%s'.\n", current_user);
}

int delete_entry(const char *sc) {
    size_t si = idx_short(sc);
    Entry *prev = NULL, *e = short_table[si];
    for (; e; prev = e, e = e->next_short) {
        if (strcmp(e->short_code, sc) != 0) continue;
        if (prev) prev->next_short = e->next_short;
        else      short_table[si]  = e->next_short;

        size_t li = idx_long(e->long_url);
        Entry *pl = NULL, *cl = long_table[li];
        for (; cl; pl = cl, cl = cl->next_long)
            if (strcmp(cl->long_url, e->long_url) == 0) {
                if (pl) pl->next_long = cl->next_long;
                else    long_table[li] = cl->next_long;
                break;
            }
        printf("Deleted '%s' -> %s\n", e->short_code, e->long_url);
        free(e->long_url); free(e);
        rebuild_store_file();
        return 1;
    }
    printf("Short code '%s' not found.\n", sc);
    return 0;
}
