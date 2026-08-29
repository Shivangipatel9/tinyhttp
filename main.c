#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(void)
{
    int server_fd;
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[4096];
    int bytes_received;

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

        buffer[bytes_received] = '\0';

        printf("Received:\n%s\n", buffer);

        close(client_fd);
    }

    close(server_fd);

    return 0;
}