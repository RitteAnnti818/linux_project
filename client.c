#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9999
#define BUF_SIZE 1024
#define MAX_LOCKERS 10

void error_handling(char *message) {
    perror(message);
    exit(1);
}

void show_menu() {
    printf("\n=========================\n\t  MENU\n=========================\n");
    printf("1. INFORMATION OF LOCKERS\n");
    printf("2. SET PASSWORD\n");
    printf("3. STORE CONTENTS\n");
    printf("4. SHOW LOCKER\n");
    printf("5. END\n");
    printf("=========================\n");
    printf("CHOOSE MENU: ");
}

void password_instructions(int locker_num) {
    printf("**********PASSWORD**********\n");
    printf("how to: password %d <your password>\n", locker_num);
}

void store_instructions(int locker_num, int high_level) {
    if (high_level) {
        printf("=========STORE=========\n");
        printf("how to: store %d <your password> <high level code> <content>\n", locker_num);
    } else {
        printf("=========STORE=========\n");
        printf("how to: store %d <your password> <content>\n", locker_num);
    }
}

void show_instructions(int locker_num, int high_level) {
    if (high_level) {
        printf("=========SHOW========\n");
        printf("how to: show %d <your password> <high level code>\n", locker_num);
    } else {
        printf("=========SHOW========\n");
        printf("how to: show %d <your password>\n", locker_num);
    }
}

int main() {
    int sock;
    struct sockaddr_in serv_addr;
    char message[BUF_SIZE];
    int str_len;
    int locker_high_level[MAX_LOCKERS] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0};  // 하이레벨 사물함 정보

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("connect() error");
    }

    printf("\n********************WELCOME TO LOCKER SERVICE********************\n");
    while (1) {
        show_menu();
        fgets(message, BUF_SIZE, stdin);
        int choice = atoi(message);

        if (choice == 5) break;

        switch (choice) {
            case 1:
                strcpy(message, "info");
                write(sock, message, strlen(message));
                str_len = read(sock, message, BUF_SIZE - 1);
                if (str_len == -1) error_handling("read() error");
                message[str_len] = 0;
                printf("서버 응답: %s\n", message);
                break;

            case 2:
                printf("CHOOSE LOCKER: ");
                fgets(message, BUF_SIZE, stdin);
                int locker_num = atoi(message);
                if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                    printf("WORNG LOCKER NUMBER.\nCHOOSE AGIAN.\n");
                    continue;
                }
                password_instructions(locker_num);
                printf("명령어를 입력하세요: ");
                fgets(message, BUF_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;  // 개행 문자 제거
                break;

            case 3:
                printf("사물함 번호를 입력하세요: ");
                fgets(message, BUF_SIZE, stdin);
                locker_num = atoi(message);
                if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                    printf("잘못된 사물함 번호입니다. 다시 시도하세요.\n");
                    continue;
                }
                store_instructions(locker_num, locker_high_level[locker_num - 1]);
                printf("명령어를 입력하세요: ");
                fgets(message, BUF_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;  // 개행 문자 제거
                break;

            case 4:
                printf("사물함 번호를 입력하세요: ");
                fgets(message, BUF_SIZE, stdin);
                locker_num = atoi(message);
                if (locker_num < 1 || locker_num > MAX_LOCKERS) {
                    printf("잘못된 사물함 번호입니다. 다시 시도하세요.\n");
                    continue;
                }
                show_instructions(locker_num, locker_high_level[locker_num - 1]);
                printf("명령어를 입력하세요: ");
                fgets(message, BUF_SIZE, stdin);
                message[strcspn(message, "\n")] = 0;  // 개행 문자 제거
                break;

            default:
                printf("잘못된 선택입니다. 다시 시도하세요.\n");
                continue;
        }

        write(sock, message, strlen(message));
        str_len = read(sock, message, BUF_SIZE - 1);
        if (str_len == -1) error_handling("read() error");
        message[str_len] = 0;
        printf("Server response: %s\n", message);
    }

    close(sock);
    return 0;
}
