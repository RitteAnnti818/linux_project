#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 9999
#define MAX_LOCKERS 10
#define BUF_SIZE 1024
#define CODE_SIZE 8

typedef struct {
    char code[CODE_SIZE + 1];
    char password[BUF_SIZE];
    char content[BUF_SIZE];
} Locker;

Locker lockers[MAX_LOCKERS];

void generate_code(char* code, int length) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (int i = 0; i < length; i++) {
        code[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    code[length] = '\0';
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);

    char buffer[BUF_SIZE];
    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int read_size = read(client_socket, buffer, BUF_SIZE - 1);
        if (read_size <= 0) break;

        char command[BUF_SIZE], response[BUF_SIZE];
        int locker_num;
        char code[CODE_SIZE + 1], password[BUF_SIZE], content[BUF_SIZE];

        sscanf(buffer, "%s", command);

        if (strcmp(command, "INFO") == 0) {
            strcpy(response, "Available lockers:\n");
            for (int i = 0; i < MAX_LOCKERS; i++) {
                char status[BUF_SIZE];
                snprintf(status, BUF_SIZE, "Locker %d: %s\n", i + 1, lockers[i].content[0] == '\0' ? "Empty" : "Occupied");
                strcat(response, status);
            }
        } else if (strcmp(command, "SET_PASSWORD") == 0) {
            sscanf(buffer, "%*s %d %s", &locker_num, password);
            if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                snprintf(response, BUF_SIZE, "Invalid locker number\n");
            } else {
                strcpy(lockers[locker_num - 1].password, password);
                snprintf(response, BUF_SIZE, "Password set for locker %d\n", locker_num);
            }
        } else if (strcmp(command, "STORE") == 0) {
            sscanf(buffer, "%*s %d %s %[^\n]", &locker_num, password, content);
            if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                snprintf(response, BUF_SIZE, "Invalid locker number\n");
            } else if (strcmp(lockers[locker_num - 1].password, password) != 0) {
                snprintf(response, BUF_SIZE, "Incorrect password\n");
            } else {
                strcpy(lockers[locker_num - 1].content, content);
                snprintf(response, BUF_SIZE, "Stored in locker %d\n", locker_num);
            }
        } else if (strcmp(command, "RETRIEVE") == 0) {
            sscanf(buffer, "%*s %d %s", &locker_num, password);
            if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                snprintf(response, BUF_SIZE, "Invalid locker number\n");
            } else if (strcmp(lockers[locker_num - 1].password, password) != 0) {
                snprintf(response, BUF_SIZE, "Incorrect password\n");
            } else {
                snprintf(response, BUF_SIZE, "Locker %d contains: %s\n", locker_num, lockers[locker_num - 1].content);
            }
        } else if (strcmp(command, "GET_CODE") == 0) {
            sscanf(buffer, "%*s %d", &locker_num);
            if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                snprintf(response, BUF_SIZE, "Invalid locker number\n");
            } else {
                generate_code(lockers[locker_num - 1].code, CODE_SIZE);
                snprintf(response, BUF_SIZE, "Locker %d code: %s\n", locker_num, lockers[locker_num - 1].code);
            }
        } else if (strcmp(command, "CHECK_CODE") == 0) {
            sscanf(buffer, "%*s %d %s", &locker_num, code);
            if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                snprintf(response, BUF_SIZE, "Invalid locker number\n");
            } else if (strcmp(lockers[locker_num - 1].code, code) != 0) {
                snprintf(response, BUF_SIZE, "Incorrect code\n");
            } else {
                snprintf(response, BUF_SIZE, "Correct code for locker %d\n", locker_num);
            }
        } else {
            snprintf(response, BUF_SIZE, "Unknown command\n");
        }

        write(client_socket, response, strlen(response));
    }

    close(client_socket);
    return NULL;
}

int main() {
    srand(time(NULL)); // Initialize random number generator

    int server_socket, client_socket, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t client_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_socket;

        if (pthread_create(&client_thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Thread creation failed");
            free(new_sock);
        }

        pthread_detach(client_thread);
    }

    close(server_socket);
    return 0;
}
