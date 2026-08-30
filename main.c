#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

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

        bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

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

        printf("Received:\n%s\n", buffer);

        char method[16];
        char path[256];
        char version[16];

        if (sscanf(buffer, "%15s %255s %15s", method, path, version) != 3) {
            printf("Invalid HTTP request\n");
            close(client_fd);
            continue;
        }

        printf("Method: %s\n", method);
        printf("Path: %s\n", path);
        printf("Version: %s\n", version);

        if (strcmp(method, "GET") != 0) {
            const char *response =
                "HTTP/1.1 405 Method Not Allowed\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 19\r\n"
                "\r\n"
                "Method Not Allowed\n";

            send(client_fd, response, strlen(response), 0);
            close(client_fd);
            continue;
        }

        char body[4096];
        int status_code;

        if (strcmp(path, "/") == 0) {

            int file_fd = open("public/index.html", O_RDONLY);

            if (file_fd == -1) {
                perror("open");
                strcpy(body, "Not Found\n");
                status_code = 404;
            } else {
                int bytes_read = read(file_fd, body, sizeof(body) - 1);
                close(file_fd);

                if (bytes_read == -1) {
                    perror("read");
                    strcpy(body, "Internal Server Error\n");
                    status_code = 500;
                } else {
                    body[bytes_read] = '\0';
                    status_code = 200;
                }
            }

       } else if (strcmp(path, "/hello") == 0) {
            strcpy(body, "Hello from TinyHTTP!\n");
            status_code = 200;

        } else if (strcmp(path, "/style.css") == 0) {
            int file_fd = open("public/style.css", O_RDONLY);

            if (file_fd == -1) {
                perror("open");
                strcpy(body, "Not Found\n");
                status_code = 404;
            } else {
                int bytes_read = read(file_fd, body, sizeof(body) - 1);
                close(file_fd);

                if (bytes_read == -1) {
                    perror("read");
                    strcpy(body, "Internal Server Error\n");
                    status_code = 500;
                } else {
                    body[bytes_read] = '\0';
                    status_code = 200;
                }
            }

        } else  {
            strcpy(body, "Not Found\n");
            status_code = 404;
        }

        const char *status_text;
        switch (status_code) {
            case 200: status_text = "OK"; break;
            case 404: status_text = "Not Found"; break;
            case 500: status_text = "Internal Server Error"; break;
            default:  status_text = "Unknown"; break;
        }

        char response[8192];

        const char *content_type;

        if (strcmp(path, "/") == 0) {
            content_type = "text/html";
        } else if (strcmp(path, "/style.css") == 0) {
            content_type = "text/css";
        } else {
            content_type = "text/plain";
        }

        snprintf(response, sizeof(response),
         "HTTP/1.1 %d %s\r\n"
         "Content-Type: %s\r\n"
         "Content-Length: %zu\r\n"
         "\r\n"
         "%s",
         status_code, status_text, content_type, strlen(body), body);

        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}