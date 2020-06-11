//
//  main.cpp
//  webserver
//
//  Created by wu hao on 2020/6/4.
//  Copyright © 2020 wu hao. All rights reserved.
//
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
    virtual void onNetJoin(ClientSocket* pClient){
        cout<<"加入客户端sock:"<<pClient->getSocket()<<endl;
        _clientCount++;
    };
    virtual void onNetLeave(ClientSocket* pClient){
        cout<<"客户端: "<<pClient->getSocket()<<" 退出"<<endl;
        _clientCount--;
    };
    virtual void onNetMsg(ClientSocket* pClient,const header* received_header){
        _recvCount++;
        switch (received_header->cmd) {
            case CMD_LOGIN:{
                LoginMsg *received_loginMsg = (LoginMsg*)received_header;
                //                cout<<"username:"<<received_loginMsg->username<<" password"<<received_loginMsg->password<<endl;
                //判断登陆的信息
                //登陆成功
                LoginResult login_result;
                login_result.res = 1;
                login_result.cmd = CMD_LOGIN_RESULT;
                login_result.length = sizeof(LoginResult);
                pClient->sendMsg(&login_result);
            }
                break;
            case CMD_LOGOUT:{
                LogoutMsg *received_logoutMsg = (LogoutMsg*)received_header;
                //                cout<<"username:"<<received_logoutMsg->username<<endl;
                //判断登出的信息
                //登出成功
                LogoutResult logout_result;
                logout_result.res = 1;
                logout_result.cmd = CMD_LOGOUT_RESULT;
                logout_result.length = sizeof(LogoutResult);
                pClient->sendMsg(&logout_result);
            }
                break;
            default:{
                //错误情况
                header errorheader;
                errorheader.cmd = ERROR;
                errorheader.length = sizeof(errorheader);
                pClient->sendMsg(&errorheader);
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
    tcp_server.Bind(NULL, 9080);
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
