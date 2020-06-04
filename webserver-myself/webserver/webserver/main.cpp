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
enum CMD{
    LOGIN,
    LOGOUT,
    ERROR
};
struct header{
    int length;
    CMD cmd;
};
struct LoginBody{
    char username[BUFFERSIZE];
    char password[BUFFERSIZE];
};
struct LoginResult{
    int res;
};
struct LogoutBody{
    char username[BUFFERSIZE];
};
struct LogoutResult{
    int res;
};
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
        header receiveheader = {};
        int received_header = recv(clientsock, (header*)&receiveheader, sizeof(header), 0);
        
        //处理buffer
        if(received_header<=0){
            cout<<"客户端退出"<<endl;
            break;
        }
//        else{
//            receive_buffer[received] = '\0';
//        }
        cout<<"接收的命令："<<receiveheader.cmd<<"接收的长度："<<receiveheader.length<<endl;
        switch (receiveheader.cmd) {
            case LOGIN:{
                LoginBody received_loginBody = {};
                int login_received_body_len = recv(clientsock, (LoginBody*)&received_loginBody, sizeof(LoginBody), 0);
                cout<<"username:"<<received_loginBody.username<<" password"<<received_loginBody.password<<endl;
                //判断登陆的信息
                //登陆成功
                LoginResult login_result= {1};
                header send_header = {};
                send_header.cmd = receiveheader.cmd;
                send_header.length = sizeof(login_result);
                send(clientsock, (header*)&send_header, sizeof(header), 0);
                send(clientsock, (LoginResult*)&login_result, sizeof(LoginResult), 0);
                }
                break;
            case LOGOUT:{
                LogoutBody received_logoutBody = {};
                int logout_received_body_len = recv(clientsock, (LogoutBody*)&received_logoutBody, sizeof(LogoutBody), 0);
                cout<<"username:"<<received_logoutBody.username<<endl;
                //判断登出的信息
                //登出成功
                LogoutResult logout_result= {1};
                header send_header = {};
                send_header.cmd = receiveheader.cmd;
                send_header.length = sizeof(logout_result);
                send(clientsock, (header*)&send_header, sizeof(header), 0);
                send(clientsock, (LogoutResult*)&logout_result, sizeof(LogoutResult), 0);
                }
                break;
            default:
                //错误情况
                receiveheader.cmd = ERROR;
                send(clientsock, (header*)&receiveheader, sizeof(header), 0);
                break;
        }
    }
    
    close(server_sock);
    return 0;
}
