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
struct DataPackage{
    int age;
    char name[BUFFERSIZE];
};
int main(int argc, const char * argv[]) {
    //    if(argc!=2){
    //        cout<<"参数错误，使用方法: echoserver <port>\n"<<endl;
    //        exit(1);
    //    }
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
        char receive_buffer[BUFFERSIZE];
        memset(receive_buffer,sizeof(receive_buffer),'\0');
        int received = recv(clientsock, receive_buffer, BUFFERSIZE, 0);
        
        //处理buffer
        if(received<=0){
            cout<<"客户端退出"<<endl;
            break;
        }
        else{
            receive_buffer[received] = '\0';
        }
        cout<<"接收的命令："<<receive_buffer<<endl;
        if(0==strcmp(receive_buffer, "getInfo")){
            DataPackage msg = {24,"吴浩"};
            send(clientsock, (char*)&msg, sizeof(DataPackage), 0);
        }
        else{
            char msg_buffer[] = "???";
            send(clientsock, msg_buffer, strlen(msg_buffer)+1, 0);
        }
    }
    
    close(server_sock);
    return 0;
}
