//
//  Tcp_Client.h
//  webclient
//
//  Created by wu hao on 2020/6/6.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef _Tcp_Client_h
#define _Tcp_Client_h
#include<stdio.h>
#include<thread>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <memory.h>
#include "TcpMessage.h"
using namespace std;
class Tcp_Client{
private:
    int _socket;
public:
    //构造
    Tcp_Client():_socket(-1){}
    //析构
    virtual ~Tcp_Client(){
        if(_socket>0){
            close(_socket);
        }
    }
    bool isRun() const{
        if(get_socket()>0) return true;
        else return false;
    }
    int get_socket() const{
        return _socket;
    }
    void Set_Socket(const int new_socket){
        _socket = new_socket;
    }
    //创建socket
    void Create_Socket(){
        if(isRun()){
            //关闭旧连接
            close(_socket);
        }
        Set_Socket(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
    }
    //连接socket
    int Connect(const char *server_ip,const int port) {
        if(!isRun()){
            Create_Socket();
        }
        //设置echo服务器
        struct sockaddr_in echoserver;
        memset(&echoserver, 0, sizeof(echoserver));
        echoserver.sin_family = AF_INET;//使用以太网
        echoserver.sin_addr.s_addr = inet_addr(server_ip);//设置需要访问服务器的ip
        echoserver.sin_port = htons(port);//设置端口号 htons将端口号转为网络字节序
        //建立连接
        int ret = connect(get_socket(),(struct sockaddr *)&echoserver, sizeof(echoserver));
        if(ret<0){
            cout<<"建立连接失败"<<endl;
        }
        else{
            cout<<"建立连接成功"<<endl;
        }
        return ret;
    }
    //关闭连接
    void Close_Socket() {
        if(get_socket()>0){
            close(get_socket());
        }
        Set_Socket(-1);
    }
    //接收消息
    int RecvMessage() const{
        //设置接收缓冲区
        char szRecv[1024];
        int received_len = recv(get_socket(), szRecv, sizeof(header), 0);
        header* received_header = (header*) szRecv;
        //处理buffer
        if(received_len<=0){
            cout<<"服务端: "<<get_socket()<<" 退出"<<endl;
            return -1;
        }
        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        recv(get_socket(), szRecv+sizeof(header), received_header->length-sizeof(header), 0);
        OnNetMsg(received_header);
        return 0;
    }
    //响应网络数据
    virtual void OnNetMsg(const header* received_header) const{
        switch (received_header->cmd) {
            case CMD_LOGIN_RESULT:{
                LoginResult *received_login_result = (LoginResult*)received_header;
                cout<<"登陆是否成功"<<received_login_result->res<<"\n"<<endl;
            }
                break;
            case CMD_LOGOUT_RESULT:{
                LogoutResult *received_logout_result = (LogoutResult*)received_header;
                cout<<"登出是否成功"<<received_logout_result->res<<"\n"<<endl;
            }
                break;
            case CMD_NEWUSERJOIN:{
                NewUserJoin *received_newuser_join = (NewUserJoin*)received_header;
                cout<<"新用户加入，新用户的socket为"<<received_newuser_join->new_user_socket<<"\n"<<endl;
            }
                break;
            default:
                break;
        }
    }
    //发送消息
    int sendMsg(const header* sendheader) const{
        if(isRun()&&sendheader){
            return send(get_socket(), (char*)sendheader, sendheader->length, 0);
        }
        return -1;
    }
    // 处理消息
    bool onRun(){
        if(isRun()){
            //接收服务器端消息
            fd_set fd_Read;
            __DARWIN_FD_ZERO(&fd_Read);
            __DARWIN_FD_SET(_socket, &fd_Read);
            timeval time_val = {1,0};
            int res = select(_socket+1, &fd_Read, NULL, NULL, &time_val);
            if(res<0){
                cout<<"select 出错，返回-1，结束\n"<<endl;
                return false;
            }
            if(__DARWIN_FD_ISSET(_socket, &fd_Read)){
                __DARWIN_FD_CLR(_socket, &fd_Read);
                if(-1==RecvMessage()){
                    return false;
                }
            }
             return true;
        }
        return false;
    }
    
};

#endif /* Tcp_Client_h */
