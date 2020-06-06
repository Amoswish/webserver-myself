//
//  TcpMessage.h
//  webclient
//
//  Created by wu hao on 2020/6/6.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef TcpMessage_h
#define TcpMessage_h
#define arr_length 32
//static int length = 32;
#pragma pack(1)
enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEWUSERJOIN,
    ERROR
};
struct header{
    int length;
    CMD cmd;
};
struct LoginMsg:public header{
    LoginMsg(){
        cmd = CMD_LOGIN;
        length = 0;
    };
    char username[arr_length];
    char password[arr_length];
};
struct LoginResult:public header{
    LoginResult(){
        cmd = CMD_LOGIN_RESULT;
        length = 0;
    };
    int res;
};
struct LogoutMsg:public header{
    LogoutMsg(){
        cmd = CMD_LOGOUT;
        length = 0;
    };
    char username[arr_length];
};
struct LogoutResult:public header{
    LogoutResult(){
        cmd = CMD_LOGOUT_RESULT;
        length = 0;
    };
    int res;
};
struct NewUserJoin:public header{
    NewUserJoin(){
        cmd = CMD_NEWUSERJOIN;
        length = 0;
    };
    int new_user_socket;
    int res;
};
#pragma pack()

#endif /* TcpMessage_h */
