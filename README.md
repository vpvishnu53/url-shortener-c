# URL Shortener

A local URL shortener written in C. Runs as a CLI and optionally as a
background HTTP redirect server on Windows.

## Build

Requires MinGW (GCC for Windows) or any C compiler with Winsock support.

```
mingw32-make
```

Or manually:

```
gcc -Wall -Wextra -Iinclude -o shortener.exe src/main.c src/hash.c src/validate.c src/storage.c src/users.c src/urls.c src/http.c -lws2_32
```

## Commands

| Command                  | Description                              |
|--------------------------|------------------------------------------|
| `shorten <url> [alias]`  | Shorten a URL with an optional alias     |
| `resolve <short>`        | Get the long URL for a short code        |
| `lookup <url>`           | Find the short code for a URL            |
| `search <keyword>`       | Search URLs and aliases                  |
| `open <short>`           | Open a short code in the default browser |
| `delete <short>`         | Delete an entry                          |
| `register <user>`        | Create a user account                    |
| `login <user>`           | Log in                                   |
| `logout`                 | Log out                                  |
| `whoami`                 | Show current logged-in user              |
| `mylinks`                | List all links created by current user   |
| `serve`                  | Start HTTP redirect server (background)  |
| `stopserver`             | Stop the HTTP server                     |
| `help`                   | Show command list                        |
| `quit`                   | Exit                                     |

## Data Files

`store.txt` and `users.txt` are created automatically in the same folder
as the executable when first used.

## HTTP Server

`serve` starts a redirect server on `http://localhost:8080` in the
background. The CLI remains usable while it runs. Visit
`http://localhost:8080/<short-code>` in any browser to be redirected.
Use `stopserver` to shut it down.
