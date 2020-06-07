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
using namespace std;
#define MAXCONNECT 5
#define BUFFERSIZE 32
struct sockaddr_in echoserver;
int main(int argc, const char * argv[]) {
    //创建服务器socket
    Tcp_Server tcp_server;
    tcp_server.InitSocket();
    //绑定服务器socket
    tcp_server.Bind(NULL, 8080);
    //开始监听
    tcp_server.Listen(MAXCONNECT);
    //处理请求
    while(tcp_server.isRun()){
        tcp_server.onRun();
    }
    //关闭服务器
    tcp_server.Close_Socket();
    return 0;
}
