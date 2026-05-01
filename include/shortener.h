#ifndef SHORTENER_H
#define SHORTENER_H

/*
 * shortener.h — shared types, constants, and extern declarations.
 * Every .c file in the project includes this header.
 */

#include <stdint.h>
#include <stddef.h>

/* ── Constants ──────────────────────────────────────────────────── */

#define SHORT_TABLE_SIZE 10007
#define LONG_TABLE_SIZE  20011
#define USER_TABLE_SIZE  1009
#define STORE_FILE       "store.txt"
#define USERS_FILE       "users.txt"
#define HTTP_PORT        8080
#define MAX_ALIAS_LEN    32
#define MIN_URL_LEN      13
#define MAX_URL_LEN      2048

/* ── Types ──────────────────────────────────────────────────────── */

typedef struct Entry {
    uint64_t  id;
    char      short_code[MAX_ALIAS_LEN];
    char     *long_url;
    char      owner[64];
    int       is_custom;
    struct Entry *next_short;
    struct Entry *next_long;
} Entry;

typedef struct User {
    char         username[64];
    struct User *next;
} User;

/* ── Shared global state (defined in storage.c) ─────────────────── */

extern Entry   *short_table[SHORT_TABLE_SIZE];
extern Entry   *long_table[LONG_TABLE_SIZE];
extern User    *user_table[USER_TABLE_SIZE];
extern uint64_t next_id;
extern char     current_user[64];

/* ── hash.c ─────────────────────────────────────────────────────── */
uint64_t fnv1a(const char *s);
size_t   idx_short(const char *s);
size_t   idx_long (const char *s);
size_t   idx_user (const char *s);
void     id_to_base62(uint64_t id, char *out, size_t out_sz);

/* ── validate.c ─────────────────────────────────────────────────── */
int validate_url  (const char *url);
int validate_alias(const char *alias);

/* ── storage.c ──────────────────────────────────────────────────── */
Entry *create_entry(uint64_t id, const char *url, const char *alias, int is_custom);
void   insert_entry(Entry *e);
void   save_entry  (const Entry *e);
void   load_store  (void);
void   rebuild_store_file(void);
void   load_users  (void);
void   free_all    (void);

/* ── users.c ────────────────────────────────────────────────────── */
User *find_user   (const char *username);
void  cmd_register(const char *username);
void  cmd_login   (const char *username);
void  cmd_logout  (void);
void  cmd_whoami  (void);

/* ── urls.c ─────────────────────────────────────────────────────── */
Entry *find_by_short(const char *sc);
Entry *find_by_long (const char *url);
void   shorten_url  (const char *url, const char *alias);
void   resolve_short(const char *sc);
void   lookup_long  (const char *url);
void   search_entries(const char *keyword);
void   mylinks       (void);
void   claimlinks    (void);
int    delete_entry  (const char *sc);
void   open_url      (const char *sc);

/* ── http.c ─────────────────────────────────────────────────────── */
void start_http_server(void);
void stop_http_server (void);

#endif /* SHORTENER_H */
