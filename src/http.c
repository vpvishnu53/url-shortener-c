/* http.c — Windows Winsock HTTP redirect server */

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "shortener.h"

#pragma comment(lib, "ws2_32.lib")

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

        send(client, response, strlen(response), 0);
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

        send(client, response, strlen(response), 0);
    }
    else {
        const char *body = "<h1>404 Not Found</h1>";
        char response[1024];

        sprintf(response,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n\r\n%s",
            (int)strlen(body), body);

        send(client, response, strlen(response), 0);
    }

    closesocket(client);
}

void start_http_server(void)
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 10);

    printf("HTTP server running at http://localhost:%d\n\n", HTTP_PORT);
    printf("Available links:\n");

    for (int i = 0; i < SHORT_TABLE_SIZE; i++) {
        Entry *e = short_table[i];

        while (e) {
            printf("http://localhost:%d/%s\n", HTTP_PORT, e->short_code);
            e = e->next_short;
        }
    }

    printf("\n");

    while (1) {
        SOCKET client = accept(server, NULL, NULL);

        if (client != INVALID_SOCKET)
            handle_client(client);
    }

    closesocket(server);
    WSACleanup();
}