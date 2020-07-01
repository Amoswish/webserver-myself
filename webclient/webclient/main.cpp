//
//  main.cpp
//  webclient
//
//  Created by wu hao on 2020/6/4.
//  Copyright © 2020 wu hao. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include <thread>
#include <vector>
#include "Tcp_Client.h"
#include "TcpMessage.h"
#include "CELLTimestamp.h"
using namespace std;
int gbrun = 1;
void cmd_thread(Tcp_Client* client){
    while(1){
        char word[1024];
        cin>>word;
        if(strlen(word)==0) continue;
        if(0==strcmp(word, "over")) {
            cout<<"over"<<endl;
            gbrun = 0;
            break;
        }
        else{
            if(0==strcmp(word, "login")){
                NetMsg_LoginMsg send_login_msg;
                strcpy(send_login_msg.username,"wuhao");
                strcpy(send_login_msg.password,"123456");
                send_login_msg.cmd = CMD_LOGIN;
                send_login_msg.length = sizeof(NetMsg_LoginMsg);
                client->sendMsg(&send_login_msg);
            }
            else if(0==strcmp(word, "logout")){
                NetMsg_LogoutMsg send_logout_msg;
                strcpy(send_logout_msg.username,"wuhao");
                send_logout_msg.cmd = CMD_LOGOUT;
                send_logout_msg.length = sizeof(NetMsg_LogoutMsg);
                client->sendMsg(&send_logout_msg);
            }
            else{
                cout<<"没有此命令"<<endl;
            }
        }
    }
    return ;
}
int clientCount = 100;
int threadCount = 4;
vector<Tcp_Client*> clients(clientCount*threadCount);
atomic_int sendCount(0);
atomic_int readyCount(0);
void recvThread(int start,int end){
    while(gbrun){
        for(int i = start;i<end;i++){
            clients[i]->onRun();
        }
    }
}
void sendThread(int index){
    int start = (index-1)*clientCount;
    int end = (index)*clientCount;
    for(int i = start;i<end;i++){
        clients[i] = new Tcp_Client;
        clients[i]->Connect("127.0.0.1", 9081);
    }
    
    readyCount++;
    
    while(readyCount<threadCount){
        std::chrono::milliseconds t(10);
        std::this_thread::sleep_for(t);
    }
    //启动接收线程
    std::thread thread_1(recvThread,start,end);
    thread_1.detach();
    //
    NetMsg_LogoutMsg send_logout_msg;
    strcpy(send_logout_msg.username,"wuhao");
    send_logout_msg.cmd = CMD_LOGOUT;
    send_logout_msg.length = sizeof(NetMsg_LogoutMsg);
    while(gbrun){
        for(int i = start;i<end;i++){
            if(-1!=clients[i]->sendMsg(&send_logout_msg)){
                sendCount++;
//                std::chrono::milliseconds t(100);
//                std::this_thread::sleep_for(t);
            }
        }
    }
    //关闭连接
    for(int i = start;i<end;i++){
        clients[i]->Close_Socket();
        delete clients[i];
    }
}
int main(int argc, const char * argv[]) {
//     启动UI线程
//        std::thread thread_ui(cmd_thread,&client);
//        thread_ui.detach();

    //启动发送线程
    for(int i = 1;i<=threadCount;i++){
            std::thread thread_1(sendThread,i);
            thread_1.detach();
    }
    CELLTimestamp tTime;
    while(gbrun){
        auto t = tTime.getElapsedSecond();
        if(t>1.0){
            cout<<"thread:"<<threadCount<<"clients"<<clientCount<<"time"<<t<<"send"<<sendCount<<endl;
            sendCount = 0;
            tTime.update();
        }
        sleep(1);
    }
    return 0;
}

