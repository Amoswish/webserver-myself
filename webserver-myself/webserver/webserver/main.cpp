//
//  main.cpp
//  webserver
//
//  Created by wu hao on 2020/6/4.
//  Copyright © 2020 wu hao. All rights reserved.
//
#include "Allocotr.cpp"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include "Tcp_Server.h"
#include "TcpMessage.h"
#include "CELLTimestamp.h"
using namespace std;
#define MAXCONNECT 1024
#define BUFFERSIZE 32
struct sockaddr_in echoserver;
class MyServer:public Tcp_Server{
public:
    void* operator new(size_t size){
        return MemoryMgr::getInstance().allocMem(size);
    }
    void* operator new[](size_t size){
        return MemoryMgr::getInstance().allocMem(size);
    }
    void operator delete(void* p,size_t size){
        MemoryMgr::getInstance().freeMem(p,size);
    }
    void operator delete[](void* p,size_t size){
        MemoryMgr::getInstance().freeMem(p,size);
    }
    virtual void onNetJoin(ClientSocket* pClient){
        Tcp_Server::onNetJoin(pClient);
    };
    virtual void onNetLeave(ClientSocket* pClient){
        Tcp_Server::onNetLeave(pClient);
    };
    virtual void onNetRecv(ClientSocket* pClient){
        Tcp_Server::onNetRecv(pClient);
    };
    virtual void onNetMsg(CellServer* pCellServer,ClientSocket* pClient,const header* received_header){
        Tcp_Server::onNetMsg(pCellServer,pClient,received_header);
        switch (received_header->cmd) {
            case CMD_LOGIN:{
                LoginMsg *received_loginMsg = (LoginMsg*)received_header;
                //                cout<<"username:"<<received_loginMsg->username<<" password"<<received_loginMsg->password<<endl;
                //判断登陆的信息
                //登陆成功
                LoginResult *login_result = new LoginResult;
                login_result->res = 1;
                login_result->cmd = CMD_LOGIN_RESULT;
                login_result->length = sizeof(LoginResult);
                pCellServer->addSendTask(pClient,login_result);
                //pClient->sendMsg(&login_result);
            }
                break;
            case CMD_LOGOUT:{
                LogoutMsg *received_logoutMsg = (LogoutMsg*)received_header;
//                                cout<<"username:"<<received_logoutMsg->username<<endl;
                //判断登出的信息
                //登出成功
                LogoutResult *res = new LogoutResult;
                res->res = 1;
                res->cmd = CMD_LOGOUT_RESULT;
                res->length = sizeof(LogoutResult);
                pCellServer->addSendTask(pClient,res);
//                pClient->sendMsg(&logout_result);
            }
                break;
            default:{
                //错误情况
                header *res = new header;
                res->cmd = ERROR;
                res->length = sizeof(res);
                pCellServer->addSendTask(pClient,res);
//                pClient->sendMsg(&errorheader);
            }
                break;
        }
    }
};
int main(int argc, const char * argv[]) {
    //创建服务器socket
    MyServer tcp_server;
    tcp_server.InitSocket();
    //绑定服务器socket
    tcp_server.Bind(NULL, 9081);
    //开始监听
    tcp_server.Listen(MAXCONNECT);
    //处理请求
    tcp_server.Start(4);
    while(tcp_server.isRun()){
        tcp_server.onRun();
    }
    //关闭服务器
    tcp_server.Close_Socket();
    return 0;
}
