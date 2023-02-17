#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <pthread.h>
#include <signal.h>

#include <winsock2.h>
#include <pthread.h>
#include "definelink.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CLIENT 100 // Max clients can connect to server
#define MAX_MESSAGE 1024
#define MAX_LINE 1024
#define ESC -1
#define ERROR 0
#define SUCCEED 1

typedef struct
{
    char question[200];
    char optionA[200];
    char optionB[200];
    char optionC[200];
    char optionD[200];
    char answers[10];
} question;

int hard_ques_size = 0;
int normal_ques_size = 0;
int easy_ques_size = 0;

question *quiz_arr = NULL;

int questList[15];

typedef struct GameTexture
{
    SDL_Texture *texture;
    int tWidth;
    int tHeight;
} GameTexture;

SDL_Window *window = NULL;
SDL_Window *window2 = NULL;

SDL_Renderer *renderer = NULL;
SDL_Renderer *renderer2 = NULL;

const int SCREEN_WIDTH = 1369;
const int SCREEN_HEIGHT = 770;
char *player_name;
char *player_password;
char *player_repassword;
char *thongbao;
int suport1check = 0;
int suport2check = 0;
int suport3check = 0;

char **MessageLine;
int current_line = 0;
SDL_Rect inputMessage_rect;
SDL_Rect chatBox_rect;
SDL_Rect send_Button;
SDL_Rect showMessage_rect;
GameTexture *sendButton;
SDL_Rect *messLine_rect;
// The music that will be played
Mix_Music *gMusic = NULL;
Mix_Chunk *gStartGame = NULL;
Mix_Chunk *gRightAnswer = NULL;
Mix_Chunk *gWrongAnswer = NULL;
Mix_Music *gPlaying = NULL;

SDL_Color white_color = {255, 255, 255};
SDL_Color black_color = {0, 0, 0};
SDL_Color red_color = {255, 0, 0};
SDL_Color yellow_color = {255, 255, 0};
SDL_Color green_color = {0, 255, 0};
SDL_Color blue_color = {0, 0, 139};

// for socket streaming
#define MAX_BUFF 200

SOCKET sockfd;
struct sockaddr_in server_addr, client_addr;
// Client *client_list;
// Room *room_list;
int thread_count = 0;
int sock;
int nread;
char buffer[MAX_BUFF];
char username[MAX_BUFF];
int flag = 0;
int totalRoom = 0;
char idRoom[1000];
int isLogin = 0;
int timer = 0;
int isOnline = 0;

SOCKET setupSocket()
{
    WSADATA wsa;
    // SOCKET sockfd;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d\n", WSAGetLastError());
        exit(0);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d\n", WSAGetLastError());
        exit(0);
    }

    return sockfd;

    // if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    // {
    //     puts("connect error");
    //     return 0;
    // }
    // return SUCCEED_RETURN;
}

struct sockaddr_in setupServerAddr()
{
    struct sockaddr_in server_addr;
    ZeroMemory(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    return server_addr;
}

typedef struct
{
    int sockfd;
    int result;
    char buffer[1000];
} Argument;

void closeClient()
{
    closesocket(sockfd);
    WSACleanup();
}

void sendToServer(SOCKET sockfd, char *buffer)
{
    // sendto(sockfd, buffer, strlen(buffer), 0, (SOCKADDR *)&client_addr, length);
    send(sockfd, buffer, strlen(buffer), 0);
}

void listenToServer(SOCKET sockfd, char *buffer)
{
    int buffer_size;
    buffer_size = recv(sockfd, buffer, MAX_MESSAGE, 0);
    buffer[buffer_size] = '\0';
}

int check_mouse_pos(SDL_Rect rect)
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    int value = 1;
    if (x < rect.x)
    {
        value = -1;
    }
    else if (x > rect.x + rect.w)
    {
        value = -1;
    }
    else if (y < rect.y)
    {
        value = -1;
    }
    else if (y > rect.y + rect.h)
    {
        value = -1;
    }
    return value;
}

SDL_Rect CreateRect(int x, int y, int w, int h)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.h = h;
    rect.w = w;
    return rect;
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

int CreateWindowGame()
{ // Create window
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("SDL could not initialize! SDL_Error %s\n", SDL_GetError());
        return ERROR;
    }
    else
    {
        window = SDL_CreateWindow("WereWolf Deluxo Edition", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (window == NULL)
        {
            printf("Window could not initialize! SDL_Error: %s\n", SDL_GetError());
            return ERROR;
        }
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (renderer == NULL)
        {
            printf("Cant create render! SDL_Error: %s\n", SDL_GetError());
            return ERROR;
        }

        int imgFlags = IMG_INIT_PNG;
        if (!(IMG_Init(imgFlags) & imgFlags))
        {
            printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
            return ERROR;
        }
        if (TTF_Init() == -1)
        {
            printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
            return ERROR;
        }
        if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        {
            printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
            return ERROR;
        }
        gMusic = (Mix_Music *)Mix_LoadMUS("resource/waitingforyou.mp3");
        if (gMusic == NULL)
        {
            printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
        }
        // Mix_PlayMusic(gMusic, -1);

        gStartGame = Mix_LoadWAV("resource/start_game.mp3");
        if (gStartGame == NULL)
        {
            printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        }
        gWrongAnswer = Mix_LoadWAV("resource/wrong_answer.mp3");
        if (gWrongAnswer == NULL)
        {
            printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        }
        gRightAnswer = Mix_LoadWAV("resource/right_answer.mp3");
        if (gRightAnswer == NULL)
        {
            printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        }
        gPlaying = (Mix_Music *)Mix_LoadWAV("resource/playing.mp3");
        if (gPlaying == NULL)
        {
            printf("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
        }
    }
    return SUCCEED;
}

void load_Texture_Text(const char *text, GameTexture *current, TTF_Font *font, SDL_Color textColor)
{ // Load text texture
    SDL_Surface *screen = TTF_RenderText_Solid(font, text, textColor);
    if (screen == NULL)
    {
        printf("Cant create surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->texture = SDL_CreateTextureFromSurface(renderer, screen);
    if (current->texture == NULL)
    {
        printf("Cant render from surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->tWidth = screen->w;
    current->tHeight = screen->h;
    SDL_FreeSurface(screen);
}

void load_Texture_IMG(char *path, GameTexture *current)
{ // Load image texture
    SDL_Surface *screen = NULL;
    screen = IMG_Load(path);
    if (screen == NULL)
    {
        printf("Cant create surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }
    current->texture = SDL_CreateTextureFromSurface(renderer, screen);
    SDL_FreeSurface(screen);
}

void load_and_Render_Texture_Text_Wrapped(const char *text, GameTexture *current, TTF_Font *font, SDL_Color textColor, int y, int width)
{
    SDL_Surface *screen = TTF_RenderText_Blended_Wrapped(font, text, textColor, width);
    if (screen == NULL)
    {
        printf("Cant create surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }

    current->texture = SDL_CreateTextureFromSurface(renderer, screen);
    if (current->texture == NULL)
    {
        printf("Cant render from surface! SDL_Error: %s\n", SDL_GetError());
        exit(ERROR);
    }

    int width2;
    int height2;
    TTF_SizeText(font, text, &width2, &height2);
    current->tWidth = screen->w;
    current->tHeight = screen->h;

    if (current->tWidth >= width)
    {
        // current->tHeight = 2 * current->tHeight;
        y = y - 20;
    }

    SDL_Rect rect = {(SCREEN_WIDTH - width) / 2, y, current->tWidth, current->tHeight};
    // printf("\n%d %d %d %d", (SCREEN_WIDTH - width) / 2, y, current->tWidth, current->tHeight);
    SDL_RenderCopy(renderer, current->texture, NULL, &rect);
    SDL_RenderPresent(renderer); // display
    SDL_DestroyTexture(current->texture);
    SDL_FreeSurface(screen);
}

void destroy_Texture(GameTexture *current)
{ // free texture
    SDL_DestroyTexture(current->texture);
    current->tHeight = 0;
    current->tWidth = 0;
    free(current);
}

void ResetRender()
{ // reset window to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void ClearRenderRect(SDL_Rect rect)
{ // fill color (set render color before use it) inside rect
    SDL_Rect tmp = rect;
    tmp = CreateRect(tmp.x + 1, tmp.y + 1, tmp.w - 2, tmp.h - 2);
    SDL_RenderFillRect(renderer, &tmp);
}

SDL_Rect Render(GameTexture *current, int width, int height)
{ // Render texture and display at position [x,y]
    SDL_Rect rect = CreateRect(width, height, current->tWidth, current->tHeight);
    SDL_RenderCopy(renderer, current->texture, NULL, &rect);
    SDL_DestroyTexture(current->texture);
    return rect;
}

SDL_Rect Render_Center(GameTexture *current, int height)
{
    SDL_Rect rect = {(SCREEN_WIDTH - current->tWidth) / 2, height, current->tWidth, current->tHeight};
    SDL_RenderCopy(renderer, current->texture, NULL, &rect);
    SDL_RenderPresent(renderer); // display
    SDL_DestroyTexture(current->texture);
    return rect;
}

SDL_Rect Render_Center_box(GameTexture *current, int x, int height, int widthbox)
{
    SDL_Rect rect = {(widthbox - current->tWidth) / 2 + x, height, current->tWidth, current->tHeight};
    SDL_RenderCopy(renderer, current->texture, NULL, &rect);
    SDL_RenderPresent(renderer); // display
    SDL_DestroyTexture(current->texture);
    return rect;
}

void close_win()
{ // Close the game

    if (isLogin)
    {
        ZeroMemory(buffer, MAX_BUFF);
        strcat(buffer, "logout:");
        strcat(buffer, username);
        send(sockfd, buffer, strlen(buffer), 0);

        ZeroMemory(buffer, MAX_BUFF);
        nread = recv(sockfd, buffer, 1000, 0);
    }
    int iResult = shutdown(sockfd, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return;
    }
    // closeClient();
    // Mix_FreeMusic(gMusic);
    // gMusic = NULL;
    // Mix_FreeMusic(gNhacnen);
    // Mix_Quit();
    // Mix_HaltMusic();
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
}

void readFileQs()
{
    question *question_arr;

    int numberQs = 0;

    FILE *file;

    char s[100] = {0};
    char *token;
    char s1[10] = {0};

    // doc file de
    question_arr = (question *)calloc(200, sizeof(question));

    if ((file = fopen("question_eassy.txt", "r")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    numberQs = 0;
    while (!feof(file))
    {
        fgets(s, 200, file);
        token = strtok(s, ":");
        token = strtok(NULL, ":");
        token[strlen(token) - 1] = '\0';
        strcat(question_arr[numberQs].question, token);
        // strcpy(quiz_arr[numberQs].question, token);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionA, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionB, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionC, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionD, s);
        fscanf(file, "%s", s1);
        strcat(question_arr[numberQs].answers, s1);
        fgets(s, 200, file);
        numberQs++;
    }
    easy_ques_size = numberQs;
    fclose(file);
    // ket thuc doc file de

    // doc file trung binh

    if ((file = fopen("question_normal.txt", "r")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    while (!feof(file))
    {
        fgets(s, 200, file);
        token = strtok(s, ":");
        token = strtok(NULL, ":");
        token[strlen(token) - 1] = '\0';
        strcat(question_arr[numberQs].question, token);
        // strcpy(quiz_arr[numberQs].question, token);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionA, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionB, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionC, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionD, s);
        fscanf(file, "%s", s1);
        strcat(question_arr[numberQs].answers, s1);
        fgets(s, 200, file);
        numberQs++;
    }
    normal_ques_size = numberQs;
    fclose(file);
    // ket thuc doc file trung binh

    // doc file kho
    if ((file = fopen("question_hard.txt", "r")) == NULL)
    {
        printf("Error! File cannot be opened.");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }
    while (!feof(file))
    {
        fgets(s, 200, file);
        token = strtok(s, ":");
        token = strtok(NULL, ":");
        token[strlen(token) - 1] = '\0';
        strcat(question_arr[numberQs].question, token);
        // strcpy(quiz_arr[numberQs].question, token);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionA, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionB, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionC, s);
        fgets(s, 200, file);
        s[strlen(s) - 1] = '\0';
        strcat(question_arr[numberQs].optionD, s);
        fscanf(file, "%s", s1);
        strcat(question_arr[numberQs].answers, s1);
        fgets(s, 200, file);
        numberQs++;
    }
    hard_ques_size = numberQs;
    quiz_arr = question_arr;
    // printf("\n%s %s %s", quiz_arr[numberQs].question, quiz_arr[numberQs].optionA, quiz_arr[numberQs].answers);

    // free(question_arr);
    fclose(file);
    // ket thuc doc file kho
}

void getQuesList(int option)
{
    // option 0-easy 1-normal 2-hard
    ZeroMemory(buffer, MAX_BUFF);
    if (isOnline == 1)
    {
        // snprintf(buffer, MAX_BUFF, "getquestion:%s %d %d %d", idRoom, easy_ques_size, normal_ques_size, hard_ques_size);
        snprintf(buffer, MAX_BUFF, "getquestion:1 %d %d %d %s", easy_ques_size, normal_ques_size, hard_ques_size, idRoom);
    }
    else
    {
        snprintf(buffer, MAX_BUFF, "getquestion:0 %d %d %d %d", easy_ques_size, normal_ques_size, hard_ques_size, option);
    }
    send(sockfd, buffer, strlen(buffer), 0);
    ZeroMemory(buffer, MAX_BUFF);
    nread = recv(sockfd, buffer, 100, 0);
    buffer[nread] = 0;
    // printf("\n%s", buffer);
    char *token;
    token = strtok(buffer, " ");
    questList[0] = atoi(token);

    for (int i = 1; i < 15; i++)
    {
        token = strtok(NULL, " ");
        questList[i] = atoi(token);
        // printf("\n%d", questList[i]);
    }
}

char *getPrize(int count)
{
    switch (count - 1)
    {
    case -1:
        return "0";
        break;
    case 0:
        return "200,000";
        break;
    case 1:
        return "400,000";
        break;
    case 2:
        return "600,000";
        break;
    case 3:
        return "1,000,000";
        break;
    case 4:
        return "2,000,000";
        break;
    case 5:
        return "3,000,000";
        break;
    case 6:
        return "6,000,000";
        break;
    case 7:
        return "10,000,000";
        break;
    case 8:
        return "14,000,000";
        break;
    case 9:
        return "22,000,000";
        break;
    case 10:
        return "30,000,000";
        break;
    case 11:
        return "40,000,000";
        break;
    case 12:
        return "60,000,000";
        break;
    case 13:
        return "85,000,000";
        break;
    case 14:
        return "150,000,000";
        break;
    }
}

void Room_screen();
void Offline_screen();
void repassword_screen();
void menu_Screen();
void startPlayOffline(int option);
void result_online_screen();

void ingame_menu()
{
    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);

    ResetRender();
    int i;
    int Menu_item = 3;
    SDL_Rect button[3];

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    char *menu[] = {"ONLINE", "OFFLINE", "DOI MAT KHAU"};
    int select[] = {0, 0, 0};
    SDL_Event menu_e;

    load_Texture_IMG("./resource/menu.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText2[i], font, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText2[i], font, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText2[i], font, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (i == 0)
                        {
                            free(back_texture);
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            isOnline = 1;
                            Room_screen();
                        }
                        else if (i == 1)
                        {
                            free(back_texture);
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            isOnline = 0;
                            Offline_screen();
                        }
                        else if (i == 2)
                        {
                            free(back_texture);
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            repassword_screen();
                        }
                    }
                    if (check_mouse_pos(back_rect) == 1)
                    {
                        if (isLogin)
                        {
                            ZeroMemory(buffer, MAX_BUFF);
                            strcat(buffer, "logout:");
                            strcat(buffer, username);
                            send(sockfd, buffer, strlen(buffer), 0);

                            ZeroMemory(buffer, MAX_BUFF);
                            nread = recv(sockfd, buffer, 1000, 0);
                        }
                        free(back_texture);
                        for (i = 0; i < Menu_item; i++)
                        {
                            free(MenuText2[i]);
                        }
                        free(MenuText2);
                        free(background);
                        menu_Screen();
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText2[i]->texture);
        MenuText2[i]->tHeight = 0;
        MenuText2[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
}

void Login_screen()
{
    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);
    // khoi tao
    int i;
    int choosing = 0;
    SDL_Rect player_name_rect;
    SDL_Rect player_password_rect;
    SDL_Rect thongbao_rect;
    int Menu_item = 1;
    SDL_Rect button[1];

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));
    // load font menu
    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font2 = TTF_OpenFont("resource/arial.ttf", 42);
    char *menu[] = {"DANG NHAP"};
    int select[] = {0, 0, 0};
    SDL_Event menu_e;
    // load image background
    load_Texture_IMG("resource/login.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    // render nut
    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }

    // load ten player
    player_name_rect = CreateRect(435, 488, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_name_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_name_rect);
    player_name = calloc(20, sizeof(char));
    strcpy(player_name, " ");
    // render ten player
    GameTexture *player_name_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_name, player_name_texture, font2, white_color);
    Render(player_name_texture, player_name_rect.x, player_name_rect.y);

    // load password player
    player_password_rect = CreateRect(435, 588, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_password_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_password_rect);
    player_password = calloc(20, sizeof(char));
    strcpy(player_password, " ");
    // render password player
    GameTexture *player_password_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_password, player_password_texture, font2, white_color);
    Render(player_password_texture, player_password_rect.x, player_password_rect.y);

    // load thongbao
    thongbao_rect = CreateRect(200, 390, 1000, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &thongbao_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(thongbao_rect);
    thongbao = calloc(50, sizeof(char));
    strcpy(thongbao, "Vui long nhap thong tin");
    GameTexture *thongbao_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(thongbao, thongbao_texture, font2, blue_color);
    Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
    SDL_RenderPresent(renderer);

    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText2[i], font2, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN: // khi chuot click
                if (check_mouse_pos(player_name_rect) == 1)
                {
                    choosing = 1;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_password_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, white_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(player_password_rect) == 1)
                {
                    choosing = 2;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, white_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                if (check_mouse_pos(back_rect) == 1)
                {
                    for (i = 0; i < Menu_item; i++)
                    {
                        free(MenuText2[i]);
                    }
                    free(MenuText2);
                    free(background);
                    free(player_name_texture);
                    free(player_password_texture);
                    free(thongbao_texture);
                    menu_Screen();
                }
                if (check_mouse_pos(button[0]) == 1)
                {
                    if (strcmp(player_name, " ") == 0 || strcmp(player_password, " ") == 0)
                    {
                        strcpy(thongbao, "Vui long nhap day du thong tin!");
                        ClearRenderRect(thongbao_rect);
                        load_Texture_Text(thongbao, thongbao_texture, font2, blue_color);
                        Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                        SDL_RenderPresent(renderer);
                    }
                    else
                    {
                        // Playing_screen();
                        ZeroMemory(buffer, MAX_BUFF);
                        strcat(buffer, "login:");
                        player_name = strtok(player_name, " ");
                        strcat(buffer, player_name);
                        strcat(buffer, " ");
                        player_password = strtok(player_password, " ");
                        strcat(buffer, player_password);
                        send(sockfd, buffer, strlen(buffer), 0);
                        ZeroMemory(buffer, MAX_BUFF);
                        nread = recv(sockfd, buffer, 100, 0);
                        if (strcmp(buffer, "login:0") == 0)
                        {
                            strcpy(thongbao, "Tai khoan khong ton tai");
                        }
                        else if (strcmp(buffer, "login:1") == 0)
                        {
                            ZeroMemory(buffer, MAX_BUFF);
                            ZeroMemory(username, MAX_BUFF);
                            nread = recv(sockfd, buffer, 100, 0);
                            // printf("\n%d %s", nread, buffer);
                            strcpy(username, buffer);
                            strcpy(thongbao, "Dang nhap thanh cong");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, green_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                            SDL_Delay(3000);
                            isLogin = 1;
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            free(player_name_texture);
                            free(player_password_texture);
                            free(thongbao_texture);

                            ingame_menu();
                        }
                        else if (strcmp(buffer, "login:2") == 0)
                        {
                            strcpy(thongbao, "Mat khau khong chinh xac");
                        }
                        else if (strcmp(buffer, "login:3") == 0)
                        {
                            strcpy(thongbao, "Tai khoan da bi khoa");
                        }
                        // reset thongbao
                        ClearRenderRect(thongbao_rect);
                        load_Texture_Text(thongbao, thongbao_texture, font2, red_color);
                        Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                        SDL_RenderPresent(renderer);
                    }
                }
                break;

            case SDL_TEXTINPUT: // khi co input tu ban phim
                if (strlen(player_name) < 30 && choosing == 1)
                {
                    strcat(player_name, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_password) < 30 && choosing == 2)
                {
                    strcat(player_password, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            case SDL_KEYDOWN: // khi co input key tren ban phim
                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 1)
                {
                    player_name[strlen(player_name) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        player_name_texture->tWidth = 750;
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 2)
                {
                    player_password[strlen(player_password) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        player_password_texture->tWidth = 750;
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            default:
                break;
            }
        }
    }
    // for (i = 0; i < 3; i++)
    // {
    //     destroy_Texture(menuTexture[i]);
    // }
    destroy_Texture(background);
    // destroy_Texture(player_name_texture);
}

void Register_screen()
{
    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);
    // khoi tao
    int i;
    int choosing = 0;
    SDL_Rect player_name_rect;
    SDL_Rect player_password_rect;
    SDL_Rect player_repassword_rect;
    SDL_Rect thongbao_rect;
    int Menu_item = 1;
    SDL_Rect button[1];

    // int selection

    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));
    // load font menu
    TTF_Font *font2 = TTF_OpenFont("resource/arial.ttf", 42);

    SDL_Event menu_e;
    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    // load image background
    load_Texture_IMG("resource/register.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    char *menu[] = {"DANG KY"};
    int select[] = {0, 0, 0};
    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }

    // load thongbao
    thongbao_rect = CreateRect(200, 300, 1000, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &thongbao_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(thongbao_rect);
    thongbao = calloc(50, sizeof(char));
    strcpy(thongbao, "Vui long nhap thong tin");
    GameTexture *thongbao_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(thongbao, thongbao_texture, font2, blue_color);
    Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);

    // load ten player
    player_name_rect = CreateRect(435, 388, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_name_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_name_rect);
    player_name = calloc(20, sizeof(char));
    strcpy(player_name, " ");
    // render ten player
    GameTexture *player_name_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_name, player_name_texture, font2, white_color);
    Render(player_name_texture, player_name_rect.x, player_name_rect.y);

    // load password player
    player_password_rect = CreateRect(435, 488, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_password_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_password_rect);
    player_password = calloc(20, sizeof(char));
    strcpy(player_password, " ");
    // render password playerF
    GameTexture *player_password_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_password, player_password_texture, font2, white_color);
    Render(player_password_texture, player_password_rect.x, player_password_rect.y);

    // load repassword player
    player_repassword_rect = CreateRect(435, 588, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_repassword_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_repassword_rect);
    player_repassword = calloc(20, sizeof(char));
    strcpy(player_repassword, " ");
    // render repassword player
    GameTexture *player_repassword_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_repassword, player_repassword_texture, font2, white_color);
    Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);

    // render menu button
    // for (i = 0; i < 3; i++)
    // {
    //     load_Texture_Text(menu_text[i], menuTexture[i], font, red_color);
    //     button[i] = Render(menuTexture[i], 40, (SCREEN_HEIGHT + menuTexture[0]->tHeight * (i * 3 - 2)) / 2);
    // }
    SDL_RenderPresent(renderer);

    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText2[i], font2, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN: // khi chuot click
                if (check_mouse_pos(back_rect) == 1)
                {
                    for (i = 0; i < Menu_item; i++)
                    {
                        free(MenuText2[i]);
                    }
                    free(MenuText2);
                    free(background);
                    free(player_name_texture);
                    free(player_password_texture);
                    free(player_repassword_texture);
                    free(thongbao_texture);
                    menu_Screen();
                }
                if (check_mouse_pos(player_name_rect) == 1)
                {
                    choosing = 1;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x, player_name_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_password_rect) != 1 && check_mouse_pos(player_repassword_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, white_color);
                        Render(player_name_texture, player_name_rect.x, player_name_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(player_password_rect) == 1)
                {
                    // Mix_PlayChannel(-1, gScratch, 0);
                    choosing = 2;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x, player_password_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1 && check_mouse_pos(player_repassword_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, white_color);
                        Render(player_password_texture, player_password_rect.x, player_password_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(player_repassword_rect) == 1)
                {
                    Mix_HaltMusic();
                    choosing = 3;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1 && check_mouse_pos(player_password_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, white_color);
                        Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(button[0]) == 1)
                {
                    if (strcmp(player_name, " ") == 0 || strcmp(player_password, " ") == 0 || strcmp(player_repassword, " ") == 0)
                    {
                        strcpy(thongbao, "Vui long nhap day du thong tin!");
                        ClearRenderRect(thongbao_rect);
                        load_Texture_Text(thongbao, thongbao_texture, font2, blue_color);
                        Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                        SDL_RenderPresent(renderer);
                    }
                    else
                    {
                        ZeroMemory(buffer, MAX_BUFF);
                        strcat(buffer, "register:");
                        player_name = strtok(player_name, " ");
                        strcat(buffer, player_name);
                        strcat(buffer, " ");
                        player_password = strtok(player_password, " ");
                        strcat(buffer, player_password);
                        strcat(buffer, " ");
                        player_repassword = strtok(player_repassword, " ");
                        strcat(buffer, player_repassword);
                        send(sockfd, buffer, strlen(buffer), 0);
                        ZeroMemory(buffer, MAX_BUFF);
                        nread = recv(sockfd, buffer, 100, 0);
                        buffer[nread] = 0;
                        if (strcmp(buffer, "register:1") == 0)
                        {
                            strcpy(thongbao, "Dang ky thanh cong. Vui long dang nhap!");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, green_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                            SDL_Delay(3000);
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            free(player_name_texture);
                            free(player_password_texture);
                            free(player_repassword_texture);
                            free(thongbao_texture);
                            Login_screen();
                        }
                        else
                        {
                            strcpy(thongbao, "Tai khoan hoac ten nguoi dung da ton tai!!!");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, red_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;

            case SDL_TEXTINPUT: // khi co input tu ban phim
                if (strlen(player_name) < 30 && choosing == 1)
                {
                    strcat(player_name, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_password) < 30 && choosing == 2)
                {
                    strcat(player_password, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_repassword) < 30 && choosing == 3)
                {
                    strcat(player_repassword, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x + 3, player_repassword_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            case SDL_KEYDOWN: // khi co input key tren ban phim
                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 1)
                {
                    player_name[strlen(player_name) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        player_name_texture->tWidth = 750;
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 2)
                {
                    player_password[strlen(player_password) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        player_password_texture->tWidth = 750;
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 3)
                {
                    player_repassword[strlen(player_repassword) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        player_repassword_texture->tWidth = 750;
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x + 3, player_repassword_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            default:
                break;
            }
        }
    }
    // for (i = 0; i < 3; i++)
    // {
    //     destroy_Texture(menuTexture[i]);
    // }
    destroy_Texture(background);
    // destroy_Texture(player_name_texture);
}

void repassword_screen()
{
    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);
    // khoi tao
    int i;
    int choosing = 0;
    SDL_Rect player_name_rect;
    SDL_Rect player_password_rect;
    SDL_Rect player_repassword_rect;
    SDL_Rect thongbao_rect;
    int Menu_item = 1;
    SDL_Rect button[1];

    // int selection

    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));
    // load font menu
    TTF_Font *font2 = TTF_OpenFont("resource/arial.ttf", 42);

    SDL_Event menu_e;
    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    // load image background
    load_Texture_IMG("resource/repassword.bmp", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    char *menu[] = {"DOI MAT KHAU"};
    int select[] = {0, 0, 0};
    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }

    // load thongbao
    thongbao_rect = CreateRect(200, 300, 1000, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &thongbao_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(thongbao_rect);
    thongbao = calloc(50, sizeof(char));
    strcpy(thongbao, "Nhap day du cac truong");
    GameTexture *thongbao_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(thongbao, thongbao_texture, font2, blue_color);
    Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);

    // load ten player
    player_name_rect = CreateRect(435, 388, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_name_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_name_rect);
    player_name = calloc(20, sizeof(char));
    strcpy(player_name, username);
    // render ten player
    GameTexture *player_name_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_name, player_name_texture, font2, white_color);
    Render(player_name_texture, player_name_rect.x, player_name_rect.y);

    // load password player
    player_password_rect = CreateRect(435, 488, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_password_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_password_rect);
    player_password = calloc(20, sizeof(char));
    strcpy(player_password, " ");
    // render password playerF
    GameTexture *player_password_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_password, player_password_texture, font2, white_color);
    Render(player_password_texture, player_password_rect.x, player_password_rect.y);

    // load repassword player
    player_repassword_rect = CreateRect(435, 588, 750, 50);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_repassword_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_repassword_rect);
    player_repassword = calloc(20, sizeof(char));
    strcpy(player_repassword, " ");
    // render repassword player
    GameTexture *player_repassword_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_repassword, player_repassword_texture, font2, white_color);
    Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);

    // render menu button
    // for (i = 0; i < 3; i++)
    // {
    //     load_Texture_Text(menu_text[i], menuTexture[i], font, red_color);
    //     button[i] = Render(menuTexture[i], 40, (SCREEN_HEIGHT + menuTexture[0]->tHeight * (i * 3 - 2)) / 2);
    // }
    SDL_RenderPresent(renderer);

    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText2[i], font2, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText2[i], font2, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 700 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN: // khi chuot click
                if (check_mouse_pos(back_rect) == 1)
                {
                    for (i = 0; i < Menu_item; i++)
                    {
                        free(MenuText2[i]);
                    }
                    free(MenuText2);
                    free(background);
                    free(player_name_texture);
                    free(player_password_texture);
                    free(player_repassword_texture);
                    free(thongbao_texture);
                    menu_Screen();
                }

                if (check_mouse_pos(player_password_rect) == 1)
                {
                    // Mix_PlayChannel(-1, gScratch, 0);
                    choosing = 2;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x, player_password_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1 && check_mouse_pos(player_repassword_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, white_color);
                        Render(player_password_texture, player_password_rect.x, player_password_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(player_repassword_rect) == 1)
                {
                    Mix_HaltMusic();
                    choosing = 3;
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }
                else
                {
                    if (check_mouse_pos(player_name_rect) != 1 && check_mouse_pos(player_password_rect) != 1)
                        choosing = 0;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, white_color);
                        Render(player_repassword_texture, player_repassword_rect.x, player_repassword_rect.y);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (check_mouse_pos(button[0]) == 1)
                {
                    if (strcmp(player_name, " ") == 0 || strcmp(player_password, " ") == 0 || strcmp(player_repassword, " ") == 0)
                    {
                        strcpy(thongbao, "Vui long nhap day du thong tin!");
                        ClearRenderRect(thongbao_rect);
                        load_Texture_Text(thongbao, thongbao_texture, font2, green_color);
                        Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                        SDL_RenderPresent(renderer);
                    }
                    else
                    {
                        ZeroMemory(buffer, MAX_BUFF);
                        strcat(buffer, "repassword:");
                        player_name = strtok(player_name, " ");
                        strcat(buffer, player_name);
                        strcat(buffer, " ");
                        player_password = strtok(player_password, " ");
                        strcat(buffer, player_password);
                        strcat(buffer, " ");
                        player_repassword = strtok(player_repassword, " ");
                        strcat(buffer, player_repassword);
                        send(sockfd, buffer, strlen(buffer), 0);
                        ZeroMemory(buffer, MAX_BUFF);
                        nread = recv(sockfd, buffer, 100, 0);
                        buffer[nread] = 0;
                        if (strcmp(buffer, "repassword:1") == 0)
                        {
                            strcpy(thongbao, "Doi mat khau thanh cong!!!");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, green_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                            SDL_Delay(3000);
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            free(player_name_texture);
                            free(player_password_texture);
                            free(player_repassword_texture);
                            free(thongbao_texture);
                            ingame_menu();
                        }
                        else if (strcmp(buffer, "repassword:0") == 0)
                        {
                            strcpy(thongbao, "Nhap lai mat khau khong chinh xac!!!");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, red_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                        }
                        else
                        {
                            strcpy(thongbao, "Mat khau giong nhu cu!!!");
                            ClearRenderRect(thongbao_rect);
                            load_Texture_Text(thongbao, thongbao_texture, font2, red_color);
                            Render(thongbao_texture, thongbao_rect.x, thongbao_rect.y);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;

            case SDL_TEXTINPUT: // khi co input tu ban phim
                if (strlen(player_name) < 30 && choosing == 1)
                {
                    strcat(player_name, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_password) < 30 && choosing == 2)
                {
                    strcat(player_password, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (strlen(player_repassword) < 30 && choosing == 3)
                {
                    strcat(player_repassword, menu_e.text.text);
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x + 3, player_repassword_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            case SDL_KEYDOWN: // khi co input key tren ban phim
                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 1)
                {
                    player_name[strlen(player_name) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_name_rect);
                    if (strlen(player_name) != 0)
                    {
                        player_name_texture->tWidth = 750;
                        load_Texture_Text(player_name, player_name_texture, font2, black_color);
                        Render(player_name_texture, player_name_rect.x + 3, player_name_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 2)
                {
                    player_password[strlen(player_password) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_password_rect);
                    if (strlen(player_password) != 0)
                    {
                        player_password_texture->tWidth = 750;
                        load_Texture_Text(player_password, player_password_texture, font2, black_color);
                        Render(player_password_texture, player_password_rect.x + 3, player_password_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }

                if (menu_e.key.keysym.sym == SDLK_BACKSPACE && choosing == 3)
                {
                    player_repassword[strlen(player_repassword) - 1] = '\0';
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                    ClearRenderRect(player_repassword_rect);
                    if (strlen(player_repassword) != 0)
                    {
                        player_repassword_texture->tWidth = 750;
                        load_Texture_Text(player_repassword, player_repassword_texture, font2, black_color);
                        Render(player_repassword_texture, player_repassword_rect.x + 3, player_repassword_rect.y - 3);
                    }
                    SDL_RenderPresent(renderer);
                }
                break;
            default:
                break;
            }
        }
    }
    // for (i = 0; i < 3; i++)
    // {
    //     destroy_Texture(menuTexture[i]);
    // }
    destroy_Texture(background);
    // destroy_Texture(player_name_texture);
}

void menu_Screen()
{
    ResetRender();
    int i;
    int Menu_item = 3;
    SDL_Rect button[3];
    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 48);
    char *menu[] = {"DANG NHAP", "DANG KY", "THOAT"};
    int select[] = {0, 0, 0};
    SDL_Event menu_e;

    load_Texture_IMG("./resource/main_menu.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText[i], font, textColor);
        button[i] = Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 258 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText[i], font, highlight_Color);
                            Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 258 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText[i], font, textColor);
                            Render(MenuText[i], (SCREEN_WIDTH - MenuText[i]->tWidth) / 2, (SCREEN_HEIGHT + 258 + MenuText[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (i == 0)
                        {
                            Login_screen();
                        }
                        else if (i == 1)
                        {
                            Register_screen();
                        }
                        else if (i == 2)
                        {
                            close_win();
                            exit(0);
                        }
                    }
                }
            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText[i]->texture);
        MenuText[i]->tHeight = 0;
        MenuText[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
}

void Offline_screen()
{
    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);

    int i;
    int Menu_item = 3;
    SDL_Rect button[3];

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    char *menu[] = {"KHO", "TRUNG BINH", "DE"};
    int select[] = {0, 0, 0};
    SDL_Event menu_e;

    load_Texture_IMG("./resource/menu.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    for (i = 0; i < Menu_item; i++)
    {
        load_Texture_Text(menu[i], MenuText2[i], font, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
    }
    SDL_RenderPresent(renderer);
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text(menu[i], MenuText2[i], font, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text(menu[i], MenuText2[i], font, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT + 245 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (i = 0; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        // Playing_screen();
                        startPlayOffline(i);
                    }
                }
                if (check_mouse_pos(back_rect) == 1)
                {
                    ingame_menu();
                }

            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText2[i]->texture);
        MenuText2[i]->tHeight = 0;
        MenuText2[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
    return;
}

void *handle_chat(void *id)
{
    SDL_Delay(1000);
    pthread_detach(pthread_self());
    // Mix_PlayMusic(gMusic, -1);
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 80);
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(30, 30, 90, 90);
    // Render(back_texture, back_rect.x, back_rect.y);

    load_Texture_Text("0", back_texture, font, white_color);
    // Render(back_texture, back_rect.x, back_rect.y);
    Render_Center_box(back_texture, 35, 30, 90);

    timer = 0;
    char str[10];

    while (timer <= 15)
    {
        // if (flag == 1)
        // {
        //     pthread_exit(NULL);
        //     break;
        // }
        // SDL_Delay(1000);
        delay(1);
        // Sleep(2000);
        timer++;
        sprintf(str, "%d", timer);
        ClearRenderRect(back_rect);
        load_Texture_Text(str, back_texture, font, white_color);
        Render(back_texture, back_rect.x, back_rect.y);
        SDL_RenderPresent(renderer);
    }
    flag = 1;
    pthread_exit(NULL);
}

void sendresult(int count)
{
    char str[12];
    ZeroMemory(buffer, MAX_BUFF);
    strcat(buffer, "result:");
    strcat(buffer, idRoom);
    strcat(buffer, " ");
    sprintf(str, "%d", count);
    strcat(buffer, str);
    send(sockfd, buffer, strlen(buffer), 0);
    // ZeroMemory(buffer, MAX_BUFF);
    // nread = recv(sockfd, buffer, 100, 0);
    // buffer[nread] = 0;
}

void Playing_screen(int count)
{
    timer = 0;
    flag = 0;
    int done = 0; // de khi xong ko hover dk
    ResetRender();
    Mix_HaltMusic();

    // back

    // khoi tao
    int i;
    int available[4] = {1, 1, 1, 1};
    SDL_Rect player_level_rect;
    SDL_Rect player_suport1_rect;
    SDL_Rect player_suport2_rect;
    SDL_Rect player_suport3_rect;
    SDL_Rect suggest_rect;
    // SDL_Rect cauhoi_rect;
    char *cauhoi;

    SDL_Rect button[4];
    SDL_Rect buttonHover[4];

    GameTexture **optionTexture = (GameTexture **)malloc(sizeof(GameTexture *) * 4);
    for (i = 0; i < 4; i++)
    {
        optionTexture[i] = (GameTexture *)malloc(sizeof(GameTexture) * 4);
    }

    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));
    char optiontext[][100] = {
        "",
        "",
        "",
        "",
        "",
    };
    strcpy(optiontext[4], quiz_arr[questList[count]].question);
    strcpy(optiontext[0], quiz_arr[questList[count]].optionA);
    strcpy(optiontext[1], quiz_arr[questList[count]].optionB);
    strcpy(optiontext[2], quiz_arr[questList[count]].optionC);
    strcpy(optiontext[3], quiz_arr[questList[count]].optionD);

    // load font menu
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 30);

    SDL_Event menu_e;

    // load image background
    load_Texture_IMG("resource/playing.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    background->tWidth = surface->w;
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    // render suport
    GameTexture *player_suport1_texture = calloc(1, sizeof(GameTexture));
    GameTexture *player_suport2_texture = calloc(1, sizeof(GameTexture));
    GameTexture *player_suport3_texture = calloc(1, sizeof(GameTexture));
    player_suport1_rect = CreateRect(1253, 15, 80, 80);
    if (suport1check == 1)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderDrawRect(renderer, &player_suport1_rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        ClearRenderRect(player_suport1_rect);
    }
    player_suport2_rect = CreateRect(1253, 100, 80, 80);
    if (suport2check == 1)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderDrawRect(renderer, &player_suport2_rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        ClearRenderRect(player_suport2_rect);
    }
    player_suport3_rect = CreateRect(1253, 185, 80, 80);
    if (suport3check == 1)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderDrawRect(renderer, &player_suport3_rect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        ClearRenderRect(player_suport3_rect);
    }
    Render(player_suport1_texture, player_suport1_rect.x, player_suport1_rect.y);
    Render(player_suport2_texture, player_suport2_rect.x, player_suport2_rect.y);
    Render(player_suport3_texture, player_suport3_rect.x, player_suport3_rect.y);

    // render menu button
    for (i = 0; i < 4; i++)
    {
        load_Texture_Text(optiontext[i], optionTexture[i], font, white_color);
        // optionTexture[i]->tWidth = 600;
    }
    // load_Texture_Text(quiz_arr[count].optionA, optionTexture[0], font, white_color);
    // load_Texture_Text(quiz_arr[count].optionB, optionTexture[1], font, white_color);
    // load_Texture_Text(quiz_arr[count].optionC, optionTexture[2], font, white_color);
    // load_Texture_Text(quiz_arr[count].optionD, optionTexture[3], font, white_color);

    button[0] = Render(optionTexture[0], 210, 597);
    button[1] = Render(optionTexture[1], 750, 597);
    button[2] = Render(optionTexture[2], 210, 697);
    button[3] = Render(optionTexture[3], 750, 697);

    // GameTexture *optionTextureHover0 = calloc(1, sizeof(GameTexture));
    // GameTexture *optionTextureHover1 = calloc(1, sizeof(GameTexture));
    // GameTexture *optionTextureHover2 = calloc(1, sizeof(GameTexture));
    // GameTexture *optionTextureHover3 = calloc(1, sizeof(GameTexture));

    buttonHover[0] = CreateRect(180, 590, 480, 60);
    buttonHover[1] = CreateRect(750, 590, 480, 60);
    buttonHover[2] = CreateRect(180, 696, 480, 60);
    buttonHover[3] = CreateRect(750, 696, 480, 60);

    // render level
    char *player_level;
    player_level_rect = CreateRect(930, 332, 250, 30);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderDrawRect(renderer, &player_level_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    ClearRenderRect(player_level_rect);

    player_level = calloc(20, sizeof(char));
    sprintf(player_level, "%d. %sd", count + 1, getPrize(count));
    // strcpy(player_level, "2) 10.000.000d");
    // render ten player
    GameTexture *player_level_texture = calloc(1, sizeof(GameTexture));
    load_Texture_Text(player_level, player_level_texture, font, white_color);
    Render(player_level_texture, player_level_rect.x, player_level_rect.y);

    // render cauhoi va suggest
    cauhoi = calloc(1000, sizeof(char));
    // render cauhoi
    GameTexture *cauhoi_texture = calloc(1, sizeof(GameTexture));
    cauhoi_texture->tHeight = 50;
    load_and_Render_Texture_Text_Wrapped(optiontext[4], cauhoi_texture, font, white_color, 450, 920);

    char *suggest = calloc(1000, sizeof(char));
    strcpy(suggest, "Ban hay lua chon phuong an dung trong 4 dap an duoi day");

    // display suggest
    GameTexture *suggest_texture = calloc(1, sizeof(GameTexture));
    load_and_Render_Texture_Text_Wrapped(suggest, suggest_texture, font, white_color, 90, 920);

    suggest_rect = CreateRect(224, 70, 920, 70);
    // player_suport1_rect = CreateRect(224, 90, 638, 35);

    SDL_RenderPresent(renderer);
    // Mix_PlayChannel(-1, gStartGame, 0);
    int rightoption = atoi(quiz_arr[questList[count]].answers) - 1;

    while (1)
    {
        if (flag == 1)
        {
            flag = -1;
            for (i = 0; i < 4; i++)
            {
                free(optionTexture[i]);
            }
            free(optionTexture);
            free(player_level_texture);
            free(suggest_texture);
            free(cauhoi_texture);

            free(background);
            free(suggest);
            if (isOnline == 0)
            {
                result_offline_screen(count);
            }
            else
            {
                printf("a\n");
                sendresult(count);
                ZeroMemory(buffer, MAX_BUFF);
                sprintf(buffer, "outRoom:%s", idRoom);
                send(sockfd, buffer, strlen(buffer), 0);
                // printf("%s out rOOM\n", username);
                // Room_screen();
                // result_online_screen();
            }
        }
        while (SDL_PollEvent(&menu_e))
        {

            switch (menu_e.type)
            {
            case SDL_QUIT:
                timer = 15;
                ZeroMemory(buffer, MAX_BUFF);
                sprintf(buffer, "outRoom:%s", idRoom);
                send(sockfd, buffer, strlen(buffer), 0);
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot
                if (done == 0)
                {
                    for (i = 0; i < 4; i++)
                    {
                        if (check_mouse_pos(buttonHover[i]) == 1)
                        {
                            load_Texture_Text(optiontext[i], optionTexture[i], font, yellow_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                        }

                        else
                        {
                            load_Texture_Text(optiontext[i], optionTexture[i], font, white_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN: // khi chuot click
                if (check_mouse_pos(player_suport1_rect) == 1 && suport1check == 0)
                {
                    srand(time(0));
                    int loai1 = 0, loai2 = 0;
                    do
                    {
                        loai1 = rand() % 4;
                    } while (loai1 == rightoption);
                    do
                    {
                        loai2 = rand() % 4;
                    } while (loai1 == rightoption || loai1 == loai2);
                    // doi suggest
                    // strcpy(suggest, getHelp(rightoption, 0));
                    strcpy(suggest, "May tinh da bo di 2 phuong an sai! Xin moi ban lua chon");

                    ClearRenderRect(suggest_rect);
                    load_and_Render_Texture_Text_Wrapped(suggest, suggest_texture, font, white_color, 90, 920);

                    // doi 2 cau hoi
                    ClearRenderRect(button[loai1]);
                    ClearRenderRect(button[loai2]);
                    strcpy(optiontext[loai1], " ");
                    strcpy(optiontext[loai2], " ");
                    available[loai1] = 0;
                    available[loai2] = 0;

                    load_Texture_Text(optiontext[loai1], optionTexture[loai1], font, black_color);
                    switch (loai1)
                    {
                    case 0:
                        Render(optionTexture[0], 210, 596);
                        break;
                    case 1:
                        Render(optionTexture[1], 750, 596);
                        break;
                    case 2:
                        Render(optionTexture[2], 210, 696);
                        break;
                    case 3:
                        Render(optionTexture[3], 750, 696);
                        break;
                    }
                    load_Texture_Text(optiontext[loai2], optionTexture[loai2], font, black_color);
                    switch (loai2)
                    {
                    case 0:
                        Render(optionTexture[0], 210, 596);
                        break;
                    case 1:
                        Render(optionTexture[1], 750, 596);
                        break;
                    case 2:
                        Render(optionTexture[2], 210, 696);
                        break;
                    case 3:
                        Render(optionTexture[3], 750, 696);
                        break;
                    }
                    suport1check = 1;
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderDrawRect(renderer, &player_suport1_rect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    ClearRenderRect(player_suport1_rect);
                    Render(player_suport1_texture, player_suport1_rect.x, player_suport1_rect.y);
                    SDL_RenderPresent(renderer);
                }
                if (check_mouse_pos(player_suport2_rect) == 1 && suport2check == 0)
                {
                    switch (rightoption)
                    {
                    case 0:
                        strcpy(suggest, "Khan gia co 60\% chon A, 10\% chon B va 10\%chon C, 20\%chon D. Va lua chon cua ban la?");

                        break;
                    case 1:
                        strcpy(suggest, "20\% chon A, 40\% chon B, 35\%chon C va 5\%chon D. Rat sit sao, va dap an ma ban chon la?");

                        break;
                    case 2:
                        strcpy(suggest, "Co 0\% chon A, 0\% chon B, 80\%chon C va 20\%chon D. Hay dua ra suy nghi cua ban!");

                        break;
                    case 3:
                        strcpy(suggest, "30\% chon A, 20\% chon B, 10\%chon C, 40\%chon D. Ket qua ma ban dua ra la?");
                        break;
                    }
                    ClearRenderRect(suggest_rect);
                    load_and_Render_Texture_Text_Wrapped(suggest, suggest_texture, font, white_color, 90, 920);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderDrawRect(renderer, &player_suport2_rect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    ClearRenderRect(player_suport2_rect);
                    Render(player_suport2_texture, player_suport1_rect.x, player_suport1_rect.y);
                    SDL_RenderPresent(renderer);
                    suport2check = 1;
                }
                if (check_mouse_pos(player_suport3_rect) == 1 && suport3check == 0)
                {
                    switch (rightoption)
                    {
                    case 0:
                        strcpy(suggest, "Nguoi than cua ban da dua ra goi y la dap an A!");

                        break;
                    case 1:
                        strcpy(suggest, "B la dap an ma nguoi than cua ban da lua chon, vay con ban?");

                        break;
                    case 2:
                        strcpy(suggest, "Theo nguoi than cua ban do la C, lieu co phai la dap an chinh xac");

                        break;
                    case 3:
                        strcpy(suggest, "D la dap an ma nguoi than cua ban da goi y, hay dua ra lua chon cua minh!");
                        break;
                    }
                    ClearRenderRect(suggest_rect);
                    load_and_Render_Texture_Text_Wrapped(suggest, suggest_texture, font, white_color, 90, 920);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    SDL_RenderDrawRect(renderer, &player_suport2_rect);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                    ClearRenderRect(player_suport3_rect);
                    Render(player_suport3_texture, player_suport1_rect.x, player_suport1_rect.y);
                    SDL_RenderPresent(renderer);
                    suport3check = 1;
                }
                // if (check_mouse_pos(back_rect) == 1)
                // {
                //     Mix_PlayMusic(gMusic, -1);
                // }
                for (i = 0; i < 4; i++)
                {

                    if (available[i] == 1 && check_mouse_pos(buttonHover[i]) == 1 && i == rightoption)
                    {
                        // printf("%d", pthread_cancel(pid));
                        Mix_PlayChannel(-1, gRightAnswer, 0);
                        done = 1;
                        for (int j = 0; j < 5; j++)
                        {

                            load_Texture_Text(optiontext[i], optionTexture[i], font, yellow_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                            SDL_Delay(200);
                            load_Texture_Text(optiontext[i], optionTexture[i], font, green_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                            SDL_Delay(200);
                        }
                        count++;
                        for (i = 0; i < 4; i++)
                        {
                            free(optionTexture[i]);
                        }
                        free(optionTexture);
                        free(player_level_texture);
                        free(suggest_texture);
                        free(cauhoi_texture);

                        free(background);
                        free(suggest);
                        if (count == 14)
                        {
                            timer = 15;
                        }
                        else
                        {

                            Playing_screen(count);
                        }
                        break;
                    }
                    else if (available[i] == 1 && check_mouse_pos(buttonHover[i]) == 1 && i != rightoption)
                    {
                        // printf("%d", pthread_cancel(pid));
                        Mix_PlayChannel(-1, gWrongAnswer, 0);
                        done = 1;
                        for (int j = 0; j < 5; j++)
                        {
                            load_Texture_Text(optiontext[i], optionTexture[i], font, yellow_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                            SDL_Delay(200);
                            load_Texture_Text(optiontext[i], optionTexture[i], font, red_color);
                            switch (i)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                            SDL_Delay(200);
                        }
                        for (int j = 0; j < 5; j++)
                        {

                            load_Texture_Text(optiontext[rightoption], optionTexture[rightoption], font, yellow_color);
                            switch (rightoption)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                            SDL_Delay(200);
                            load_Texture_Text(optiontext[rightoption], optionTexture[rightoption], font, green_color);
                            switch (rightoption)
                            {
                            case 0:
                                Render(optionTexture[0], 210, 596);
                                break;
                            case 1:
                                Render(optionTexture[1], 750, 596);
                                break;
                            case 2:
                                Render(optionTexture[2], 210, 696);
                                break;
                            case 3:
                                Render(optionTexture[3], 750, 696);
                                break;
                            }
                            SDL_RenderPresent(renderer);
                        }
                        timer = 15;

                        // if (isOnline == 0)
                        // {
                        //     // SDL_Delay(3000);
                        //     result_offline_screen(count);
                        // }
                        // else
                        // {
                        //     // printf("%d", pthread_cancel(pid));
                        //     sendresult(count);
                        //     // SDL_Delay(3000);
                        //     result_online_screen();
                        // }
                        break;
                    }
                }
            }

            break;
        }
    }

    // for (i = 0; i < 3; i++)
    // {
    //     destroy_Texture(menuTexture[i]);
    // }
    destroy_Texture(background);
    // destroy_Texture(player_name_texture);
}

void startPlayOffline(int option)
{
    flag = 0;
    suport1check = 0;
    suport2check = 0;
    suport3check = 0;
    isOnline = 0;
    getQuesList(option);
    Mix_PlayChannel(-1, gStartGame, 0);
    SDL_Delay(3000);
    pthread_t pid;
    int *arg;
    pthread_create(&pid, NULL, &handle_chat, (void *)arg);
    Playing_screen(0);
}

void startPlayOnline()
{
    flag = 0;
    suport1check = 0;
    suport2check = 0;
    suport3check = 0;
    isOnline = 1;
    Mix_PlayChannel(-1, gStartGame, 0);
    SDL_Delay(3000);
    pthread_t pid;
    int *arg;
    pthread_create(&pid, NULL, &handle_chat, (void *)arg);
    Playing_screen(0);
}

void result_offline_screen(int count)
{

    ResetRender();
    int i;
    int Menu_item = 3;
    SDL_Rect button[3];

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);

    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    char menu[][100] = {
        " ",
        " ",
        "MENU"};
    int select[] = {
        0,
        0, 0};

    SDL_Event menu_e;

    load_Texture_IMG("resource/offline_result.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);
    // ten nguoi 1
    load_Texture_Text(username, MenuText[0], font, textColor);
    button[0] = Render(MenuText[0], (SCREEN_WIDTH - MenuText[0]->tWidth) / 2, (SCREEN_HEIGHT + 180 + MenuText[0]->tHeight * (0 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);
    // KET QUA NGUOI 1

    load_Texture_Text(getPrize(count), MenuText[1], font, textColor);
    button[1] = Render(MenuText[1], (SCREEN_WIDTH - MenuText[1]->tWidth) / 2, (SCREEN_HEIGHT + 145 + MenuText[0]->tHeight * (1 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);

    // in ra nút quay ve

    load_Texture_Text(menu[2], MenuText[2], font, textColor);
    button[2] = Render_Center(MenuText[2], 665);
    SDL_RenderPresent(renderer);

    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot

                if (check_mouse_pos(button[2]) == 1)
                {
                    if (select[2] == 0)
                    {
                        select[2] = 5;
                        load_Texture_Text(menu[2], MenuText[2], font, highlight_Color);
                        button[2] = Render_Center(MenuText[2], 665);
                        SDL_RenderPresent(renderer);
                    }
                }
                else
                {
                    if (select[2] != 0)
                    {
                        select[2] = 0;
                        load_Texture_Text(menu[2], MenuText[2], font, textColor);
                        button[2] = Render_Center(MenuText[2], 665);
                        SDL_RenderPresent(renderer);
                    }
                }

                break;
            case SDL_MOUSEBUTTONDOWN:
                if (check_mouse_pos(button[2]) == 1)
                {
                    ingame_menu();
                }
            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText[i]->texture);
        MenuText[i]->tHeight = 0;
        MenuText[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
}

void result_online_screen()
{
    ResetRender();
    int i;
    int Menu_item = 6;
    SDL_Rect button[6];

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);

    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    char menu[][100] = {
        "Nghiem",
        "1000",
        "thuy",
        "2000",
        "menu",
        ""};
    if (atoi(menu[1]) > atoi(menu[3]))
    {
        strcpy(menu[5], "WIN");
    }
    else if (atoi(menu[1]) < atoi(menu[3]))
    {
        strcpy(menu[5], "LOSE");
    }
    else
    {
        strcpy(menu[5], "DRAW");
    }
    int select[] = {
        0,
        0, 0, 0, 0, 0};

    SDL_Event menu_e;

    load_Texture_IMG("./resource/ketqua.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);
    // ten nguoi 1
    load_Texture_Text(menu[0], MenuText[0], font, textColor);
    button[0] = Render(MenuText[0], (SCREEN_WIDTH - MenuText[0]->tWidth) / 2 - 310, (SCREEN_HEIGHT + 290 + MenuText[0]->tHeight * (0 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);
    // KET QUA NGUOI 1
    load_Texture_Text(menu[1], MenuText[1], font, textColor);
    button[1] = Render(MenuText[1], (SCREEN_WIDTH - MenuText[1]->tWidth) / 2 - 310, (SCREEN_HEIGHT + 230 + MenuText[0]->tHeight * (1 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);
    // ten nguoi 2
    load_Texture_Text(menu[2], MenuText[2], font, textColor);
    button[2] = Render(MenuText[2], (SCREEN_WIDTH - MenuText[2]->tWidth) / 2 + 300, (SCREEN_HEIGHT - 155 + MenuText[0]->tHeight * (2 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);
    // KET QUA NGUOI 2
    load_Texture_Text(menu[3], MenuText[3], font, textColor);
    button[3] = Render(MenuText[3], (SCREEN_WIDTH - MenuText[3]->tWidth) / 2 + 300, (SCREEN_HEIGHT - 215 + MenuText[0]->tHeight * (3 * 4 - 2)) / 2);
    SDL_RenderPresent(renderer);

    // in ra nút quay ve
    load_Texture_Text(menu[4], MenuText[4], font, textColor);
    button[4] = Render_Center(MenuText[4], 670);
    SDL_RenderPresent(renderer);
    // IN RA THANG , THUA , HOA
    load_Texture_Text(menu[5], MenuText[5], font, textColor);
    button[5] = Render_Center(MenuText[5], 520);
    SDL_RenderPresent(renderer);
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION: // di chuyen chuot
                if (i == 4)
                {
                    if (check_mouse_pos(button[4]) == 1)
                    {
                        if (select[4] == 0)
                        {
                            select[4] = 5;
                            load_Texture_Text(menu[4], MenuText[4], font, highlight_Color);
                            button[4] = Render_Center(MenuText[4], 670);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[4] != 0)
                        {
                            select[4] = 0;
                            load_Texture_Text(menu[4], MenuText[4], font, textColor);
                            button[4] = Render_Center(MenuText[4], 670);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:

                if (check_mouse_pos(button[4]) == 1)
                {
                }

            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText[i]->texture);
        MenuText[i]->tHeight = 0;
        MenuText[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
}

void Waiting_screen(char *numberUser, char *listNameUser, char *idRoom)
{
    ResetRender();
    SDL_Delay(2000);
    int i;
    int Menu_item = 4;
    SDL_Rect button[4];
    char *nameUser;
    char menu[][100] = {"", "", "", "VS"};
    strcpy(menu[0], idRoom);
    if (atoi(numberUser) == 1)
    {
        nameUser = strtok(listNameUser, ",");
        strcpy(menu[1], nameUser);
    }
    else
    {
        nameUser = strtok(listNameUser, ",");
        strcpy(menu[1], nameUser);
        // char nameUser1[MAX_LINE] = {0};
        nameUser = strtok(NULL, ",");
        strcpy(menu[2], nameUser);
    }

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    // char *menu[] = {"101", "nghiem", "thuy", "VS"};

    load_Texture_IMG("./resource/phongcho.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);
    // 101
    load_Texture_Text(menu[0], MenuText[0], font, textColor);
    button[0] = Render(MenuText[0], (SCREEN_WIDTH - MenuText[0]->tWidth) / 2, (SCREEN_HEIGHT - 30 + MenuText[0]->tHeight * (-2)) / 2);
    SDL_RenderPresent(renderer);
    // nghiem
    load_Texture_Text(menu[1], MenuText[1], font, textColor);
    button[1] = Render(MenuText[1], (SCREEN_WIDTH - MenuText[1]->tWidth) / 2 - 350, (SCREEN_HEIGHT + 130 + MenuText[0]->tHeight * 2) / 2);
    SDL_RenderPresent(renderer);
    // thuy
    if (strcmp(menu[2], "") != 0)
    {
        // printf("%s %s\n", menu[1], menu[2]);
        load_Texture_Text(menu[2], MenuText[2], font, textColor);
        button[2] = Render(MenuText[2], (SCREEN_WIDTH - MenuText[2]->tWidth) / 2 + 300, (SCREEN_HEIGHT - 100 + MenuText[0]->tHeight * (6)) / 2);
        SDL_RenderPresent(renderer);
    }
    // vs
    load_Texture_Text(menu[3], MenuText[3], font, textColor);
    button[3] = Render_Center(MenuText[3], 510);
    SDL_RenderPresent(renderer);
    SDL_Delay(3000);

    // while (1)
    // {
    //     // while (SDL_PollEvent(&menu_e))
    //     // {
    //     //     switch (menu_e.type)
    //     //     {
    //     //     case SDL_QUIT:
    //     //         close_win();
    //     //         exit(0);
    //     //         break;
    //     //     default:
    //     //         break;
    //     //     }
    //     // }
    //     if (flag1)
    //         break;
    // }
    // for (i = 0; i < Menu_item; i++)
    // {
    //     SDL_DestroyTexture(MenuText[i]->texture);
    //     MenuText[i]->tHeight = 0;
    //     MenuText[i]->tWidth = 0;
    // }
    // SDL_DestroyTexture(background->texture);
    // background->tHeight = 0;
    // background->tWidth = 0;
}

void Waiting_Full_screen(char *numberUser, char *listNameUser, char *idRoom)
{
    ResetRender();
    SDL_Delay(2000);
    int i;
    int Menu_item = 4;
    SDL_Rect button[4];
    char *nameUser;
    char menu[][100] = {"", "", "", "VS"};
    strcpy(menu[0], idRoom);
    if (atoi(numberUser) == 1)
    {
        nameUser = strtok(listNameUser, ",");
        strcpy(menu[1], nameUser);
    }
    else
    {
        nameUser = strtok(listNameUser, ",");
        strcpy(menu[1], nameUser);
        // char nameUser1[MAX_LINE] = {0};
        nameUser = strtok(NULL, ",");
        strcpy(menu[2], nameUser);
    }

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    // char *menu[] = {"101", "nghiem", "thuy", "VS"};

    load_Texture_IMG("./resource/phongcho.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);
    // 101
    load_Texture_Text(menu[0], MenuText[0], font, textColor);
    button[0] = Render(MenuText[0], (SCREEN_WIDTH - MenuText[0]->tWidth) / 2, (SCREEN_HEIGHT - 30 + MenuText[0]->tHeight * (-2)) / 2);
    SDL_RenderPresent(renderer);
    // nghiem
    load_Texture_Text(menu[1], MenuText[1], font, textColor);
    button[1] = Render(MenuText[1], (SCREEN_WIDTH - MenuText[1]->tWidth) / 2 - 350, (SCREEN_HEIGHT + 130 + MenuText[0]->tHeight * 2) / 2);
    SDL_RenderPresent(renderer);
    // thuy
    if (strcmp(menu[2], "") != 0)
    {
        // printf("%s %s\n", menu[1], menu[2]);
        load_Texture_Text(menu[2], MenuText[2], font, textColor);
        button[2] = Render(MenuText[2], (SCREEN_WIDTH - MenuText[2]->tWidth) / 2 + 300, (SCREEN_HEIGHT - 100 + MenuText[0]->tHeight * (6)) / 2);
        SDL_RenderPresent(renderer);
    }
    // vs
    load_Texture_Text(menu[3], MenuText[3], font, textColor);
    button[3] = Render_Center(MenuText[3], 510);
    SDL_RenderPresent(renderer);
    // while (1)
    // {
    //     // while (SDL_PollEvent(&menu_e))
    //     // {
    //     //     switch (menu_e.type)
    //     //     {
    //     //     case SDL_QUIT:
    //     //         close_win();
    //     //         exit(0);
    //     //         break;
    //     //     default:
    //     //         break;
    //     //     }
    //     // }
    //     if (flag1 == 1)
    //         break;
    // }
    // for (i = 0; i < Menu_item; i++)
    // {
    //     SDL_DestroyTexture(MenuText[i]->texture);
    //     MenuText[i]->tHeight = 0;
    //     MenuText[i]->tWidth = 0;
    // }
    // SDL_DestroyTexture(background->texture);
    // destroy_Texture(background);
    // background->tHeight = 0;
    // background->tWidth = 0;
}

void FullRoom_screen()
{
    ResetRender();
    int i;
    int Menu_item = 1;
    SDL_Rect button[1];

    GameTexture **MenuText = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    // char menu[][100] = {
    //     "101",
    // };
    load_Texture_IMG("./resource/phongday.png", background);
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    // in ra id phong
    load_Texture_Text(idRoom, MenuText[0], font, textColor);
    button[0] = Render_Center(MenuText[0], 325);
    SDL_RenderPresent(renderer);

    // while (1)
    // {
    //     while (SDL_PollEvent(&menu_e))
    //     {
    //         switch (menu_e.type)
    //         {
    //         case SDL_QUIT:
    //             close_win();
    //             exit(0);
    //             break;

    //         default:
    //             break;
    //         }
    //     }
    // }
    // for (i = 0; i < Menu_item; i++)
    // {
    //     SDL_DestroyTexture(MenuText[i]->texture);
    //     MenuText[i]->tHeight = 0;
    //     MenuText[i]->tWidth = 0;
    // }
    // SDL_DestroyTexture(background->texture);
    // background->tHeight = 0;
    // background->tWidth = 0;
}

void *start(void *Arg)
{
    SDL_Delay(3000);
    // Sleep(5000);
    Argument *arg = (Argument *)Arg;
    pthread_detach(pthread_self());
    char buffer[MAX_LINE] = {0};

    while (1)
    {
        char *c;
        nread = recv(sockfd, buffer, MAX_LINE, 0);
        buffer[nread] = 0;
        // printf("%s\n", buffer);

        c = strtok(buffer, ":");

        system("clear");

        // nếu thông điệp nhận được là "wait"
        if (strcmp(c, "wait") == 0)
        {
            char numberUser[10] = {0};
            char nameUser[MAX_LINE] = {0};

            c = strtok(NULL, " ");
            strcpy(numberUser, c);
            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            c = strtok(NULL, " ");
            strcpy(nameUser, c);

            // gotoxy(0, 0);
            printf("Nick name : %s\n", "nameOfClient");
            // gotoxy(20, 2);
            printf("Cho du so luong nguoi choi...\n");
            // gotoxy(20, 3);
            printf("Dang co %s nguoi trong phong\n", numberUser);
            // gotoxy(20, 4);
            printf("Danh sach ten nguoi choi : %s\n", nameUser);
            getQuesList(1);
            Waiting_screen(numberUser, nameUser, idRoom);
            ResetRender();
        }
        else if (strcmp(c, "oke") == 0) // nếu oke rồi thì chơi
        {
            char numberUser[10] = {0};
            char nameUser[MAX_LINE] = {0};

            c = strtok(NULL, " ");
            strcpy(numberUser, c);
            c = strtok(NULL, " ");
            strcpy(idRoom, c);
            c = strtok(NULL, " ");
            strcpy(nameUser, c);
            // c = strtok(NULL, ",");
            // strcpy(nameUser, c);

            // gotoxy(0, 0);
            printf("Nick name : %s\n", "nameOfClient");
            // gotoxy(20, 2);
            printf("Da du nguoi trong phong\n");
            // gotoxy(20, 3);
            printf("Dang co %s nguoi trong phong\n", numberUser);
            // gotoxy(20, 4);
            printf(" oke Danh sach ten nguoi choi : %s\n", nameUser);
            // sleep(2);
            printf("Chuan bi bat dau tro choi\n");
            // sleep(1);
            getQuesList(1);
            Waiting_Full_screen(numberUser, nameUser, idRoom);
            SDL_Delay(2000);
            arg->result = 1;
            break;
        }
        else if (strcmp(c, "full") == 0)
        {
            FullRoom_screen();
            SDL_Delay(2000);
            arg->result = 0;
            break;
        }
    }
    // playgame();
}

void Room_screen()
{

    ResetRender();
    // back
    SDL_Rect back_rect;
    GameTexture *back_texture = calloc(1, sizeof(GameTexture));
    back_rect = CreateRect(20, 20, 80, 80);
    Render(back_texture, back_rect.x, back_rect.y);
    int i;
    int a = 5; // a la so phong
    int Menu_item = 9;
    SDL_Rect button[9];

    GameTexture **MenuText2 = (GameTexture **)malloc(sizeof(GameTexture *) * Menu_item);
    for (i = 0; i < Menu_item; i++)
    {
        MenuText2[i] = (GameTexture *)malloc(sizeof(GameTexture) * Menu_item);
    }
    GameTexture *background = (GameTexture *)malloc(sizeof(GameTexture));

    SDL_Color textColor = {255, 255, 255};
    SDL_Color highlight_Color = {255, 255, 0};
    TTF_Font *font = TTF_OpenFont("resource/arial.ttf", 50);
    // gửi đi thông điệp để nhận các phòng hiện tại đang có - cũng k cần thiết vì là TCP
    send(sockfd, "room:room", 100, 0);
    char buffer[1000];
    // ZeroMemory(buffer, 1000);
    nread = recv(sockfd, buffer, 1000, 0);
    // printf("buffer %s %d, nread %d\n", buffer, strlen(buffer), nread);
    // lưu các phòng đó vào room
    buffer[nread] = 0;
    char *c = strtok(buffer, " ");
    // char *menu[] = {"100", "101", "102", "103", "104"};
    // char **menu = calloc(100, sizeof(char *));
    char menu[100][100] = {0};

    // memset(menu, 10000, )
    i = 0;
    while (c != NULL)
    {
        // ZeroMemory(menu[i], 100);
        strcat(menu[i], c);
        c = strtok(NULL, " ");
        strcat(menu[i], "  ");
        strcat(menu[i], c);
        strcat(menu[i], "/2");
        c = strtok(NULL, " ");
        // printf("c: %s\n", menu[i]);
        i++;
    }
    totalRoom = i;
    a = totalRoom;
    // strcpy(menu[i + 1], "TẠO PHÒNG");
    int select[] = {
        0,
        0,
        0, 0, 0, 0, 0, 0, 0};
    SDL_Event menu_e;
    switch (a)
    {
    case 0:
        load_Texture_IMG("./resource/room0.png", background);
        break;
    case 1:
        load_Texture_IMG("./resource/room1.png", background);
        break;
    case 2:
        load_Texture_IMG("./resource/room2.png", background);
        break;
    case 3:
        load_Texture_IMG("./resource/room3.png", background);
        break;
    case 4:
        load_Texture_IMG("./resource/room4.png", background);
        break;
    case 5:
        load_Texture_IMG("./resource/room5.png", background);
        break;
    case 6:
        load_Texture_IMG("./resource/room6.png", background);
        break;
    case 7:
        load_Texture_IMG("./resource/room7.png", background);
        break;
    case 8:
        load_Texture_IMG("./resource/room8.png", background);
        break;
    default:
        break;
    }
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    background->tWidth = surface->w; // get sizeof background
    background->tHeight = surface->h;
    SDL_FreeSurface(surface);
    Render(background, 0, 0);

    for (int i = 0; i < a; i++)
    {
        if (i < 4)
        {
            load_Texture_Text(menu[i], MenuText2[i], font, textColor);
            button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 - 380, (SCREEN_HEIGHT - 275 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
            SDL_RenderPresent(renderer);
        }
        else
        {
            load_Texture_Text(menu[i], MenuText2[i], font, textColor);
            button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 + 380, (SCREEN_HEIGHT - 1165 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
            SDL_RenderPresent(renderer);
        }
    }
    for (i = Menu_item - 1; i < Menu_item; i++)
    {
        load_Texture_Text("TAO PHONG", MenuText2[i], font, textColor);
        button[i] = Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT - 1150 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
        SDL_RenderPresent(renderer);
    }
    while (1)
    {
        while (SDL_PollEvent(&menu_e))
        {
            switch (menu_e.type)
            {
            case SDL_QUIT:
                close_win();
                exit(0);
                break;
            case SDL_MOUSEMOTION:
                for (i = 0; i < a; i++)
                {
                    if (i < 4)
                    {
                        if (check_mouse_pos(button[i]) == 1)
                        {
                            if (select[i] == 0)
                            {
                                select[i] = i + 1;
                                load_Texture_Text(menu[i], MenuText2[i], font, highlight_Color);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 - 380, (SCREEN_HEIGHT - 275 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                        else
                        {
                            if (select[i] != 0)
                            {
                                select[i] = 0;
                                load_Texture_Text(menu[i], MenuText2[i], font, textColor);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 - 380, (SCREEN_HEIGHT - 275 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                    }
                    else
                    {
                        if (check_mouse_pos(button[i]) == 1)
                        {
                            if (select[i] == 0)
                            {
                                select[i] = i + 1;
                                load_Texture_Text(menu[i], MenuText2[i], font, highlight_Color);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 + 380, (SCREEN_HEIGHT - 1165 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                        else
                        {
                            if (select[i] != 0)
                            {
                                select[i] = 0;
                                load_Texture_Text(menu[i], MenuText2[i], font, textColor);
                                Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2 + 380, (SCREEN_HEIGHT - 1165 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                                SDL_RenderPresent(renderer);
                            }
                        }
                    }
                }
                for (i = Menu_item - 1; i < Menu_item; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        if (select[i] == 0)
                        {
                            select[i] = i + 1;
                            load_Texture_Text("TAO PHONG", MenuText2[i], font, highlight_Color);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT - 1150 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                    else
                    {
                        if (select[i] != 0)
                        {
                            select[i] = 0;
                            load_Texture_Text("TAO PHONG", MenuText2[i], font, textColor);
                            Render(MenuText2[i], (SCREEN_WIDTH - MenuText2[i]->tWidth) / 2, (SCREEN_HEIGHT - 1150 + MenuText2[0]->tHeight * (i * 4 - 2)) / 2);
                            SDL_RenderPresent(renderer);
                        }
                    }
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (check_mouse_pos(back_rect) == 1)
                {
                    for (i = 0; i < Menu_item; i++)
                    {
                        free(MenuText2[i]);
                    }
                    free(MenuText2);
                    free(background);
                    ingame_menu();
                }
                for (i = 0; i < totalRoom; i++)
                {
                    if (check_mouse_pos(button[i]) == 1)
                    {
                        ZeroMemory(buffer, 1000);
                        strcat(buffer, "start:");
                        // strcat(buffer, menu[i]);
                        char *c = strtok(menu[i], " ");
                        strcpy(idRoom, c);
                        strcat(buffer, c);
                        strcat(buffer, " ");
                        strcat(buffer, username);
                        // gửi thông điệp sẵn sàng
                        send(sockfd, buffer, strlen(buffer), 0);
                        pthread_t tid;

                        Argument *arg = calloc(1, sizeof(Argument));
                        arg->result = 0;
                        pthread_create(&tid, NULL, &start, (void *)arg);
                        pthread_join(tid, NULL);
                        int result = arg->result;
                        // printf(" result : %d\n", result);
                        if (result)
                        {
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            startPlayOnline();
                        }
                        else
                        {
                            for (i = 0; i < Menu_item; i++)
                            {
                                free(MenuText2[i]);
                            }
                            free(MenuText2);
                            free(background);
                            Room_screen();
                        }
                    }
                }
                if (check_mouse_pos(button[Menu_item - 1]) == 1)
                {
                    ZeroMemory(buffer, 1000);
                    strcat(buffer, "createRoom:");
                    strcat(buffer, username);
                    send(sockfd, buffer, strlen(buffer), 0);
                    ZeroMemory(buffer, 1000);
                    nread = recv(sockfd, buffer, 1000, 0);
                    // printf("after create Room: %s", buffer);
                    char *c = strtok(buffer, ":");
                    c = strtok(NULL, " ");
                    strcpy(idRoom, c);
                    // printf("idRoom %s\n", c);
                    // Room_screen();
                    // Waiting_screen("1", "nghiemdz,", c);
                    ZeroMemory(buffer, 1000);
                    strcat(buffer, "start:");
                    strcat(buffer, idRoom);
                    strcat(buffer, " ");
                    strcat(buffer, username);
                    send(sockfd, buffer, strlen(buffer), 0);

                    pthread_t tid;

                    Argument *arg = calloc(1, sizeof(Argument));
                    arg->result = 0;
                    pthread_create(&tid, NULL, &start, (void *)arg);
                    pthread_join(tid, NULL);
                    int result = arg->result;
                    // printf(" result : %d\n", result);
                    for (i = 0; i < Menu_item; i++)
                    {
                        SDL_DestroyTexture(MenuText2[i]->texture);
                        MenuText2[i]->tHeight = 0;
                        MenuText2[i]->tWidth = 0;
                    }
                    SDL_DestroyTexture(background->texture);
                    background->tHeight = 0;
                    background->tWidth = 0;

                    for (i = 0; i < Menu_item; i++)
                    {
                        free(MenuText2[i]);
                    }
                    free(MenuText2);
                    free(background);
                    Playing_screen(0);
                }
                break;

                break;
            default:
                break;
            }
        }
    }
    for (i = 0; i < Menu_item; i++)
    {
        SDL_DestroyTexture(MenuText2[i]->texture);
        MenuText2[i]->tHeight = 0;
        MenuText2[i]->tWidth = 0;
    }
    SDL_DestroyTexture(background->texture);
    background->tHeight = 0;
    background->tWidth = 0;
}

int main(int argc, char **argv)
{

    // setup client

    sockfd = setupSocket();
    server_addr = setupServerAddr();

    // Connect to remote server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        puts("connect error");
        return 1;
    }

    if (CreateWindowGame() == 0)
    {
        exit(ERROR);
    }

    readFileQs();
    // getQuesList();
    // Playing_screen();
    // repassword_screen();
    // selection = ingame_menu(selection);
    // selection = Login_screen(selection);
    // selection = room(selection);
    menu_Screen();

    close_win();
    return SUCCEED;
}