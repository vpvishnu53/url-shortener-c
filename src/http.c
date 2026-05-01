/* http.c — Windows Winsock HTTP redirect server (background thread) */

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "shortener.h"

#pragma comment(lib, "ws2_32.lib")

/*
 * server_running: flag checked by stopserver command.
 * volatile tells the compiler this value can change from another thread,
 * so it must never cache it in a register — always read from memory.
 */
static volatile int server_running = 0;
static SOCKET       server_socket  = INVALID_SOCKET;

static void handle_client(SOCKET client)
{
    char buf[4096] = {0};
    recv(client, buf, sizeof(buf) - 1, 0);

    char method[8], path[256];

    if (sscanf(buf, "%7s %255s", method, path) != 2) {
        closesocket(client);
        return;
    }

    if (path[0] == '/')
        memmove(path, path + 1, strlen(path));

    if (strlen(path) == 0) {
        const char *body =
            "<h1>URL Shortener</h1>"
            "<p>Visit /code to redirect.</p>";

        char response[1024];
        sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n\r\n%s",
            (int)strlen(body), body);

        send(client, response, (int)strlen(response), 0);
        closesocket(client);
        return;
    }

    Entry *e = find_by_short(path);

    if (e) {
        char response[4096];
        sprintf(response,
            "HTTP/1.1 302 Found\r\n"
            "Location: %s\r\n"
            "Connection: close\r\n\r\n",
            e->long_url);
        send(client, response, (int)strlen(response), 0);
    } else {
        const char *body = "<h1>404 Not Found</h1>";
        char response[1024];
        sprintf(response,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n\r\n%s",
            (int)strlen(body), body);
        send(client, response, (int)strlen(response), 0);
    }

    closesocket(client);
}

/*
 * server_thread: the function that runs on the background thread.
 *
 * DWORD WINAPI is the required signature for a Windows thread function.
 * LPVOID is a void* parameter we don't use.
 *
 * accept() blocks here on the background thread, so the main thread
 * (the CLI) is completely free to keep running.
 *
 * The loop exits when server_running is set to 0 by stop_http_server(),
 * which also closes server_socket so accept() unblocks immediately and
 * returns INVALID_SOCKET, breaking the loop.
 */
static DWORD WINAPI server_thread(LPVOID unused)
{
    (void)unused;

    while (server_running) {
        SOCKET client = accept(server_socket, NULL, NULL);

        if (client == INVALID_SOCKET)
            break;

        handle_client(client);
    }

    return 0;
}

void start_http_server(void)
{
    /* If already running, stop first then fall through to restart */
    if (server_running) {
        printf("Server already running — restarting...\n");
        stop_http_server();
    }

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Error: could not create socket.\n");
        WSACleanup();
        return;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
               (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        printf("Error: could not bind to port %d. "
               "Is something else using it?\n", HTTP_PORT);
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
        WSACleanup();
        return;
    }

    listen(server_socket, 10);
    server_running = 1;

    /*
     * CreateThread spins up server_thread as a background thread.
     * CloseHandle is called immediately on the returned handle because
     * we don't need to wait on or control the thread later. Without
     * CloseHandle the handle leaks until the process exits.
     */
    HANDLE t = CreateThread(NULL, 0, server_thread, NULL, 0, NULL);
    if (!t) {
        printf("Error: could not create server thread.\n");
        server_running = 0;
        closesocket(server_socket);
        server_socket = INVALID_SOCKET;
        WSACleanup();
        return;
    }
    CloseHandle(t);

    printf("Server started on http://localhost:%d\n", HTTP_PORT);
    printf("Type 'stopserver' to stop it.\n\n");

    printf("Available links:\n");
    for (int i = 0; i < SHORT_TABLE_SIZE; i++) {
        Entry *e = short_table[i];
        while (e) {
            printf("  http://localhost:%d/%s\n", HTTP_PORT, e->short_code);
            e = e->next_short;
        }
    }
    printf("\n");
}

void stop_http_server(void)
{
    if (!server_running) {
        printf("Server is not running.\n");
        return;
    }

    server_running = 0;

    /*
     * Closing server_socket from this thread unblocks the accept() call
     * sitting in server_thread, causing it to exit cleanly.
     */
    closesocket(server_socket);
    server_socket = INVALID_SOCKET;

    WSACleanup();
    printf("Server stopped.\n");
}
