/* storage.c — global tables, entry lifecycle, and file persistence. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "shortener.h"

/* ── Global state (declared extern in shortener.h) ──────────────── */
Entry   *short_table[SHORT_TABLE_SIZE] = {0};
Entry   *long_table[LONG_TABLE_SIZE]   = {0};
User    *user_table[USER_TABLE_SIZE]   = {0};
uint64_t next_id                       = 1;
char     current_user[64]              = "";

/* ── Entry lifecycle ─────────────────────────────────────────────── */

Entry *create_entry(uint64_t id, const char *url,
                    const char *alias, int is_custom) {
    Entry *e = malloc(sizeof(Entry));
    if (!e) return NULL;
    e->id        = id;
    e->is_custom = is_custom;
    if (is_custom && alias)
        strncpy(e->short_code, alias, sizeof(e->short_code) - 1);
    else
        id_to_base62(id, e->short_code, sizeof(e->short_code));
    e->short_code[sizeof(e->short_code) - 1] = '\0';
    e->long_url = strdup(url);
    strncpy(e->owner, current_user[0] ? current_user : "anonymous",
            sizeof(e->owner) - 1);
    e->owner[sizeof(e->owner) - 1] = '\0';
    e->next_short = e->next_long = NULL;
    return e;
}

void insert_entry(Entry *e) {
    size_t si = idx_short(e->short_code);
    e->next_short   = short_table[si]; short_table[si] = e;
    size_t li = idx_long(e->long_url);
    e->next_long    = long_table[li];  long_table[li]  = e;
}

/* ── File persistence ────────────────────────────────────────────── */

void save_entry(const Entry *e) {
    FILE *f = fopen(STORE_FILE, "a");
    if (!f) return;
    fprintf(f, "%" PRIu64 "\t%s\t%s\t%s\t%d\n",
            e->id, e->short_code, e->long_url, e->owner, e->is_custom);
    fclose(f);
}

void load_store(void) {
    FILE *f = fopen(STORE_FILE, "r");
    if (!f) return;
    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        uint64_t id;
        char sc[MAX_ALIAS_LEN], url[MAX_URL_LEN], owner[64];
        int cust = 0;
        if (sscanf(line, "%" SCNu64 "\t%31s\t%2047[^\t]\t%63s\t%d",
                   &id, sc, url, owner, &cust) < 3) continue;
        Entry *e = malloc(sizeof(Entry));
        if (!e) continue;
        e->id = id; e->is_custom = cust;
        strncpy(e->short_code, sc,    sizeof(e->short_code) - 1);
        e->short_code[sizeof(e->short_code) - 1] = '\0';
        strncpy(e->owner,      owner, sizeof(e->owner) - 1);
        e->owner[sizeof(e->owner) - 1] = '\0';
        e->long_url   = strdup(url);
        e->next_short = e->next_long = NULL;
        insert_entry(e);
        if (id >= next_id) next_id = id + 1;
    }
    fclose(f);
}

void rebuild_store_file(void) {
    FILE *f = fopen(STORE_FILE, "w");
    if (!f) return;
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            fprintf(f, "%" PRIu64 "\t%s\t%s\t%s\t%d\n",
                    e->id, e->short_code, e->long_url,
                    e->owner, e->is_custom);
    fclose(f);
}

void load_users(void) {
    FILE *f = fopen(USERS_FILE, "r");
    if (!f) return;
    char line[128];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\n")] = '\0';
        if (!*line || find_user(line)) continue;
        User *u = malloc(sizeof(User));
        if (!u) continue;
        strncpy(u->username, line, sizeof(u->username) - 1);
        u->username[sizeof(u->username) - 1] = '\0';
        size_t i = idx_user(u->username);
        u->next = user_table[i]; user_table[i] = u;
    }
    fclose(f);
}

void free_all(void) {
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++) {
        Entry *e = long_table[i];
        while (e) { Entry *nx = e->next_long; free(e->long_url); free(e); e = nx; }
    }
    for (size_t i = 0; i < USER_TABLE_SIZE; i++) {
        User *u = user_table[i];
        while (u) { User *nx = u->next; free(u); u = nx; }
    }
}
