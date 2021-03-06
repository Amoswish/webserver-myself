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
#include "MemoryMgr.h"
using namespace std;
#define MAXCONNECT 1024
#define BUFFERSIZE 32
struct sockaddr_in echoserver;

class MyServer:public Tcp_Server{
public:
    virtual void onNetJoin(CellClient* pClient){
        Tcp_Server::onNetJoin(pClient);
    };
    virtual void onNetLeave(CellClient* pClient){
        Tcp_Server::onNetLeave(pClient);
    };
    virtual void onNetRecv(CellClient* pClient){
        Tcp_Server::onNetRecv(pClient);
    };
    virtual void onNetMsg(CellServer* pCellServer,CellClient* pClient,const NetMsg_Header* received_header){
        Tcp_Server::onNetMsg(pCellServer,pClient,received_header);
        switch (received_header->cmd) {
            case CMD_LOGIN:{
                pClient->resetDTHeart();
                NetMsg_LoginMsg *received_loginMsg = (NetMsg_LoginMsg*)received_header;
                //                cout<<"username:"<<received_loginMsg->username<<" password"<<received_loginMsg->password<<endl;
                //判断登陆的信息
                //登陆成功
                NetMsg_LoginResult *res = new NetMsg_LoginResult;
                res->res = 1;
                if(0 >= pClient->sendMsg(res)){
                    //消息发送失败
                    cout<<"消息发送失败"<<endl;
                }
//                pCellServer->addSendTask(pClient,res);
            }
                break;
            case CMD_LOGOUT:{
                pClient->resetDTHeart();
                NetMsg_LogoutMsg *received_logoutMsg = (NetMsg_LogoutMsg*)received_header;
//                                cout<<"username:"<<received_logoutMsg->username<<endl;
                //判断登出的信息
                //登出成功
                NetMsg_LogoutResult *res = new NetMsg_LogoutResult;
                res->res = 1;
                if(0 >= pClient->sendMsg(res)){
                    //消息发送失败
                    cout<<"消息发送失败"<<endl;
                }
//                pCellServer->addSendTask(pClient,res);
            }
                break;
            case CMD_C2S_HEART:{
                pClient->resetDTHeart();
                NetMsg_S2C_Heart *res = new NetMsg_S2C_Heart;
                if(0 >= pClient->sendMsg(res)){
                    //消息发送失败
                    cout<<"消息发送失败"<<endl;
                }
//                pCellServer->addSendTask(pClient,res);
            }
                break;
            default:{
                //错误情况
                NetMsg_Header *res = new NetMsg_Header;
                res->cmd = ERROR;
                res->length = sizeof(res);
                if(0 >= pClient->sendMsg(res)){
                    //消息发送失败
                    cout<<"消息发送失败"<<endl;
                }
//                pCellServer->addSendTask(pClient,res);
            }
                break;
        }
    }
};
void cmd_thread(Tcp_Server* myServer){
    while(1){
        char word[1024];
        cin>>word;
        if(strlen(word)==0) continue;
        if(0==strcmp(word, "over")) {
            cout<<"over"<<endl;
            myServer->Close_Socket();
            break;
        }
    }
}
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
    //启动UI线程
    std::thread thread_ui(cmd_thread,&tcp_server);
    thread_ui.detach();
    //关闭服务器
    while(1){}
    auto t = std::chrono::milliseconds(10);
    std::this_thread::sleep_for(t);
    return 0;
}
