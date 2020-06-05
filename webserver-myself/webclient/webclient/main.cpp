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
#define BUFFERSIZE 32
using namespace std;
int BUFFER_SIZE = 32;
struct sockaddr_in echoserver;
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
    int sock;
    char server_ip[] = "127.0.0.1";
    
    char port[] = "8080";
//    if(argc!=4){
//        cout<<"参数错误，请按以下格式输入：TCPecho <server_ip> <word> <port>\n"<<endl;
//        exit(1);
//    }
    sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock<0){
        cout<<"调用失败"<<endl;
        exit(1);
    }
    //设置echo服务器
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;//使用以太网
    echoserver.sin_addr.s_addr = inet_addr(server_ip);//设置需要访问服务器的ip
    echoserver.sin_port = htons(atoi(port));//设置端口号 htons将端口号转为网络字节序
    //建立连接
    int newconnect = connect(sock,(struct sockaddr *)&echoserver, sizeof(echoserver));
    if(newconnect<0){
        cout<<"建立连接失败"<<endl;
        exit(1);
    }
   
    while(1){
        char word[BUFFER_SIZE];
        memset(word,sizeof(word),'\0');
        cin>>word;
        if(strlen(word)==0) continue;
        if(0==strcmp(word, "over")) {
            cout<<"over"<<endl;
            break;
        }
        else{
            if(0==strcmp(word, "login")){
                LoginMsg send_login_msg;
                strcpy(send_login_msg.username,"wuhao");
                strcpy(send_login_msg.password,"123456");
                send_login_msg.cmd = CMD_LOGIN;
                send_login_msg.length = sizeof(LoginMsg);
                send(sock, (LoginMsg*)&send_login_msg, sizeof(LoginMsg), 0);
                LoginResult received_login_msg;
                int receivedmsg_len = recv(sock, (char*)&received_login_msg, sizeof(LoginResult), 0);
                cout<<"login:"<<received_login_msg.res;
            }
            else if(0==strcmp(word, "logout")){
                LogoutMsg send_logout_msg;
                strcpy(send_logout_msg.username,"wuhao");
                send_logout_msg.cmd = CMD_LOGOUT;
                send_logout_msg.length = sizeof(LogoutMsg);
                send(sock, (LogoutMsg*)&send_logout_msg, sizeof(LogoutMsg), 0);
                LogoutResult received_logout_msg = {};
                int receivedmsg_len = recv(sock, (char*)&received_logout_msg, sizeof(LogoutResult), 0);
                cout<<"logout:"<<received_logout_msg.res;
            }
            else{
                cout<<"没有此命令"<<endl;
            }
        }
        
    }
    //关闭连接
    close(sock);
    exit(0);
    return 0;
}

