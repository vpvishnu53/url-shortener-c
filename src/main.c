/* main.c — CLI entry point and command dispatch. */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "shortener.h"

static void print_help(void) {
    puts("Commands:\n"
         "  shorten <url> [alias]  shorten URL with optional custom alias\n"
         "  resolve <short>        get long URL for a short code\n"
         "  lookup  <url>          find short code for a URL\n"
         "  search  <keyword>      search stored URLs and aliases\n"
         "  delete  <short>        delete an entry\n"
         "  register <user>        create a user account\n"
         "  login    <user>        log in\n"
         "  logout                 log out\n"
         "  whoami                 show current user\n"
         "  mylinks                list your links\n"
         "  serve                  start HTTP redirect server\n"
         "  help / quit");
}

int main(void) {
    puts("URL Shortener");
    load_users();
    load_store();
    printf("Next id = %" PRIu64 "\n", next_id);
    print_help();

    char input[4096];
    for (;;) {
        printf(current_user[0] ? "[%s]> " : "> ", current_user);
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        char *cmd = strtok(input, " ");
        if (!cmd) continue;

        if (!strcmp(cmd, "shorten")) {
            char *url   = strtok(NULL, " ");
            char *alias = strtok(NULL, "");
            if (!url) { puts("Usage: shorten <url> [alias]"); continue; }
            shorten_url(url, alias);
        }
        else if (!strcmp(cmd, "resolve")) {
            char *s = strtok(NULL, " ");
            if (!s) { puts("Usage: resolve <short>"); continue; }
            resolve_short(s);
        }
        else if (!strcmp(cmd, "lookup")) {
            char *u = strtok(NULL, "");
            if (!u) { puts("Usage: lookup <url>"); continue; }
            lookup_long(u);
        }
        else if (!strcmp(cmd, "search"))   { search_entries(strtok(NULL, ""));  }
        else if (!strcmp(cmd, "delete"))   { char *s = strtok(NULL," "); if(!s){puts("Usage: delete <short>"); continue;} delete_entry(s); }
        else if (!strcmp(cmd, "register")) { cmd_register(strtok(NULL, " ")); }
        else if (!strcmp(cmd, "login"))    { cmd_login(strtok(NULL, " "));    }
        else if (!strcmp(cmd, "logout"))   { cmd_logout();            }
        else if (!strcmp(cmd, "whoami"))   { cmd_whoami();            }
        else if (!strcmp(cmd, "mylinks"))  { mylinks();               }
        else if (!strcmp(cmd, "serve"))    { start_http_server();     }
        else if (!strcmp(cmd, "help"))     { print_help();            }
        else if (!strcmp(cmd, "quit"))     { break;                   }
        else puts("Unknown command. Type 'help'.");
    }

    free_all();
    puts("Goodbye.");
    return 0;
}
