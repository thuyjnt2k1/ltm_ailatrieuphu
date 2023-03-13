#include <winsock2.h>
#include <pthread.h>
#include "definelink.h"
#include <stdio.h>
#include <time.h>

typedef struct
{
    char account[20];
    char password[10];
    char username[20];
    int status;
    int count;
} User;

typedef struct
{
    char idRoom[5];
    int totalUserRoom; // so nguoi choi hien tai trong phong
    int maxUser;       // toi da so nguoi choi trong phong
    int start;         // so nguoi bat dau an start  // tức là mỗi người thích bắt đầu trước cũng được
    int user[3][2];    // nguoi choi va diem
    char name[100];    // ten nguoi choi
    char account[2][100];
    int currentQs; // cau hoi hien tai
    int playing;   // so nguoi tiep tuc choi
    int userAnswer[3];
    int answered;  // so nguoi da tra loi
    int result[3]; // ket qua dap an(dung,sai)
    int endGame;   // so nguoi da ket thuc tro choi
    int quesList[15];
    int user0End;
    int user1End;
} Room;

typedef struct
{
    int sockfd;
    char buffer[1000];
} Argument;

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENT 100 // Max clients can connect to server
#define MAX_MESSAGE 1024
#define MAX_LINE 2048
int nread;

// local variable
SOCKET sockfd;
struct sockaddr_in server_addr, client_addr;
// Client *client_list;
// Room *room_list;
int thread_count = 0;
int totalUser = 0;
int totalRoom = 0;
int numberQs = 0;

User *user = NULL; // toàn bộ user
Room *room = NULL; // toàn bộ phòng
// Quiz *quiz_arr = NULL;   // toàn bộ câu hỏi
// History *history = NULL; // toàn bộ lịch sử của user

void *sendToClient(void *Arg)
{
    pthread_detach(pthread_self());
    Argument *arg = (Argument *)Arg;
    int sockfd = arg->sockfd;
    char *buffer = arg->buffer;
    // int length = (int)sizeof(client_addr);
    // sendto(sockfd, buffer, strlen(buffer), 0, (SOCKADDR *)&client_addr, length);
    send(sockfd, buffer, strlen(buffer), 0);
}

void listenToClient(SOCKET sockfd, char *buffer)
{
    int buffer_size;
    buffer_size = recv(sockfd, buffer, MAX_MESSAGE, 0);
    buffer[buffer_size] = '\0';
}

User *readFileUser(int *n)
{
    int totalUser = 0;
    User *user;

    /* file reading*/
    user = (User *)calloc(100, sizeof(User));

    FILE *file;
    if ((file = fopen("user.txt", "r")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }

    char s[100] = {0};

    while (!feof(file))
    {
        fscanf(file, "%s %s %s %d", user[totalUser].account, user[totalUser].password, user[totalUser].username, &user[totalUser].status);
        user[totalUser].status = 0;
        user[totalUser].count = 0;
        fgets(s, 100, file);

        totalUser++;
    }

    *n = totalUser;

    return user;
}

void writeFileUser(User *user, int totalUser)
{
    FILE *file;
    if ((file = fopen("user.txt", "w")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }

    for (int i = 0; i < totalUser; i++)
    {
        if (i == totalUser - 1)
            fprintf(file, "%s %s %s %d", user[i].account, user[i].password, user[i].username, user[i].status);
        else
            fprintf(file, "%s %s %s %d\n", user[i].account, user[i].password, user[i].username, user[i].status);
    }

    fclose(file);
}

int checkLogin(User *user, int totalUser, char *account, char *password, char *username)
{
    printf("\n%d %s %s %s", totalUser, user[totalUser].account, user[totalUser].password, user[totalUser].username);
    printf("\n%s %s", account, password);
    for (int i = 0; i < totalUser; i++)
    {
        printf("\n %s %s %s", user[i].account, user[i].password, user[i].username);
        if (strcmp(user[i].account, account) == 0)
        {
            if (user[i].status == 0)
            {
                if (strcmp(user[i].password, password) == 0)
                {
                    user[i].status = 1;
                    strcpy(username, user[i].username);
                    return 1;
                }
                else
                {
                    user[i].count++;
                    if (user[i].count == 3)
                    {
                        user[i].status = 1;
                    }
                    return 2;
                }
            }
            else
                return 3;
        }
    }

    return 0;
}

int checkRegister(User *user, int *totalUser, char *account, char *password, char *username)
{
    int i;
    printf("\n%s %s %s", account, password, username);
    for (i = 0; i < *totalUser; i++)
    {
        if (strcmp(user[i].account, account) == 0)
        {
            return 0;
        }
        else if (strcmp(user[i].username, username) == 0)
        {
            return 0;
        }
    }

    strcpy(user[*totalUser].account, account);
    strcpy(user[*totalUser].password, password);
    strcpy(user[*totalUser].username, username);

    user[*totalUser].status = 0;
    printf("\n%d %s %s %s", i, user[*totalUser].account, user[*totalUser].password, user[*totalUser].username);

    (*totalUser)++;
    writeFileUser(user, *totalUser);

    return 1;
}

int checkRepassword(User *user, int *totalUser, char *username, char *password, char *repassword)
{
    if (strcmp(password, repassword) != 0)
    {
        return 0;
    }
    int i;
    printf("\n%s %s %s", username, password, repassword);
    for (i = 0; i < *totalUser; i++)
    {
        if (strcmp(user[i].username, username) == 0)
        {
            if (strcmp(user[i].password, password) == 0)
                return 2;
            else
                return 1;
        }
    }
}

int getUsername(User *user, int totalUser, char *account, char *username)
{
    for (int i = 0; i < totalUser; i++)
    {
        if (strcmp(user[i].account, account) == 0)
        {
            strcpy(username, user[i].username);
            return 0;
        }
    }

    return 0;
}

void start(Room *room, int totalRoom, char *idRoom, char *userName, int fd)
{
    int count;

    for (int i = 0; i < totalRoom; i++)
    {
        if (strcmp(idRoom, room[i].idRoom) == 0)
        {
            count = i;
            room[i].user[room[i].start][0] = fd;
            room[i].user[room[i].start][1] = 0;
            room[i].start++;
            room[i].totalUserRoom++;
            strcat(room[i].name, userName);
            strcat(room[i].name, ",");
        }
    }

    // khi số lượng người bấm start bằng maxUser thì sẽ gửi 1 gói tin lên cho các user để bắt đầu chơi
    if (room[count].start == room[count].maxUser)
    {
        char buffer[MAX_LINE] = {0};
        char start[10] = {0};

        sprintf(start, "%d", room[count].start);

        strcat(buffer, "oke:");
        strcat(buffer, start);
        strcat(buffer, " ");
        strcat(buffer, room[count].idRoom);
        strcat(buffer, " ");
        strcat(buffer, room[count].name);

        for (int i = 0; i < room[count].start; i++)
        {
            pthread_t id;
            printf("S->C: %s\n", buffer);
            // send(room[count].user[i][0], buffer, strlen(buffer), 0);
            Argument *arg = calloc(1, sizeof(Argument));
            arg->sockfd = room[count].user[i][0];
            strcpy(arg->buffer, buffer);
            pthread_create(&id, NULL, &sendToClient, (void *)arg);
        }
    }
    else
    {

        // nếu số lượng người bấm start không đủ full phòng thì sẽ phải gửi gói tin cho user là chờ
        char buffer[MAX_LINE] = {0};
        char start[10] = {0};

        sprintf(start, "%d", room[count].start);

        strcat(buffer, "wait:");
        strcat(buffer, start);
        strcat(buffer, " ");
        strcat(buffer, room[count].idRoom);
        strcat(buffer, " ");
        strcat(buffer, room[count].name); // dang tra lai ten minh

        for (int i = 0; i < room[count].start; i++)
        {
            pthread_t id;
            printf("S->C: %s\n", buffer);
            Argument *arg = calloc(1, sizeof(Argument));
            arg->sockfd = room[count].user[i][0];
            strcpy(arg->buffer, buffer);
            pthread_create(&id, NULL, &sendToClient, (void *)arg);
            // send(room[count].user[i][0], buffer, strlen(buffer), 0);
        }
    }
}

int logout(User *user, int totalUser, char *username)
{
    for (int i = 0; i < totalUser; i++)
    {
        if (strcmp(user[i].username, username) == 0)
        {
            user[i].status = 0;
            return 1;
        }
    }

    return 0;
}

void handleSendResultOFOther(Room *room, char *idRoom, int fd)
{
    char username[20];
    char buffer[1000];
    char thongbao[1000];
    for (int i = 0; i < totalRoom; i++)
    {
        if (strcmp(idRoom, room[i].idRoom) == 0)
        {
            for (int j = 0; j < 2; j++)
            {
                if (room[i].user[j][0] != fd)
                {
                    char name[10];
                    strcpy(name, room[i].name);
                    char *c = strtok(name, ",");
                    if (j == 0)
                    {
                        if (room[i].user0End == 0)
                        {
                            // client nay chua gui result
                            strcpy(thongbao, "notend");
                        }
                        else
                        {
                            strcpy(thongbao, "oke");
                        }
                        strcpy(username, c);
                    }
                    else
                    {
                        if (room[i].user1End == 0)
                        {
                            strcpy(thongbao, "notend");
                        }
                        else
                        {
                            strcpy(thongbao, "oke");
                        }
                        c = strtok(NULL, ",");
                        strcpy(username, c);
                    }
                    sprintf(buffer, "%s:%s %d", thongbao, username, room[i].user[j][1]);
                    send(fd, buffer, strlen(buffer), 0);
                }
            }
        }
    }
}

void freeData()
{
    free(user);
    free(room);
    // free(quiz_arr);
    // free(history);
}

Room *readFileRoom(int *n)
{
    Room *room;
    int totalRoom = 0;

    room = (Room *)calloc(100, sizeof(Room));
    FILE *file;

    if ((file = fopen("room.txt", "r")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }

    char s[100] = {0};

    while (!feof(file))
    {
        fscanf(file, "%s %d", room[totalRoom].idRoom, &room[totalRoom].maxUser);
        room[totalRoom].totalUserRoom = 0;
        room[totalRoom].start = 0;
        room[totalRoom].currentQs = 0;
        room[totalRoom].answered = 0;
        room[totalRoom].playing = room[totalRoom].maxUser;
        room[totalRoom].endGame = 0;
        fgets(s, 100, file);
        room[totalRoom].user0End = 0;
        room[totalRoom].user1End = 0;
        totalRoom++;
    }

    *n = totalRoom;

    return room;
}

int checkFullRoom(Room *room, int totalRoom, char *idRoom)
{
    for (int i = 0; i < totalRoom; i++)
    {
        if (strcmp(idRoom, room[i].idRoom) == 0)
        {
            if (room[i].start == room[i].maxUser)
            {
                return 1;
            }
        }
    }
    return 0;
}

void sendIdRoom(Room *room, int totalRoom, int fd)
{
    char buffer[MAX_LINE] = {0};

    for (int i = 0; i < totalRoom; i++)
    {
        char c[12];
        strcat(buffer, room[i].idRoom);
        strcat(buffer, " ");
        ZeroMemory(c, 12);
        sprintf(c, "%d", room[i].start);
        strcat(buffer, c);
        if (i != totalRoom - 1)
            strcat(buffer, " ");
        // strcat(buffer, "a|");
    }

    printf("S->C: room:%s\n", buffer);

    send(fd, buffer, strlen(buffer), 0);
}

void createRoom(Room *room, char *userName, int fd, char *idRoom)
{
    totalRoom--;
    // printf("lastIdRoom %s\n", room[totalRoom].idRoom);
    int lastIdRoom = atoi(room[totalRoom].idRoom);
    // printf("lastiDrOOM %d\n", lastIdRoom);
    totalRoom++;
    sprintf(idRoom, "%d", lastIdRoom + 1);
    // printf("%s idRoom\n", idRoom);
    strcpy(room[totalRoom].idRoom, idRoom);
    room[totalRoom].totalUserRoom = 0;
    room[totalRoom].start = 0;
    room[totalRoom].currentQs = 0;
    room[totalRoom].answered = 0;
    room[totalRoom].playing = 2;
    room[totalRoom].endGame = 0;
    room[totalRoom].maxUser = 2;
    room[totalRoom].user0End = 0;
    room[totalRoom].user1End = 0;
    totalRoom++;
}

void handleResultRoom(Room *room, char *idRoom, int fd, int numberResult)
{
    for (int i = 0; i < totalRoom; i++)
    {
        if (strcmp(idRoom, room[i].idRoom) == 0)
        {
            for (int j = 0; j < 2; j++)
            {
                if (room[i].user[j][0] == fd)
                {
                    room[i].user[j][1] = numberResult;
                    if (j == 0)
                        room[i].user0End = 1;
                    if (j == 1)
                        room[i].user1End = 1;
                }
            }
        }
    }
}

void handleOutRoom(Room *room, char *idRoom, int fd)
{
    for (int i = 0; i < totalRoom; i++)
    {
        if (strcmp(idRoom, room[i].idRoom) == 0)
        {
            room[i].totalUserRoom--;
            if (room[i].totalUserRoom == 0)
            {
                // printf("start = 0\n");
                room[i].start = 0;
                ZeroMemory(room[i].name, 100);
            }
            // printf("start not 0\n");
        }
    }
}

void *handleDataFromClient(void *arg)
{
    int fd = *(int *)arg;
    pthread_detach(pthread_self());
    char buffer[MAX_LINE] = {0};

    while (1)
    {

        ZeroMemory(buffer, MAX_LINE);
        nread = recv(fd, buffer, 1000, 0);
        if (nread == 0)
            break;
        buffer[nread] = 0;
        // printf("%s\n", buffer);

        // ngắt lấy vùng text đầu tiên ngăn bởi dấu : của buffer
        char *c;
        c = strtok(buffer, ":");
        int i;

        if (strcmp(c, "getResult") == 0)
        {
            char idRoom[10];
            c = strtok(NULL, " ");
            printf("c %s\n", c);
            strcpy(idRoom, c);
            handleSendResultOFOther(room, idRoom, fd);
            // send(fd, "oke:a 6", strlen("oke:a 6"), 0);
        }
        // trong trường hợp gói tin nhận được là "login"
        else if (strcmp(c, "login") == 0)
        {
            char account[100] = {0};
            char password[100] = {0};
            char username[100] = {0};
            char mess[MAX_LINE] = {0};

            // lấy ra account vs password
            char *c;
            c = strtok(NULL, " ");
            strcpy(account, c);
            c = strtok(NULL, " ");
            strcpy(password, c);
            ZeroMemory(mess, MAX_LINE);

            // check ca user và password
            switch (checkLogin(user, totalUser, account, password, username))
            {
            case 0:
                strcat(mess, "login:0");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0); // gửi kết quả đăng nhập cho user
                break;
            case 1:
                // đăng nhập thất bại sẽ gửi đi thông điệp
                strcat(mess, "login:1");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
                strcpy(mess, username);
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
                break;
            case 2:
                strcat(mess, "login:2");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
                break;
            case 3:
                strcat(mess, "login:3");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
                break;
            default:
                break;
            }
            writeFileUser(user, totalUser);
        }
        else if (strcmp(c, "createRoom") == 0)
        {
            c = strtok(NULL, " ");
            char idRoom[1000];
            createRoom(room, c, fd, idRoom);
            ZeroMemory(buffer, 1000);
            // printf("idroom %s\n", idRoom);
            sprintf(buffer, "createOk:%s", idRoom);
            send(fd, buffer, strlen(buffer), 0);
        }

        // trong trường hợp gói tin nhận được là "login"
        else if (strcmp(c, "room") == 0)
        {
            // nread = recv(fd, buffer, MAX_LINE, 0);
            sendIdRoom(room, totalRoom, fd);
        }
        else if (strcmp(c, "logout") == 0)
        {

            // trong trường hợp gói tin nhận được là "logout"
            char mess[MAX_LINE] = {0};
            char username[20] = {0};

            // không hiểu 2 lệnh này lắm
            c = strtok(NULL, " ");
            strcpy(username, c);

            if (logout(user, totalUser, username) == 1)
            {
                // đăng xuất thành công thì gửi thông điệp cho user
                strcpy(mess, "logout:1");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            else
            {
                // đăng nhập thất bại
                strcpy(mess, "logout:0");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            writeFileUser(user, totalUser);
        }
        else if (strcmp(c, "register") == 0)
        {
            // nếu gói tin nhận được là "register"
            char mess[MAX_LINE] = {0};
            char account[10] = {0};
            char password[10] = {0};
            char username[10] = {0};

            // lấy username vs password
            c = strtok(NULL, " ");
            strcpy(account, c);
            c = strtok(NULL, " ");
            strcpy(password, c);
            c = strtok(NULL, " ");
            strcpy(username, c);
            if (checkRegister(user, &totalUser, account, password, username) == 1)
            {
                // đăng kí thành công và gửi thông điệp
                // printf("\n%d %s %s %s", totalUser, user[totalUser].account, user[totalUser].password, user[totalUser].username);

                strcpy(mess, "register:1");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            else
            {
                // đăng kí thất bại thì gửi lại thông điệp
                strcpy(mess, "register:0");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            writeFileUser(user, totalUser);
        }
        else if (strcmp(c, "repassword") == 0)
        {
            // nếu gói tin nhận được là "register"
            char mess[MAX_LINE] = {0};
            char account[10] = {0};
            char password[10] = {0};
            char repassword[10] = {0};

            // lấy username vs password
            c = strtok(NULL, " ");
            strcpy(account, c);
            c = strtok(NULL, " ");
            strcpy(password, c);
            c = strtok(NULL, " ");
            strcpy(repassword, c);
            if (checkRepassword(user, &totalUser, account, password, repassword) == 1)
            {
                // đăng kí thành công và gửi thông điệp

                strcpy(mess, "repassword:1");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            else if (checkRepassword(user, &totalUser, account, password, repassword) == 2)
            {
                // đăng kí thành công và gửi thông điệp

                strcpy(mess, "repassword:2");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            else
            {
                // đăng kí thất bại thì gửi lại thông điệp
                strcpy(mess, "repassword:0");
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            writeFileUser(user, totalUser);
        }
        else if (strcmp(c, "getquestion") == 0)
        {
            int option = 0, easysize = 0, normalsize = 0, hardsize = 0, isOnline = 0;
            // nếu gói tin nhận được là "getquestion"
            srand(time(0));
            char idRoom[20];
            char mess[MAX_LINE] = {0};
            c = strtok(NULL, " ");
            isOnline = atoi(c);

            c = strtok(NULL, " ");
            easysize = atoi(c);

            c = strtok(NULL, " ");
            normalsize = atoi(c);

            c = strtok(NULL, " ");
            hardsize = atoi(c);

            if (isOnline == 0)
            {
                c = strtok(NULL, " ");
                option = atoi(c);
                int count = 0;
                i = 0;
                int y = 0, num = 0, shown = 0;
                int *quesList = (int *)malloc(sizeof(int) * 15);
                switch (option)
                {
                case 2:
                    count = 0;
                    while (1)
                    {
                        if (count == 15)
                            break;
                        shown = 0;
                        if (count <= 9)
                            num = (rand() % (easysize - 0 + 1)) + 0;
                        else if (count <= 14)
                            num = (rand() % (normalsize - easysize + 1)) + easysize;
                        for (y = 0; y <= count; y++)
                        {
                            if (quesList[y] == num)
                            {
                                shown = 1;
                                break;
                            }
                        }
                        if (shown == 0)
                        {
                            quesList[count] = num;
                            // printf("%d ", num);

                            count++;
                        }
                    }
                    break;
                case 1:
                    count = 0;
                    while (1)
                    {
                        if (count == 15)
                            break;
                        shown = 0;
                        if (count <= 4)
                            num = (rand() % (easysize - 0 + 1)) + 0;
                        else if (count <= 9)
                            num = (rand() % (normalsize - easysize + 1)) + easysize;
                        else if (count <= 14)
                            num = (rand() % (hardsize - normalsize + 1)) + normalsize;
                        for (y = 0; y <= count; y++)
                        {
                            if (quesList[y] == num)
                            {
                                shown = 1;
                                break;
                            }
                        }
                        if (shown == 0)
                        {
                            quesList[count] = num;
                            // printf("%d ", num);
                            count++;
                        }
                    }
                    break;
                case 0:
                    count = 0;
                    while (1)
                    {
                        if (count == 15)
                            break;
                        shown = 0;
                        if (count <= 5)
                            num = (rand() % (normalsize - easysize + 1)) + easysize;
                        else if (count <= 14)
                            num = (rand() % (hardsize - normalsize + 1)) + normalsize;
                        for (y = 0; y <= count; y++)
                        {
                            if (quesList[y] == num)
                            {
                                shown = 1;
                                break;
                            }
                        }
                        if (shown == 0)
                        {
                            quesList[count] = num;
                            // printf("%d ", num);
                            count++;
                        }
                    }
                    break;
                }
                char str[12];
                strcat(mess, "questlist:");
                for (i = 0; i < 14; i++)
                {
                    sprintf(str, "%d ", quesList[i]);
                    strcat(mess, str);
                }
                sprintf(str, "%d", quesList[14]);
                strcat(mess, str);
                printf("S->C: %s\n", mess);
                send(fd, mess, strlen(mess), 0);
            }
            else if (isOnline == 1)
            {
                char str[12];
                c = strtok(NULL, " ");
                strcpy(idRoom, c);
                for (int i = 0; i < totalRoom; i++)
                {
                    if (strcmp(idRoom, room[i].idRoom) == 0)
                    {
                        // printf("\n%d", room[i].start);
                        if (room[i].start == 2)
                        {
                            for (int j = 0; j < 14; j++)
                            {
                                sprintf(str, "%d ", room[i].quesList[j]);
                                strcat(mess, str);
                            }
                            sprintf(str, "%d", room[i].quesList[14]);
                            strcat(mess, str);
                            printf("S->C: %s\n", mess);
                            send(fd, mess, strlen(mess), 0);
                        }
                        else
                        {
                            int count = 0;
                            int j = 0;
                            int y = 0, num = 0, shown = 0;
                            int *quesList = (int *)malloc(sizeof(int) * 15);
                            while (1)
                            {
                                if (count == 15)
                                    break;
                                shown = 0;
                                if (count <= 4)
                                    num = (rand() % (easysize - 0 + 1)) + 0;
                                else if (count <= 9)
                                    num = (rand() % (normalsize - easysize + 1)) + easysize;
                                else if (count <= 14)
                                    num = (rand() % (hardsize - normalsize + 1)) + normalsize;
                                for (y = 0; y <= count; y++)
                                {
                                    if (room[i].quesList[y] == num)
                                    {
                                        shown = 1;
                                        break;
                                    }
                                }
                                if (shown == 0)
                                {
                                    room[i].quesList[count] = num;
                                    // printf("\n%d %d", room[i].quesList[count], num);

                                    count++;
                                }
                            }
                            for (int j = 0; j < 14; j++)
                            {
                                sprintf(str, "%d ", room[i].quesList[j]);
                                strcat(mess, str);
                            }
                            sprintf(str, "%d", room[i].quesList[14]);
                            strcat(mess, str);
                            printf("S->C: %s\n", mess);
                            send(fd, mess, strlen(mess), 0);
                        }
                    }
                }
            }
        }

        else if (strcmp(c, "start") == 0)
        {
            // nếu gói tin nhận được là "start"
            char idRoom[5] = {0};
            char userName[20] = {0};

            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            c = strtok(NULL, " ");
            strcpy(userName, c);

            // sẵn sàng chơi cho user đó và gửi thông điệp chờ nếu chưa đủ ....
            if (checkFullRoom(room, totalRoom, idRoom))
            {
                send(fd, "full:full", strlen("full:full"), 0);
            }
            else
            {
                start(room, totalRoom, idRoom, userName, fd);
            }
        }
        else if (strcmp(c, "result") == 0)
        {
            char idRoom[5] = {0};
            char userName[20] = {0};
            int result = 0;
            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            c = strtok(NULL, " ");
            result = atoi(c);
            // printf("\n%s %s %d", idRoom, userName, result);
            handleResultRoom(room, idRoom, fd, result);
        }
        if (strcmp(c, "getResult") == 0)
        {
            char idRoom[10];
            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            handleSendResultOFOther(room, idRoom, fd);
        }
        else if (strcmp(c, "outRoom") == 0)
        {
            char idRoom[100];
            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            handleOutRoom(room, idRoom, fd);
        }
    }
}

int setupServer()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        return ERROR_RETURN;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d\n", WSAGetLastError());
        return ERROR_RETURN;
    }

    printf("Socket created. \n");

    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return ERROR_RETURN;
    }
    listen(sockfd, 3);
    return SUCCEED_RETURN;
}

void closeSocket()
{
    closesocket(sockfd);
    WSACleanup();
}

int main()
{
    user = readFileUser(&totalUser);
    room = readFileRoom(&totalRoom);
    pthread_t tid;
    // char buffer[MAX_MESSAGE];
    if (setupServer() == SUCCEED_RETURN)
    {
        printf("Server is online\n");
        int c = sizeof(struct sockaddr_in);
        int new_socket;
        while ((new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &c)) != INVALID_SOCKET)
        {
            printf("Connection accepted: %d\n", new_socket);
            // Reply to the client
            //
            int *arg = &new_socket;
            pthread_create(&tid, NULL, &handleDataFromClient, (void *)arg);
            // handleDataFromClient((void *)arg);
        }
        if (new_socket == INVALID_SOCKET)
        {
            printf("accept failed with error code : %d", WSAGetLastError());
        }
    }
    writeFileUser(user, totalUser);

    closeSocket();
    return 0;
}