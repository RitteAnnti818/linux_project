#define LOCKER_H
#define LOCKER_H

#define MAX_LOCKERS 9
#define PASSWORD_LENGTH 20

// 사물함 정보를 저장하는 구조체
struct locker {
    int menu;
    int in_use;           // 사물함 사용 중 여부 (0: 사용 가능, 1: 사용 중)
    int locker_number;    // 사물함 번호        
} Locker;

// 사물함 비밀번호를 저장하는 구조체
struct password {
    char password[PASSWORD_LENGTH];  // 사물함 비밀번호
} LockerPassword;
