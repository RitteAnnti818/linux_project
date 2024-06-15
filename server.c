#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_LOCKERS 10
#define LOCK_TIME 15
#define PORT 12345

typedef struct {
    int id;
    char password[20];
    int in_use;
    int lock_time;
    char items[100]; // 물건 저장
    char code[9];    // highlevel locker 코드
} Locker;

Locker lockers[MAX_LOCKERS];
pthread_mutex_t locker_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_server() {
    for (int i = 0; i < MAX_LOCKERS; i++) {
        lockers[i].id = i;
        strcpy(lockers[i].password, "");
        lockers[i].in_use = 0;
        lockers[i].lock_time = 0;
        strcpy(lockers[i].items, ""); // 물건 초기화
        strcpy(lockers[i].code, "");  // 코드 초기화
    }
}

void* lock_timer(void* arg) {
    int locker_id = *(int*)arg;
    sleep(LOCK_TIME);
    pthread_mutex_lock(&locker_mutex);
    lockers[locker_id].lock_time = 0;
    pthread_mutex_unlock(&locker_mutex);
    return NULL;
}

void generate_random_code(char *code, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    for (size_t i = 0; i < length; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        code[i] = charset[key];
    }
    code[length] = '\0';
}

int allocate_locker(int locker_id, const char *password, const char *items, char *code) {
    pthread_mutex_lock(&locker_mutex);
    if (lockers[locker_id].in_use) {
        pthread_mutex_unlock(&locker_mutex);
        return -1;
    }
    lockers[locker_id].in_use = 1;
    strcpy(lockers[locker_id].password, password);
    strcpy(lockers[locker_id].items, items);
    if (locker_id >= 1 && locker_id <= 3) {
        generate_random_code(code, 8);
        strcpy(lockers[locker_id].code, code);
    }
    pthread_mutex_unlock(&locker_mutex);
    return 0;
}

int access_locker(int locker_id, const char *password, const char *code, char *items) {
    pthread_mutex_lock(&locker_mutex);
    if (lockers[locker_id].lock_time > 0) {
        pthread_mutex_unlock(&locker_mutex);
        return -2;
    }
    if (strcmp(lockers[locker_id].password, password) != 0 || (locker_id >= 1 && locker_id <= 3 && strcmp(lockers[locker_id].code, code) != 0)) {
        lockers[locker_id].lock_time = LOCK_TIME;
        pthread_t tid;
        pthread_create(&tid, NULL, lock_timer, &lockers[locker_id].id);
        pthread_detach(tid);
        pthread_mutex_unlock(&locker_mutex);
        return -1;
    }
    strcpy(items, lockers[locker_id].items); // 물건 정보 반환
    pthread_mutex_unlock(&locker_mutex);
    return 0;
}

int store_items(int locker_id, const char *items) {
    pthread_mutex_lock(&locker_mutex);
    if (lockers[locker_id].in_use) {
        strcpy(lockers[locker_id].items, items);
        pthread_mutex_unlock(&locker_mutex);
        return 0;
    }
    pthread_mutex_unlock(&locker_mutex);
    return -1;
}

int change_password(int locker_id, const char *current_password, const char *new_password) {
    pthread_mutex_lock(&locker_mutex);
    if (strcmp(lockers[locker_id].password, current_password) == 0) {
        strcpy(lockers[locker_id].password, new_password);
        pthread_mutex_unlock(&locker_mutex);
        return 0;
    }
    pthread_mutex_unlock(&locker_mutex);
    return -1;
}

void show_lockers() {
    pthread_mutex_lock(&locker_mutex);
    for (int i = 0; i < MAX_LOCKERS; i++) {
        printf("Locker %d: %s\n", i, lockers[i].in_use ? "In Use" : "Available");
    }
    pthread_mutex_unlock(&locker_mutex);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    while (1) {
        int choice, locker_id;
        char password[20];
        char items[100];
        char code[9];
        read(client_socket, &choice, sizeof(choice));
        
        switch (choice) {
            case 1:
                pthread_mutex_lock(&locker_mutex);
                for (int i = 0; i < MAX_LOCKERS; i++) {
                    write(client_socket, &lockers[i], sizeof(Locker));
                }
                pthread_mutex_unlock(&locker_mutex);
                break;
            case 2:
                read(client_socket, &locker_id, sizeof(locker_id));
                read(client_socket, password, sizeof(password));
                read(client_socket, items, sizeof(items));
                int allocate_result = allocate_locker(locker_id, password, items, code);
                write(client_socket, &allocate_result, sizeof(allocate_result));
                if (allocate_result == 0 && locker_id >= 1 && locker_id <= 3) {
                    write(client_socket, code, sizeof(code));
                }
                break;
            case 3:
                read(client_socket, &locker_id, sizeof(locker_id));
                read(client_socket, password, sizeof(password));
                if (locker_id >= 1 && locker_id <= 3) {
                    read(client_socket, code, sizeof(code));
                }
                int access_result = access_locker(locker_id, password, locker_id >= 1 && locker_id <= 3 ? code : "", items);
                write(client_socket, &access_result, sizeof(access_result));
                if (access_result == 0) {
                    write(client_socket, items, sizeof(items));
                }
                break;
            case 4:
                read(client_socket, &locker_id, sizeof(locker_id));
                read(client_socket, password, sizeof(password));
                char new_password[20];
                read(client_socket, new_password, sizeof(new_password));
                int change_result = change_password(locker_id, password, new_password);
                write(client_socket, &change_result, sizeof(change_result));
                break;
            default:
                close(client_socket);
                return NULL;
        }
    }
}

int main() {
    srand(time(NULL));
    init_server();
    
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, 5);
    
    printf("Server is running on port %d\n", PORT);
    
    while (1) {
        int client_socket;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, pclient);
        pthread_detach(tid);
    }
    
    return 0;
}
