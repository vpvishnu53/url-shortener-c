/* users.c — user account registration, login, logout. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shortener.h"

User *find_user(const char *un) {
    for (User *u = user_table[idx_user(un)]; u; u = u->next)
        if (strcmp(u->username, un) == 0) return u;
    return NULL;
}

void cmd_register(const char *un) {
    if (!un || !*un)   { printf("Usage: register <username>\n");     return; }
    if (find_user(un)) { printf("User '%s' already exists.\n", un);  return; }

    User *u = malloc(sizeof(User));
    if (!u) return;
    strncpy(u->username, un, sizeof(u->username) - 1);
    u->username[sizeof(u->username) - 1] = '\0';

    size_t i = idx_user(un);
    u->next = user_table[i]; user_table[i] = u;

    FILE *f = fopen(USERS_FILE, "a");
    if (f) { fprintf(f, "%s\n", un); fclose(f); }

    printf("Registered '%s'.\n", un);
}

void cmd_login(const char *un) {
    if (!un)            { printf("Usage: login <username>\n");               return; }
    if (!find_user(un)) { printf("User '%s' not found. Register first.\n", un); return; }
    strncpy(current_user, un, sizeof(current_user) - 1);
    current_user[sizeof(current_user) - 1] = '\0';
    printf("Logged in as '%s'.\n", current_user);
}

void cmd_logout(void) {
    if (!current_user[0]) { printf("Not logged in.\n"); return; }
    printf("Logged out '%s'.\n", current_user);
    current_user[0] = '\0';
}

void cmd_whoami(void) {
    printf("%s\n", current_user[0] ? current_user : "(not logged in)");
}
