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
#define recv_buffer_size 10240
class Tcp_Client{
private:
    int _socket;
    //设置接收缓冲区
    char _szRecv[recv_buffer_size] ;
    //设置二级缓冲区解决粘包、少包问题
    char _szRecvMsg[recv_buffer_size*5];
    int _lastPos;
    bool _isConnect;
public:
    //构造
    Tcp_Client():_socket(-1),_lastPos(0){}
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
    bool Connect(const char *server_ip,const int port) {
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
            _isConnect = false;
        }
        else{
            cout<<"建立连接成功"<<endl;
        }
        _isConnect = true;
        return _isConnect;
    }
    //关闭连接
    void Close_Socket() {
        if(get_socket()>0){
            close(get_socket());
        }
        Set_Socket(-1);
        _isConnect = false;
    }
    //接收消息
    int RecvMessage() {
        int received_len = (int)recv(get_socket(), _szRecv, recv_buffer_size, 0);
        //处理buffer
        if(received_len<=0){
            cout<<"服务端: "<<get_socket()<<" 退出"<<endl;
            return -1;
        }
        memcpy(_szRecvMsg+_lastPos, _szRecv, received_len);
        _lastPos+=received_len;
        while(_lastPos>=sizeof(NetMsg_Header)){
            NetMsg_Header* received_header = (NetMsg_Header*) _szRecvMsg;
            if(_lastPos>=received_header->length){
                int n_size = _lastPos-received_header->length;
//                cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
                OnNetMsg(received_header);
                memcpy(_szRecvMsg, _szRecvMsg+received_header->length, n_size);
                _lastPos = n_size;
            }
            else break ;
        }
        
        return 0;
    }
    //响应网络数据
    virtual void OnNetMsg(const NetMsg_Header* received_header) const{
        switch (received_header->cmd) {
            case CMD_LOGIN_RESULT:{
                NetMsg_LoginResult *received_login_result = (NetMsg_LoginResult*)received_header;
//                cout<<"登陆是否成功"<<received_login_result->res<<"\n"<<endl;
            }
                break;
            case CMD_LOGOUT_RESULT:{
                NetMsg_LogoutResult *received_logout_result = (NetMsg_LogoutResult*)received_header;
//                cout<<"登出是否成功"<<received_logout_result->res<<"\n"<<endl;
            }
                break;
            case CMD_NEWUSERJOIN:{
                NetMsg_NewUserJoin *received_newuser_join = (NetMsg_NewUserJoin*)received_header;
//                cout<<"新用户加入，新用户的socket为"<<received_newuser_join->new_user_socket<<"\n"<<endl;
            }
                break;
            case ERROR:{
                cout<<"收到错误消息，"<<received_header->cmd<<"消息长度："<<received_header->length<<"\n"<<endl;
            }
                break;
            default:{
                cout<<"收到未定义消息，"<<received_header->cmd<<"消息长度："<<received_header->length<<"\n"<<endl;
            }
                break;
        }
    }
    //发送消息
    int sendMsg(const NetMsg_Header* sendheader) const{
        if(isRun()&&sendheader&&_isConnect){
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
            timeval time_val = {0,0};
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
