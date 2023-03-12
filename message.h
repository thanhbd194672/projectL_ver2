#include "define.h"
enum mess_type{
    REGISTER_GAME = 0, //dang ki tai khoan
    LOGIN_GAME = 1, //tin hieu dang nhap
    EXIT_GAME = 2, // tin hieu thoat game
    HOST_GAME = 3, // tao phong
    FIND_ROOM = 4, // tim ra nhung phong co the tham gia
    JOIN_ROOM = 5,  // tin hieu tham gia mot phong game
    READY = 6, // tin hieu san sang bat dau chien dau
    ROOM_INFO = 7, // thong tin phong game
    SUCCEED_PACK = 8, // tra lai thong bao thanh cong
    ERROR_PACK = 9,// tra lai thong bao loi
    START_GAME = 10, // bat dau game dau
    PLAYER_TURN = 11,
    ROLE = 12,
    LOGIN_FAIL = 13,
    LOGIN_SUCCESS = 14,
    IN_GAME = 15,
    EXIT_ROOM = 16,
    OUT_ROOM =17,
};

enum rank{
    IRON = 0,
    BRONZE = 1,
    SILVER = 2,
    GOLD = 3,
    PLATINUM = 4,
    DIAMOND = 5,
};

typedef struct _PlayerInfor{
    char *name;
    int id;
    char *room;
    int isHost;
    int role;
}PlayerInfor;

enum mess_type GetType(char *message);

char *MakeMessage(char **token, int tokenSize, enum mess_type type);

char **GetToken(char *message, int tokenSize);

char **makeToken();

void cleanToken(char **token, int num);