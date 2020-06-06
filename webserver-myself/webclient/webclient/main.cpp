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
#include "Tcp_Client.h"
#include "TcpMessage.h"
#define BUFFERSIZE 32
using namespace std;
int BUFFER_SIZE = 32;
struct sockaddr_in echoserver;

void cmd_thread(Tcp_Client* client){
    while(1){
        char word[BUFFER_SIZE];
        memset(word,sizeof(word),'\0');
        cin>>word;
        if(strlen(word)==0) continue;
        if(0==strcmp(word, "over")) {
            cout<<"over"<<endl;
            client->Close_Socket();
            break;
        }
        else{
            if(0==strcmp(word, "login")){
                LoginMsg send_login_msg;
                strcpy(send_login_msg.username,"wuhao");
                strcpy(send_login_msg.password,"123456");
                send_login_msg.cmd = CMD_LOGIN;
                send_login_msg.length = sizeof(LoginMsg);
                client->sendMsg(&send_login_msg);
            }
            else if(0==strcmp(word, "logout")){
                LogoutMsg send_logout_msg;
                strcpy(send_logout_msg.username,"wuhao");
                send_logout_msg.cmd = CMD_LOGOUT;
                send_logout_msg.length = sizeof(LogoutMsg);
                client->sendMsg(&send_logout_msg);
            }
            else{
                cout<<"没有此命令"<<endl;
            }
        }
    }
    return ;
}
int main(int argc, const char * argv[]) {
    Tcp_Client client;
    client.Connect("127.0.0.1", 8080);
    // 启动线程
    std::thread thread_1(cmd_thread,&client);
    thread_1.detach();
    while(client.isRun()){
        //接收服务器端消息
        client.onRun();
    }
    //关闭连接
    client.Close_Socket();
    exit(0);
    return 0;
}

