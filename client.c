#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define PORT 23

typedef struct {
    int id;
    char password[20];
    int in_use;
    int lock_time;
    char items[100];
    char code[9];    
} Locker;

void show_menu() {
    printf("\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
    printf("==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==\n");
    printf("1. Locker Information\n");
    printf("\n2. Set Up Locker (Password and Store Items)\n");
    printf("\n3. Access Locker (Verify Password and Retrieve Items)\n");
    printf("\n4. Change Password\n");
    printf("\n5. Request content Hint\n");
    printf("\n6. Empty Locker\n");
    printf("\n7. Exit\n");
    printf("==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==  ==\n");
    printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
}

void displayLockerInfo(Locker locker) {
    if (locker.id >= 1 && locker.id <= 3) {
        printf("Locker %d >> High-Level Locker - %s\n", locker.id, locker.in_use ? "In Use" : "Available");
    } else if (locker.id >= 4 && locker.id <= 6) {
        printf("Locker %d >> Large Locker - %s\n", locker.id, locker.in_use ? "In Use" : "Available");
    } else {
        printf("Locker %d >> Normal Locker - %s\n", locker.id, locker.in_use ? "In Use" : "Available");
    }
}

void locker_info(int client_socket) {
    int choice = 1;
    write(client_socket, &choice, sizeof(choice));
    Locker lockers[10];
    for (int i = 0; i < 10; i++) {
        read(client_socket, &lockers[i], sizeof(Locker));
        displayLockerInfo(lockers[i]);
    }
    sleep(1); 
}

void setup_locker(int client_socket) {
    int choice = 2, locker_id, item_count;
    char password[20];
    char items[100];
    char code[9];
    
    printf("\nEnter locker number (1~10) >> ");
    scanf("%d", &locker_id);
    printf("\nSet your password >> ");
    scanf("%s", password);
    if (locker_id >= 4 && locker_id <= 6) {
        printf("\nEnter the number of items to store >> ");
        scanf("%d", &item_count);
        write(client_socket, &choice, sizeof(choice));
        write(client_socket, &locker_id, sizeof(locker_id));
        write(client_socket, password, sizeof(password));
        write(client_socket, &item_count, sizeof(item_count));
        for (int i = 0; i < item_count; i++) {
            char item[20];
            printf("\nEnter item %d >> ", i+1);
            scanf("%s", item);
            write(client_socket, item, sizeof(item));
        }
    } else {
        printf("\nEnter items to store >> ");
        scanf(" %[^\n]", items);
        write(client_socket, &choice, sizeof(choice));
        write(client_socket, &locker_id, sizeof(locker_id));
        write(client_socket, password, sizeof(password));
        write(client_socket, items, sizeof(items));
    }
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        printf("\nLocker %d successfully set up.\n", locker_id);
        if (locker_id >= 1 && locker_id <= 3) {
            read(client_socket, code, sizeof(code));
            printf("\nYour access code is >> %s\n", code);
        }
    } else {
        printf("\nFailed to set up Locker %d.\n", locker_id);
    }
    sleep(1); 
}

void access_locker(int client_socket) {
    int choice = 3, locker_id;
    char password[20];
    char items[100];
    char code[9] = "";
    
    printf("\nEnter locker number (1-10) >> ");
    scanf("%d", &locker_id);
    printf("\nEnter your password >> ");
    scanf("%s", password);
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, password, sizeof(password));
    
    if (locker_id >= 1 && locker_id <= 3) {
        printf("\nEnter access code >> ");
        scanf("%s", code);
        write(client_socket, code, sizeof(code));
    }
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        read(client_socket, items, sizeof(items));
        printf("\nAccess to Locker %d granted.\n", locker_id);
        printf("\nStored items >> %s\n", items);
    } else if (result == -1) {
        printf("\nIncorrect password or code. Locker %d is locked for 30 seconds.\n", locker_id);
    } else if (result == -2) {
        printf("\nLocker %d is locked. Try again later.\n", locker_id);
    }
    sleep(1); 
}

void change_password(int client_socket) {
    int choice = 4, locker_id;
    char current_password[20], new_password[20], confirm_password[20];
    
    printf("\nEnter locker number >> ");
    scanf("%d", &locker_id);
    printf("\nEnter current password >> ");
    scanf("%s", current_password);
    printf("\nEnter new password >> ");
    scanf("%s", new_password);
    printf("\nConfirm new password >> ");
    scanf("%s", confirm_password);
    
    if (strcmp(new_password, confirm_password) != 0) {
        printf("\nPasswords do not match.\n");
        return;
    }
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, current_password, sizeof(current_password));
    write(client_socket, new_password, sizeof(new_password));
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        printf("\nPassword successfully changed.\n");
    } else {
        printf("\nFailed to change password.\n");
    }
    sleep(1); 
}

void request_hint(int client_socket) {
    int choice = 5, locker_id;
    printf("\nEnter locker number >> ");
    scanf("%d", &locker_id);
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    
    char hint[100];
    read(client_socket, hint, sizeof(hint));
    printf("\nHint for locker %d >> %s\n", locker_id, hint);
    sleep(1); 
}

void empty_locker(int client_socket) {
    int choice = 6, locker_id;
    char password[20];
    char code[9] = "";
    
    printf("\nEnter locker number (1-10) >> ");
    scanf("%d", &locker_id);
    printf("\nEnter your password >> ");
    scanf("%s", password);
    
    write(client_socket, &choice, sizeof(choice));
    write(client_socket, &locker_id, sizeof(locker_id));
    write(client_socket, password, sizeof(password));
    
    if (locker_id >= 1 && locker_id <= 3) {
        printf("\nEnter access code >> ");
        scanf("%s", code);
        write(client_socket, code, sizeof(code));
    }
    
    int result;
    read(client_socket, &result, sizeof(result));
    if (result == 0) {
        printf("\nLocker %d is empty now.\n", locker_id);
    } else if (result == -1) {
        printf("\nIncorrect password or code. Locker %d is locked for 30 seconds.\n", locker_id);
    } else if (result == -2) {
        printf("\nLocker %d is locked. Try again later.\n", locker_id);
    }
    sleep(1); 
}

int main() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_port = htons(PORT);
    
    connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    
    int choice; 
    printf("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n");
    printf("\n* * * * * * * * * * * WELCOME TO LOCKER SERVICE!! * * * * * * * * * * *\n");
    while (1) {
        show_menu();
        printf("\nChoose an option >> ");
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
                request_hint(client_socket);
                break;
            case 6:
                empty_locker(client_socket);
                break;
            case 7:
                close(client_socket);
                exit(0);
            default:
                printf("\nInvalid choice.\n");
        }
    }
    
    return 0;
}