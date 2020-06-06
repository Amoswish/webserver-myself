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
    CMD_NEWUSERJOIN,
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
struct NewUserJoin:public header{
    NewUserJoin(){
        cmd = CMD_NEWUSERJOIN;
        length = 0;
    };
    int new_user_socket;
    int res;
};
#pragma pack()
int process(int serversock){
    //设置接收缓冲区
    char szRecv[1024];
    int received_len = recv(serversock, szRecv, sizeof(header), 0);
    header* received_header = (header*) szRecv;
    //处理buffer
    if(received_len<=0){
        cout<<"服务端: "<<serversock<<" 退出"<<endl;
        return -1;
    }
    cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
    switch (received_header->cmd) {
        case CMD_LOGIN_RESULT:{
            int login_received_body_len = recv(serversock,szRecv+sizeof(header), received_header->length-sizeof(header), 0);
            LoginResult *received_login_result = (LoginResult*)szRecv;
            cout<<"登陆是否成功"<<received_login_result->res<<"\n"<<endl;
        }
            break;
        case CMD_LOGOUT_RESULT:{
            int logout_received_body_len = recv(serversock,szRecv+sizeof(header), received_header->length-sizeof(header), 0);
            LogoutResult *received_logout_result = (LogoutResult*)szRecv;
            cout<<"登出是否成功"<<received_logout_result->res<<"\n"<<endl;
        }
            break;
        case CMD_NEWUSERJOIN:{
            int logout_received_body_len = recv(serversock,szRecv+sizeof(header), received_header->length-sizeof(header), 0);
            NewUserJoin *received_newuser_join = (NewUserJoin*)szRecv;
            cout<<"新用户加入，新用户的socket为"<<received_newuser_join->new_user_socket<<"\n"<<endl;
        }
            break;
        default:
            break;
    }
    return 0;
}
bool thread_exit = true;
void cmd_thread(int server_socket){
    while(1){
        char word[BUFFER_SIZE];
        memset(word,sizeof(word),'\0');
        cin>>word;
        if(strlen(word)==0) continue;
        if(0==strcmp(word, "over")) {
            cout<<"over"<<endl;
            thread_exit = false;
            break;
        }
        else{
            if(0==strcmp(word, "login")){
                LoginMsg send_login_msg;
                strcpy(send_login_msg.username,"wuhao");
                strcpy(send_login_msg.password,"123456");
                send_login_msg.cmd = CMD_LOGIN;
                send_login_msg.length = sizeof(LoginMsg);
                send(server_socket, (LoginMsg*)&send_login_msg, sizeof(LoginMsg), 0);
                LoginResult received_login_msg;
                int receivedmsg_len = recv(server_socket, (char*)&received_login_msg, sizeof(LoginResult), 0);
                cout<<"login:"<<received_login_msg.res;
            }
            else if(0==strcmp(word, "logout")){
                LogoutMsg send_logout_msg;
                strcpy(send_logout_msg.username,"wuhao");
                send_logout_msg.cmd = CMD_LOGOUT;
                send_logout_msg.length = sizeof(LogoutMsg);
                send(server_socket, (LogoutMsg*)&send_logout_msg, sizeof(LogoutMsg), 0);
                LogoutResult received_logout_msg = {};
                int receivedmsg_len = recv(server_socket, (char*)&received_logout_msg, sizeof(LogoutResult), 0);
                cout<<"logout:"<<received_logout_msg.res;
            }
            else{
                cout<<"没有此命令"<<endl;
            }
        }
    }
    return ;
}
int main(int argc, const char * argv[]) {
    int server_socket;
    char server_ip[] = "127.0.0.1";
    char port[] = "8080";
    server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket<0){
        cout<<"调用失败"<<endl;
        exit(1);
    }
    //设置echo服务器
    memset(&echoserver, 0, sizeof(echoserver));
    echoserver.sin_family = AF_INET;//使用以太网
    echoserver.sin_addr.s_addr = inet_addr(server_ip);//设置需要访问服务器的ip
    echoserver.sin_port = htons(atoi(port));//设置端口号 htons将端口号转为网络字节序
    //建立连接
    int newconnect = connect(server_socket,(struct sockaddr *)&echoserver, sizeof(echoserver));
    if(newconnect<0){
        cout<<"建立连接失败"<<endl;
        exit(1);
    }
    // 启动线程
    std::thread thread_1(cmd_thread,server_socket);
    while(thread_exit){
        //接收服务器端消息
        fd_set fd_Read;
        __DARWIN_FD_ZERO(&fd_Read);
        __DARWIN_FD_SET(server_socket, &fd_Read);
        timeval time_val = {1,0};
        int res = select(server_socket+1, &fd_Read, NULL, NULL, &time_val);
        if(res<0){
            cout<<"select 出错，返回-1，结束\n"<<endl;
            break;
        }
        if(__DARWIN_FD_ISSET(server_socket, &fd_Read)){
            __DARWIN_FD_CLR(server_socket, &fd_Read);
            if(-1==process(server_socket)){
                break;
            }
        }
//        cout<<"空闲时间处理其他请求"<<endl;
//        LoginMsg send_login_msg;
//        strcpy(send_login_msg.username,"wuhao");
//        strcpy(send_login_msg.password,"123456");
//        send_login_msg.cmd = CMD_LOGIN;
//        send_login_msg.length = sizeof(LoginMsg);
//        send(server_socket, (LoginMsg*)&send_login_msg, sizeof(LoginMsg), 0);
    }
//    while(1){
//        char word[BUFFER_SIZE];
//        memset(word,sizeof(word),'\0');
//        cin>>word;
//        if(strlen(word)==0) continue;
//        if(0==strcmp(word, "over")) {
//            cout<<"over"<<endl;
//            break;
//        }
//        else{
//            if(0==strcmp(word, "login")){
//                LoginMsg send_login_msg;
//                strcpy(send_login_msg.username,"wuhao");
//                strcpy(send_login_msg.password,"123456");
//                send_login_msg.cmd = CMD_LOGIN;
//                send_login_msg.length = sizeof(LoginMsg);
//                send(server_socket, (LoginMsg*)&send_login_msg, sizeof(LoginMsg), 0);
//                LoginResult received_login_msg;
//                int receivedmsg_len = recv(server_socket, (char*)&received_login_msg, sizeof(LoginResult), 0);
//                cout<<"login:"<<received_login_msg.res;
//            }
//            else if(0==strcmp(word, "logout")){
//                LogoutMsg send_logout_msg;
//                strcpy(send_logout_msg.username,"wuhao");
//                send_logout_msg.cmd = CMD_LOGOUT;
//                send_logout_msg.length = sizeof(LogoutMsg);
//                send(server_socket, (LogoutMsg*)&send_logout_msg, sizeof(LogoutMsg), 0);
//                LogoutResult received_logout_msg = {};
//                int receivedmsg_len = recv(server_socket, (char*)&received_logout_msg, sizeof(LogoutResult), 0);
//                cout<<"logout:"<<received_logout_msg.res;
//            }
//            else{
//                cout<<"没有此命令"<<endl;
//            }
//        }
//
//    }
    //关闭连接
    close(server_socket);
    exit(0);
    return 0;
}

