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
using namespace std;
int BUFFER_SIZE = 32;
struct sockaddr_in echoserver;
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
            //发送数据
            int echolen = strlen(word);
            cout<<echolen<<endl;
            if(send(sock, word, echolen, 0)!=echolen){
                cout<<"发送不匹配"<<endl;
                exit(1);
            }
            //接收数据
            cout<<"接收的数据为：\n"<<endl;
            int havereceivedlen = 0;
            char buffer[BUFFER_SIZE];
            int receivedbytes = recv(sock, buffer, BUFFER_SIZE, 0);
            if(receivedbytes<0){
                cout<<"接收失败"<<endl;
                exit(1);
            }
            cout<<buffer<<endl;
        }
        
    }
    //关闭连接
    close(sock);
    exit(0);
    return 0;
}

