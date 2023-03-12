#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include<stdint.h>
#include<pthread.h>
#include<windef.h>
#include <time.h>
#include"message.h"
#include"define.h"

struct sockaddr_in server_addr;
struct sockaddr_in game_addr; 
struct sockaddr_in game_addr_1; // dia chi server
pthread_t tid1, tid2; // bien tao thread
pthread_t tid;
SOCKET sockfd; 
SOCKET gamefd;
SOCKET gamefd_1;
WSADATA wsa;
char **token ; // token phuc vu phan tach message
char *roomName; // ten room cua User hien tai

PlayerInfor *currUser; // User hien tai

typedef struct account{ 
	int id;
	char *username;
	char *password;
}account; 


typedef struct _Arg{ // struct bao gom thong tin user de cho vao thread
	PlayerInfor *currUser;
    enum mess_type type;
    char *buffer;
}Arg; 

typedef struct _Row{  // struct thong tin room
    char *roomName;
    char *playerCount;
    char *maxPlayer;
}Row;

int roomcount; // so room duoc server tra ve
Row *roomlist;// list room phu hop duoc server tra ve khi chon find_room

void GetCurrUser(PlayerInfor *currUser ){ // Cap phat dong cho CurrentUser
    // currUser = (PlayerInfor*)calloc(1, sizeof(PlayerInfor));
    currUser->name = (char*)calloc(MAX_NAME, sizeof(char));
    currUser->room = (char*)calloc(MAX_RNAME, sizeof(char));
    currUser->id = -1;
    currUser->isHost = -1;
}
int setupClient(){ // Setup client
	//Initialise winsock
    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0){
		printf("Failed. Error Code : %d\n", WSAGetLastError());
		return -1;
	}
	//Create socket
	if((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR){
		printf("Socket creation failed : %d\n" , WSAGetLastError());
		return -1;
	}

    if((gamefd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR){
		printf("Socket creation failed : %d\n" , WSAGetLastError());
		return -1;
	}

    if((gamefd_1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR){
		printf("Socket creation failed : %d\n" , WSAGetLastError());
		return -1;
	}
    
    memset(&game_addr, 0, sizeof(game_addr));
    game_addr.sin_family = AF_INET;
	game_addr.sin_port = htons(5500 + 2);
	game_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    //Setup address structure
    memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5500);
	server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    memset(&game_addr_1, 0, sizeof(game_addr_1));
    game_addr_1.sin_family = AF_INET;
	game_addr_1.sin_port = htons(5503);
	game_addr_1.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    return 1;
}

void closeClient(SOCKET sockfd){ // dong client
    closesocket(sockfd);
	WSACleanup();
}
void sendToServer(SOCKET sockfd, struct sockaddr_in server_addr, char *buffer){  // gui den server
    int length = (int)sizeof(server_addr);
    sendto(sockfd, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, length);
}
void ListenToServer(SOCKET sockfd, struct sockaddr_in server_addr, char *buffer){ // nghe tu server
    int buffer_size, length = sizeof(server_addr);
    buffer_size = recvfrom(sockfd, buffer, MAX_MESSAGE, 0, (SOCKADDR *)&server_addr, &length);
    buffer[buffer_size] = '\0';
}

void *sendToPingServer(){ // send ping lien tuc den server xac nhan con ket noi
    pthread_detach(pthread_self());
    char *buffer = calloc(4, sizeof(char));
    sprintf(buffer, "%d", currUser->id);
    printf("%s\n", buffer);
    struct sockaddr_in pingServer;
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ZeroMemory(&pingServer, sizeof(pingServer));
    pingServer.sin_family = AF_INET;
	pingServer.sin_port = htons(PORT + 1);
	pingServer.sin_addr.S_un.S_addr = inet_addr(SERVER_IP_ADDR);
    while(currUser->id != -1){
        sendToServer(sockfd, pingServer, buffer);
        Sleep(1000);
    }
    return NULL;
}

void register_Acc(){ // dang ki
	char *buffer = (char *)malloc(sizeof(char) * MAX);
    int length = sizeof(server_addr);
    int buff_size = 0;
    int c;
    printf("User: ");
    fflush(stdin);
    scanf("%[^\n]%*c", token[0]);
    printf("Password: ");
    fflush(stdin);
    scanf("%[^\n]%*c", token[1]);
	buffer = MakeMessage(token,2,REGISTER_GAME);
	cleanToken(token,2);
	sendToServer(sockfd,server_addr,buffer);
    printf("send OK %s\n", buffer);
}

void login_to_server(){ // dang nhap
    char *buffer = (char *)malloc(sizeof(char) * MAX);
    int length = sizeof(server_addr);
    int buff_size = 0;
    int c;
    printf("User: ");
    fflush(stdin);
    scanf("%[^\n]%*c", token[0]);
    printf("Password: ");
    fflush(stdin);
    scanf("%[^\n]%*c", token[1]);
	buffer = MakeMessage(token,2,LOGIN_GAME);
	cleanToken(token,2);
	sendToServer(sockfd,server_addr,buffer);
}

void host_game(PlayerInfor *currUser){    // ham tao 1 room 
    char **token = makeToken();
    char *buffer = (char*)calloc(MAX_MESSAGE ,sizeof(char));
    roomName = calloc(MAX_RNAME, sizeof(char));
    sprintf(token[0],"%s",currUser->name);
    strcpy(roomName , token[0]);
    memset(buffer, 0, sizeof(*buffer));
    buffer = MakeMessage(token, 1, HOST_GAME);
    sendToServer(sockfd, server_addr, buffer); // gui yeu cau tao room len server
    memset(buffer, 0, sizeof(*buffer));
    ListenToServer(sockfd, server_addr, buffer); // nhan lai ket qua tu server
    memset(token, 0, sizeof(*token[0]));
    token = GetToken(buffer, 2);
    enum mess_type type = (enum mess_type)atoi(token[0]);
    if(type == SUCCEED_PACK){ // tao duoc room
        printf("[+]Room's status -> Creating -> %s\n", token[1]);
        strcpy(currUser->room, roomName);
        currUser->isHost = 1;
        memset(token, 0, sizeof(*token[0]));;
        printf("Ban dang trong room : %s\n",currUser->room );
        
        
    }else if(type == ERROR_PACK){ // khong tao duoc
         printf("[-]Room's status -> Creating -> %s\n", token[1]);
    }else{
        
    }
    cleanToken(token, 2);
    
}

void GetRoomList( Row *roomlist, char **token){ // Gui yeu cau tim phong len server va nhan lai danh sach phong
    int i, j;
    char *buffer = calloc(MAX_MESSAGE ,sizeof(char));
    memset(buffer, 0, sizeof(*buffer));
    buffer = MakeMessage(token, 0, FIND_ROOM);
    sendToServer(sockfd, server_addr, buffer);
    memset(buffer, 0, sizeof(*buffer));
    ListenToServer(sockfd, server_addr, buffer);

    memset(token, 0, sizeof(*token[0]));
    token = GetToken(buffer, 2);
    
    enum mess_type type = (enum mess_type)atoi(token[0]);
    if(type == FIND_ROOM){
        roomcount = atoi(token[1]);
        printf("[+]Room founds: %d rooms!\n", roomcount);
    }else{
        printf("[-]Can't get room list from server!\n");
        return;
    }
    for(i = 0; i < roomcount; i++){  
        roomlist[i].roomName = calloc(MAX_RNAME , sizeof(char));
        roomlist[i].playerCount = calloc(4 , sizeof(char));
        roomlist[i].maxPlayer = calloc(4 , sizeof(char));
    }

    memset(buffer, 0, sizeof(*buffer));
    ListenToServer(sockfd, server_addr, buffer);
    memset(token, 0, sizeof(*token[0]));
    token = GetToken(buffer, roomcount * 3 + 1);
    j = 1;
    for(i = 0; i < roomcount; i++){  // phan tach token de lay thong tin phong
        strcpy(roomlist[i].roomName, token[j++]);
        strcpy(roomlist[i].maxPlayer, token[j++]);
        strcpy(roomlist[i].playerCount, token[j++]);
        roomlist[i].roomName[strlen(roomlist[i].roomName)] = '\0';
        roomlist[i].maxPlayer[strlen(roomlist[i].maxPlayer)] = '\0';
        roomlist[i].playerCount[strlen(roomlist[i].playerCount)] = '\0'; 
    }

    for (i = 0; i < roomcount; i++){ // in ra danh sach phong
        printf("%d roomName:%s max %s players , now: %s\n",i , roomlist[i].roomName, roomlist[i].maxPlayer, roomlist[i].playerCount);
    }
    
    for(i = 0; i < roomcount *3 + 1; i++){
        memset(token[i], 0, sizeof(*token[i]));
    }
    free(buffer);
}

void join_room(int row, Row *roomlist, PlayerInfor *currUser){  // tham gia vao 1 room nao do
    enum mess_type type;
    char *buffer = calloc(MAX_MESSAGE ,sizeof(char));
    printf("row = %d and room = %s and player = %d!\n", row, roomlist[row].roomName, currUser->id); // test 
    memset(buffer, 0, sizeof(*buffer));
    strcpy(token[0], roomlist[row].roomName);
    printf("token = %s!\n", token[0]);
    buffer = MakeMessage(token, 1, JOIN_ROOM);
    cleanToken(token,1);
    printf ("%s\n", buffer);
    sendToServer(sockfd, server_addr, buffer); // gui message bao gom type join room va ten phong muon join
    memset(buffer, 0, sizeof(*buffer));
    ListenToServer(sockfd, server_addr, buffer);
    memset(token, 0, sizeof(*token[0]));
    token = GetToken(buffer, 2);
    
    type = (enum mess_type)atoi(token[0]);
    if(type == SUCCEED_PACK){
        printf("[+]Joining Room -> %s\n", token[1]);
        strcpy(currUser->room, roomlist[row].roomName);
        memset(token, 0, sizeof(*token[0]));
        return;
    }else if(type == ERROR_PACK){
        printf("[-]Joining Room -> %s\n", token[1]);
            for(int i = 0; i < 2; i++){
                    memset(token[i], 0 ,sizeof(*token[i]));
                }
        }    
}

void exit_room(){
    char *buffer = calloc(4, sizeof(char));
    memset(buffer, 0, sizeof(*buffer));
    makeToken();
    buffer = MakeMessage(token, 0, EXIT_ROOM);
    sendToServer(sockfd, server_addr, buffer);
    free(buffer);
}

void start_game(PlayerInfor *currUser){ //gui tin hieu bat dau game len server
        enum mess_type type;
        char *buffer = calloc(MAX_MESSAGE ,sizeof(char));
        printf("You are %s , HOST of room %s \n", currUser->name, currUser->room);
        memset(buffer, 0, sizeof(*buffer));
        buffer = MakeMessage(token, 0, START_GAME);
        sendToServer(sockfd, server_addr, buffer);
        memset(buffer, 0, sizeof(*buffer));
        ListenToServer(sockfd, server_addr, buffer);
        printf("%s\n", buffer);
        free(buffer);
}

void *sendToInGame(){ // send ping lien tuc den server xac nhan con ket noi
    // pthread_detach(pthread_self());
    int i = 0;
    int my_hp;
    char *buffer = calloc(4, sizeof(char));
    memset(buffer, 0, sizeof(*buffer));
    char *buffer_1 = calloc(4, sizeof(char));
    memset(buffer_1, 0, sizeof(*buffer_1));
    char **token = makeToken();
    while(1){
        sprintf(token[0], "%d", currUser->id);
        strcpy(token[1], "trigger");
      
        if(i ==5 ) sprintf(token[2],"%d",0);
        else   sprintf(token[2],"%d",19);
        buffer = MakeMessage(token, 3, IN_GAME);
        cleanToken(token, 3);
        sendToServer(gamefd, game_addr, buffer);
        memset(buffer, 0, sizeof(*buffer));
      
        Sleep(1000);
        ListenToServer(sockfd, server_addr, buffer_1);
        //trigger - hp 
        //ingame-id-trigger-hp
        //if atoi hp <0 ko gui 
        token = GetToken(buffer_1, 4);
        if(atoi(token[1]) == currUser->id){
            my_hp = atoi(token[3]);
        }
        if(atoi(token[1]) != currUser->id){ // loai mess gui cho chinh no
            printf("Client kia gui %s \n", buffer_1);   
            if(atoi(token[3]) == 0 && my_hp !=0){
                printf("Ban da thang ");
                break;
            }
        }
        i++;
         cleanToken(token, 4);
    }
    return NULL;
}

void in_game(){ // xu ly in game, dang lam
    int dem = 20;
    enum mess_type type;
    char *buffer = calloc(MAX_MESSAGE ,sizeof(char));
    memset(buffer, 0, sizeof(*buffer));
    char **token = makeToken();
    ListenToServer(sockfd, server_addr, buffer);
    printf("%s\n", buffer);
    type = GetType(buffer);
    if (type == IN_GAME){
        memset(buffer, 0, sizeof(*buffer));
        buffer = MakeMessage(token, 0, IN_GAME);
        sendToServer(sockfd, server_addr, buffer);
        printf("Gui den SERVER \n");
        sendToInGame();
    }
    else if (type == OUT_ROOM){
        printf("Ban bi da khoi phong\n");
    }
    
    
    // while (dem > 0){
    //     dem --;
    //     memset(buffer, 0, sizeof(*buffer));
    //     strcpy(token[0], "trigger");
    //     sprintf(token[1], "%d", dem);
    //     buffer = MakeMessage(token, 2, IN_GAME);
    //     cleanToken(token, 2);
    //     sendToServer(sockfd, server_addr, buffer);
    //     memset(buffer, 0, sizeof(*buffer));
    //     Sleep(1);
    //     printf("Den day\n");
    //     ListenToServer(sockfd, server_addr, buffer);
    //     printf("Client kia gui %s \n", buffer);
    // } 
}



void *handleMess(void *argument){
    int choose = 0;
    int choose_1 = 0;
    int row;

	pthread_detach(pthread_self());
    int id;
	Arg *arg = (Arg *)argument;
    char *buffer = calloc(MAX_MESSAGE ,sizeof(char));
	// printf("[+]Server: %s!\n", arg->buffer);
	 while(1){ 
        memset(arg->buffer, 0, sizeof(*(arg->buffer)));
        ListenToServer(sockfd, server_addr, arg->buffer);
		printf("%s\n", arg->buffer);
        arg->type = GetType(arg->buffer);
        printf("%d\n", arg->type);
		if(arg->type == LOGIN_SUCCESS){  // dang nhap thanh cong
            GetCurrUser(arg->currUser); 
			token = GetToken(arg->buffer,4);
			printf("id cua %s la %d\n", token[3],atoi(token[2]));
            arg->currUser->id = atoi(token[2]);
            currUser->id = arg->currUser->id; // den day lay duoc thong tin user da dang nhap
            strcpy(arg->currUser->name, token[3]);
            pthread_create(&tid1, NULL, sendToPingServer, NULL); // tao thread send ping lien tuc
            cleanToken(token, 4);
            printf("chon :\n");
            scanf("%d", &choose);
            if (choose == 1)
            {   printf("ID hien tai la %d\n", currUser->id);
                host_game(arg->currUser);
                

                while (1){
                    printf("OK\n");
                    memset(buffer, 0, sizeof(*buffer));
                    ListenToServer(sockfd, server_addr, buffer);
                    token = GetToken(buffer, 3);
                    if (atoi(token[1]) == 2){
                        printf("%s join room, can start game\n", token[2]);
                    } 
                    if (atoi(token[1]) == 1){
                        printf("%s out room\n", token[2]);
                    }

                    printf("Chon start: \n");
                    scanf("%d", &choose_1);
                    if (choose_1 == 1){
                        start_game(arg->currUser);
                        in_game(arg->currUser);
                        break;
                    }else if (choose_1 == 2)
                    {
                        exit_room();

                    }
                    
                }

                
            }else if (choose == 2)
            {   
                GetRoomList(roomlist, token);
                fflush(stdin);
                printf("Chon join phong: \n");
                scanf("%d", &row);
                if (row == 0)
                {   
                    printf("Row = %d \n", row);
                    join_room(row,  roomlist , arg->currUser);
                    // fflush(stdin);
                    // scanf("%d", &choose_1);
                    // if (choose_1 == 1)
                    // {
                    //     exit_room();
                    // }
                    
                    in_game(arg->currUser);
                }
            }
            

        }
        else if (arg->type == LOGIN_FAIL) // dang nhap sai thi dang nhap lai den khi ok
        {   
            printf("Login fail ! Try again !\n");
            login_to_server();
        }
        
        
	 }

}

int main(){
	token = makeToken();
    roomlist = calloc(20, sizeof(Row));
    
	setupClient();
	login_to_server();
	// register_Acc();
    
	Arg *arg = calloc(1,sizeof(Arg));
	arg->buffer = calloc(100,sizeof(char));
    arg->currUser = calloc(1,sizeof(PlayerInfor));

    currUser = (PlayerInfor*)calloc(1, sizeof(PlayerInfor));
    currUser->name = (char*)calloc(MAX_NAME, sizeof(char));
    currUser->room = (char*)calloc(MAX_RNAME, sizeof(char));
    currUser->id = -1;
    currUser->isHost = -1;
    handleMess(arg);

  
	while(1);
    
	return 0;
}