//
//  main.cpp
//  webserver
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
using namespace std;
#define MAXCONNECT 5
#define BUFFERSIZE 32
struct sockaddr_in echoserver,echoclient;
#pragma pack(1)
enum CMD{
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
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
    char username[BUFFERSIZE];
    char password[BUFFERSIZE];
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
    char username[BUFFERSIZE];
};
struct LogoutResult:public header{
    LogoutResult(){
        cmd = CMD_LOGOUT_RESULT;
        length = 0;
    };
    int res;
};
#pragma pack()
int main(int argc, const char * argv[]) {
    int port = 8080;
    //创建服务器socket
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;//使用以太网
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);//设置server可接受任何ip htonl 将网络字节序转成本机字节序
    echoserver.sin_port = htons(port);//设置端口号 htons将端口号转为网络字节序
    //绑定服务器socket
    int server_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);;
    int isbind = ::bind(server_sock, (struct sockaddr *) &echoserver,sizeof(echoserver));
    if(isbind<0){
        cout<<"绑定失败:"<<isbind<<endl;
        exit(1);
    }
    //开始监听
    int islisten = listen(server_sock, MAXCONNECT);
    if(islisten<0){
        cout<<"监听失败"<<endl;
    }
    //处理请求
    //accept 处理请求
    unsigned int echoclient_len = sizeof(echoclient);
    int clientsock = accept(server_sock, (struct sockaddr *) &echoclient,&echoclient_len);
    if(clientsock<0){
        cout<<"accept失败"<<endl;
    }
    cout<<"客户端地址"<<inet_ntoa(echoclient.sin_addr)<<endl;
    while(1){
        char szRecv[1024];
        int received_len = recv(clientsock, szRecv, sizeof(header), 0);
        header* received_header = (header*) szRecv;
        //处理buffer
        if(received_len<=0){
            cout<<"客户端退出"<<endl;
            break;
        }
//        else{
//            receive_buffer[received] = '\0';
//        }
        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        switch (received_header->cmd) {
            case CMD_LOGIN:{
                int login_received_body_len = recv(clientsock,szRecv+sizeof(header), received_header->length-sizeof(header), 0);
                LoginMsg *received_loginMsg = (LoginMsg*)szRecv;
//                printf("username:%s,password:%s",received_loginMsg.username,received_loginMsg.password);
                cout<<"username:"<<received_loginMsg->username<<" password"<<received_loginMsg->password<<endl;
                //判断登陆的信息
                //登陆成功
                LoginResult login_result;
                login_result.res = 1;
                login_result.cmd = CMD_LOGIN_RESULT;
                login_result.length = sizeof(LoginResult);
                send(clientsock, (char*)&login_result, sizeof(LoginResult), 0);
                }
                break;
            case CMD_LOGOUT:{
                int logout_received_body_len = recv(clientsock,szRecv+sizeof(header), received_header->length-sizeof(header), 0);
                LogoutMsg *received_logoutMsg = (LogoutMsg*)szRecv;
                cout<<"username:"<<received_logoutMsg->username<<endl;
                //判断登出的信息
                //登出成功
                LogoutResult logout_result;
                logout_result.res = 1;
                logout_result.cmd = CMD_LOGOUT_RESULT;
                logout_result.length = sizeof(LogoutResult);
                send(clientsock, (char*)&logout_result, sizeof(LogoutResult), 0);
                }
                break;
            default:{
                //错误情况
                header errorheader;
                errorheader.cmd = ERROR;
                send(clientsock, (header*)&errorheader, sizeof(header), 0);
                }
                break;
        }
    }
    
    close(server_sock);
    return 0;
}
