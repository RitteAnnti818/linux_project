#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUF_SIZE 1024

void error_handling(char *message) {
    perror(message);
    exit(1);
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    while (1) {
        printf("Enter command:\n{locker(show locker information)\npassword <locker_num> <password>\nstore<locker_num> <password> <content>\nshow <locker_num> <password>} ");
        fgets(message, BUF_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;  // Remove newline character

        if (strcmp(message, "exit") == 0) break;

        write(sock, message, strlen(message));

        str_len = read(sock, message, BUF_SIZE - 1);
        if (str_len == -1) error_handling("read() error");

        message[str_len] = 0;
        printf("Server response: %s\n", message);
    }

    close(sock);
    return 0;
}

