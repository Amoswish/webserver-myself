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
struct DataPackage{
    int age;
    char name[BUFFERSIZE];
};
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
                header send_header = {};
                LoginBody send_login_body = {};
                strcpy(send_login_body.username,"wuhao");
                strcpy(send_login_body.password,"123456");
                send_header.cmd = LOGIN;
                send_header.length = sizeof(send_login_body);
                send(sock, (header*)&send_header, sizeof(header), 0);
                send(sock, (LoginBody*)&send_login_body, sizeof(LoginBody), 0);
                header received_header = {};
                LoginResult received_login_body = {};
                int receivedheader_len = recv(sock, (header*)&received_header, sizeof(header), 0);
                int receivedbody_len = recv(sock, (LoginResult*)&received_login_body, sizeof(LoginResult), 0);
                cout<<"login:"<<received_login_body.res;
            }
            else if(0==strcmp(word, "logout")){
                header send_header = {};
                LogoutBody send_logout_body = {};
                strcpy(send_logout_body.username,"wuhao");
                send_header.cmd = LOGOUT;
                send_header.length = sizeof(send_logout_body);
                send(sock, (header*)&send_header, sizeof(header), 0);
                send(sock, (LogoutBody*)&send_logout_body, sizeof(LogoutBody), 0);
                header received_header = {};
                LogoutResult received_logout_body = {};
                int receivedheader_len = recv(sock, (header*)&received_header, sizeof(header), 0);
                int receivedbody_len = recv(sock, (LogoutResult*)&received_logout_body, sizeof(LogoutResult), 0);
                cout<<"logout:"<<received_logout_body.res;
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

