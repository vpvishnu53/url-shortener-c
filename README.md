# URL Shortener

A local URL shortener written in C for Windows.

## Download

Grab the latest `shortener.exe` from the [Releases](https://github.com/vpvishnu53/url-shortener-c/releases) page. No build required.

## Build

Requires MinGW (GCC for Windows).

```
mingw32-make
```

Or manually:

```
gcc -Wall -Wextra -Iinclude -o shortener.exe src/main.c src/hash.c src/validate.c src/storage.c src/users.c src/urls.c src/http.c -lws2_32
```

## Commands

| Command | Description |
|---|---|
| `shorten <url> [alias]` | Shorten a URL with an optional custom alias |
| `resolve <short>` | Get the long URL for a short code |
| `lookup <url>` | Find the short code for a URL |
| `search <keyword>` | Search URLs and short codes |
| `open <short>` | Open a short code in the default browser |
| `delete <short>` | Delete an entry |
| `register <user>` | Create a user account |
| `login <user>` | Log in |
| `logout` | Log out |
| `whoami` | Show current logged-in user |
| `mylinks` | List your links and any anonymous links |
| `claimlinks` | Assign all anonymous links to the current user |
| `serve` | Start or restart the HTTP redirect server |
| `stopserver` | Stop the HTTP server |
| `help` | Show command list |
| `quit` | Exit |

## HTTP Server

`serve` starts a redirect server on `http://localhost:8080` in the background.
The CLI stays usable while it runs. Typing `serve` again restarts it.

Visit `http://localhost:8080/<short-code>` in any browser or use `open <short-code>` in the CLI to be redirected.

## Example Usage

```
> register example_user
Registered 'example_user'.

> login example_user
Logged in as 'example_user'.

> shorten https://github.com gh
short: gh

> shorten https://google.com
short: 4Kx9mZ

> resolve gh
long: https://github.com

> search github
[gh] https://github.com  (owner: example_user)

> open gh
Opening: https://github.com

> mylinks
=== Links owned by 'example_user' ===
  [gh] https://github.com
  [4Kx9mZ] https://google.com

> serve
Server started on http://localhost:8080
Type 'stopserver' to stop it.

> stopserver
Server stopped.
```

## Data Files

`store.txt` and `users.txt` are created automatically in the project folder on first use. Do not commit these to Git.

## Notes

- Links created before logging in are tagged as `anonymous`. Use `claimlinks` after logging in to reassign them to your account.
- Aliases can only contain letters, digits, `-` and `_` (max 32 characters).
- URLs must start with `http://` or `https://`.
