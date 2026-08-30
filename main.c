#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void)
{
    int server_fd;
    int client_fd;
    int bytes_received;

    struct sockaddr_in server_addr;

    char buffer[4096];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) == -1) {

        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) == -1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    printf("Listening on 127.0.0.1:8080\n");

    while (1) {

        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("Client connected!\n");

        bytes_received = recv(client_fd,
                              buffer,
                              sizeof(buffer) - 1,
                              0);

        if (bytes_received == -1) {
            perror("recv");
            close(client_fd);
        continue;
        }

        if (bytes_received == 0) {
            printf("Client closed the connection.\n");
            close(client_fd);
            continue;
        }

        buffer[bytes_received] = '\0';

        printf("Received:\n");
        printf("%s\n", buffer);

        char method[16];
        char path[256];
        char version[16];

        if (sscanf(buffer, "%15s %255s %15s", method, path, version) == 3) {
            printf("Method: %s\n", method);
            printf("Path: %s\n", path);
            printf("Version: %s\n", version);
        }

        char *headers = strstr(buffer, "\r\n");

        if (headers != NULL) {
            headers += 2;

            printf("Headers:\n");
            printf("%s\n", headers);
        }

        const char *body = "Hello, world!\n";
        char response[1024];

        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/plain\r\n"
                 "Content-Length: %zu\r\n"
                 "\r\n"
                 "%s",
                 strlen(body), body);

        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }

    close(server_fd);

    return 0;
}