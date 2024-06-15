#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define PORT 12345

typedef struct {
    int id;
    char password[20];
    int in_use;
    int lock_time;
    char items[100];
    char code[9];    // highlevel locker 코드
} Locker;

void show_menu() {
    printf("1. 사물함 정보\n");
    printf("2. 사물함 설정 (비밀번호 및 물건 저장)\n");
    printf("3. 사물함 접근 (비밀번호 확인 및 물건 반환)\n");
    printf("4. 비밀번호 변경\n");
    printf("5. 종료\n");
}

void locker_info(int client_socket) {
    int choice = 1;
    write(client_socket, &choice, sizeof(choice));
    Locker lockers[10];
    for (int i = 0; i < 10; i++) {
        read(client_socket, &lockers[i], sizeof(Locker));
        printf("Locker %d: %s\n", lockers[i].id, lockers[i].in_use ? "In Use" : "Available");
    }
}

void setup_locker(int client_socket) {
    int choice = 2, locker_id;
    char password[20];
    char items[100];
    char code[9];
    
    printf("사물함 번호를 입력하세요 (1~10): ");
    scanf("%d", &locker_id);
    printf("비밀번호를 설정하세요: ");
    scanf("%s", password);
    printf("저장할 물건을 입력하세요: ");
    scanf(" %[^\n]", items);
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, password, sizeof(password));
    write(client_socket, items, sizeof(items));
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        printf("Locker %d successfully set up.\n", locker_id);
        if (locker_id >= 1 && locker_id <= 3) {
            read(client_socket, code, sizeof(code));
            printf("Your access code is: %s\n", code);
        }
    } else {
        printf("Failed to set up Locker %d.\n", locker_id);
    }
}

void access_locker(int client_socket) {
    int choice = 3, locker_id;
    char password[20];
    char items[100];
    char code[9] = "";
    
    printf("사물함 번호를 입력하세요 (1~10): ");
    scanf("%d", &locker_id);
    printf("비밀번호를 입력하세요: ");
    scanf("%s", password);
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, password, sizeof(password));
    
    if (locker_id >= 1 && locker_id <= 3) {
        printf("Access code를 입력하세요: ");
        scanf("%s", code);
        write(client_socket, code, sizeof(code));
    }
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        read(client_socket, items, sizeof(items));
        printf("Access to Locker %d granted.\n", locker_id);
        printf("Stored items: %s\n", items);
    } else if (result == -1) {
        printf("Incorrect password or code. Locker %d is locked for 15 seconds.\n", locker_id);
    } else if (result == -2) {
        printf("Locker %d is locked. Try again later.\n", locker_id);
    }
}

void change_password(int client_socket) {
    int choice = 4, locker_id;
    char current_password[20], new_password[20], confirm_password[20];
    
    printf("사물함 번호를 입력하세요: ");
    scanf("%d", &locker_id);
    printf("현재 비밀번호를 입력하세요: ");
    scanf("%s", current_password);
    printf("새로운 비밀번호를 입력하세요: ");
    scanf("%s", new_password);
    printf("비밀번호 확인을 입력하세요: ");
    scanf("%s", confirm_password);
    
    if (strcmp(new_password, confirm_password) != 0) {
        printf("비밀번호가 일치하지 않습니다.\n");
        return;
    }
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, current_password, sizeof(current_password));
    write(client_socket, new_password, sizeof(new_password));
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        printf("비밀번호가 성공적으로 변경되었습니다.\n");
    } else {
        printf("비밀번호 변경에 실패했습니다.\n");
    }
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_port = htons(PORT);
    
    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    int choice;
    while (1) {
        show_menu();
        printf("선택하세요: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                locker_info(client_socket);
                break;
            case 2:
                setup_locker(client_socket);
                break;
            case 3:
                access_locker(client_socket);
                break;
            case 4:
                change_password(client_socket);
                break;
            case 5:
                close(client_socket);
                exit(0);
            default:
                printf("잘못된 선택입니다.\n");
        }
    }
    
    return 0;
}
