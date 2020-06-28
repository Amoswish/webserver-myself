//
//  TcpMessage.h
//  webserver
//
//  Created by wu hao on 2020/6/7.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef TcpMessage_h
#define TcpMessage_h
#define arr_length 1024
//static int length = 32;
#include<iostream>
#include "MemoryMgr.h"
#pragma pack(1)
enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEWUSERJOIN,
    CMD_C2S_HEART,
    CMD_S2C_HEART,
    ERROR
};
struct NetMsg_Header{
    int length;
    CMD cmd;
    NetMsg_Header():length(sizeof(NetMsg_Header)),cmd(ERROR){}
    void* operator new(size_t size){
        //std::cout<<"new_size:"<<size<<std::endl;
        return MemoryMgr::getInstance().allocMem(size);
    }
    void* operator new[](size_t size){
        //std::cout<<"new[]_size:"<<size<<std::endl;
        return MemoryMgr::getInstance().allocMem(size);
    }
    void operator delete(void* p,size_t size){
        MemoryMgr::getInstance().freeMem(p,size);
    }
    void operator delete[](void* p,size_t size){
        MemoryMgr::getInstance().freeMem(p,size);
    }
};
struct NetMsg_LoginMsg:public NetMsg_Header{
    NetMsg_LoginMsg(){
        cmd = CMD_LOGIN;
        length = sizeof(NetMsg_LoginMsg);
    };
    char username[arr_length];
    char password[arr_length];
};
struct NetMsg_C2S_Heart:public NetMsg_Header{
    NetMsg_C2S_Heart(){
        cmd = CMD_C2S_HEART;
        length = sizeof(NetMsg_LoginMsg);
    };
};
struct NetMsg_S2C_Heart:public NetMsg_Header{
    NetMsg_S2C_Heart(){
        cmd = CMD_S2C_HEART;
        length = sizeof(NetMsg_LoginMsg);
    };
};
struct NetMsg_LoginResult:public NetMsg_Header{
    NetMsg_LoginResult(){
        cmd = CMD_LOGIN_RESULT;
        length = sizeof(NetMsg_LoginResult);
    };
    int res;
};
struct NetMsg_LogoutMsg:public NetMsg_Header{
    NetMsg_LogoutMsg(){
        cmd = CMD_LOGOUT;
        length = 0;
    };
    char username[arr_length];
};
struct NetMsg_LogoutResult:public NetMsg_Header{
    NetMsg_LogoutResult(){
        cmd = CMD_LOGOUT_RESULT;
        length = sizeof(NetMsg_LogoutResult);
    };
    int res;
};
struct NetMsg_NewUserJoin:public NetMsg_Header{
    NetMsg_NewUserJoin(){
        cmd = CMD_NEWUSERJOIN;
        length = sizeof(NetMsg_NewUserJoin);
    };
    int new_user_socket;
    int res;
};
#pragma pack()
#endif /* TcpMessage_h */
