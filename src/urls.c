/* urls.c — URL shortening, lookup, search, and deletion. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "shortener.h"

/*
 * can_access: returns 1 if the current session is allowed to use this entry.
 *
 * Rules:
 *   - anonymous entries are public — anyone can access them
 *   - owned entries can only be accessed by that owner
 *   - if nobody is logged in, only anonymous entries are accessible
 */
static int can_access(const Entry *e) {
    int is_anon = (strcmp(e->owner, "anonymous") == 0 ||
                   strcmp(e->owner, "")          == 0);
    if (is_anon)            return 1;
    if (!current_user[0])  return 0;
    return strcmp(e->owner, current_user) == 0;
}

static void access_denied(const Entry *e) {
    if (!current_user[0])
        printf("'%s' is owned by '%s'. Log in to access it.\n",
               e->short_code, e->owner);
    else
        printf("'%s' belongs to '%s', not '%s'.\n",
               e->short_code, e->owner, current_user);
}

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
        /* Dedup: only reuse if the existing entry is accessible */
        Entry *exist = find_by_long(url);
        if (exist && can_access(exist)) {
            printf("short: %s\n", exist->short_code); return;
        }
        Entry *e = create_entry(next_id++, url, NULL, 0);
        if (!e) return;
        insert_entry(e); save_entry(e);
        printf("short: %s\n", e->short_code);
    }
}

void resolve_short(const char *sc) {
    Entry *e = find_by_short(sc);
    if (!e)              { printf("not found\n"); return; }
    if (!can_access(e))  { access_denied(e);      return; }
    printf("long: %s\n", e->long_url);
}

void lookup_long(const char *url) {
    if (!validate_url(url)) return;
    Entry *e = find_by_long(url);
    if (!e)             { printf("not found\n"); return; }
    if (!can_access(e)) { access_denied(e);      return; }
    printf("short: %s\n", e->short_code);
}

/* search only shows entries the current session can access */
void search_entries(const char *kw) {
    if (!kw || !*kw) { printf("Usage: search <keyword>\n"); return; }
    int n = 0;
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (can_access(e) &&
                (strstr(e->long_url, kw) || strstr(e->short_code, kw))) {
                printf("[%s] %s  (owner: %s)\n",
                       e->short_code, e->long_url, e->owner);
                n++;
            }
    if (!n) printf("No results for '%s'.\n", kw);
}

void mylinks(void) {
    if (!current_user[0]) { printf("Not logged in.\n"); return; }

    int mine = 0, anon = 0;

    printf("=== Links owned by '%s' ===\n", current_user);
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (strcmp(e->owner, current_user) == 0) {
                printf("  [%s] %s\n", e->short_code, e->long_url);
                mine++;
            }
    if (!mine) printf("  (none)\n");

    printf("\n=== Anonymous links ===\n");
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (strcmp(e->owner, "anonymous") == 0 ||
                strcmp(e->owner, "")          == 0) {
                printf("  [%s] %s\n", e->short_code, e->long_url);
                anon++;
            }
    if (!anon) printf("  (none)\n");
    else       printf("\n  Tip: type 'claimlinks' to assign these to '%s'.\n",
                      current_user);

    printf("\nTotal: %d owned, %d anonymous\n", mine, anon);
}

void claimlinks(void) {
    if (!current_user[0]) { printf("Not logged in.\n"); return; }
    int claimed = 0;
    for (size_t i = 0; i < LONG_TABLE_SIZE; i++)
        for (Entry *e = long_table[i]; e; e = e->next_long)
            if (strcmp(e->owner, "anonymous") == 0 ||
                strcmp(e->owner, "")          == 0) {
                strncpy(e->owner, current_user, sizeof(e->owner) - 1);
                e->owner[sizeof(e->owner) - 1] = '\0';
                claimed++;
            }
    if (claimed) {
        rebuild_store_file();
        printf("Claimed %d link(s) for '%s'.\n", claimed, current_user);
    } else {
        printf("No anonymous links to claim.\n");
    }
}

void open_url(const char *sc) {
    Entry *e = find_by_short(sc);
    if (!e)             { printf("Short code '%s' not found.\n", sc); return; }
    if (!can_access(e)) { access_denied(e);                           return; }
    printf("Opening: %s\n", e->long_url);
    ShellExecuteA(NULL, "open", e->long_url, NULL, NULL, SW_SHOWNORMAL);
}

int delete_entry(const char *sc) {
    size_t si = idx_short(sc);
    Entry *prev = NULL, *e = short_table[si];
    for (; e; prev = e, e = e->next_short) {
        if (strcmp(e->short_code, sc) != 0) continue;

        if (!can_access(e)) { access_denied(e); return 0; }

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
