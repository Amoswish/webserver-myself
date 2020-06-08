//
//  Tcp_Server.h
//  webserver
//
//  Created by wu hao on 2020/6/7.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef Tcp_Server_h
#define Tcp_Server_h
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
#define recv_buffer_size 1024
using namespace std;
class ClientSocket{
private:
    int _socket;
    char _recvMsg[recv_buffer_size*10];
    int _lastPos;
public:
    ClientSocket(int socket = -1):_socket(socket),_lastPos(0){
        memset(_recvMsg,'\0',sizeof(_recvMsg));
    }
    int getSocket() const{
        return _socket;
    }
    void setSocket(const int socket) {
        _socket = socket;
    }
    int getLastPos() const{
        return _lastPos;
    }
    void setLastPos(const int lastPos){
        _lastPos = lastPos;
    }
    char* getRecvMsg(){
        return _recvMsg;
    }
};
class Tcp_Server{
private:
    int _serversocket;
    vector<ClientSocket*> _clients;
public:
    Tcp_Server():_serversocket(-1){}
    virtual ~Tcp_Server(){
        Close_Socket();
    }
    //获得socket
    int getServerSocket() const{
        return _serversocket;
    }
    void setServerSocket(int new_serversocket){
        _serversocket = new_serversocket;
    }
    //初始化socket
    void InitSocket(){
        //绑定服务器socket
        setServerSocket(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
    }
    //绑定端口号
    int Bind(const char* server_ip,const int port){
        if(getServerSocket()<0){
            InitSocket();
        }
        //创建服务器socket
         struct sockaddr_in echoserver;
        memset(&echoserver, 0, sizeof(echoserver));
        echoserver.sin_family = AF_INET;//使用以太网
        if(server_ip==NULL){
            echoserver.sin_addr.s_addr = htonl(INADDR_ANY);//设置server可接受任何ip htonl 将网络字节序转成本机字节序
        }
        else{
            echoserver.sin_addr.s_addr = inet_addr(server_ip);
        }
        echoserver.sin_addr.s_addr = htonl(INADDR_ANY);//设置server可接受任何ip htonl 将网络字节序转成本机字节序
        echoserver.sin_port = htons(port);//设置端口号 htons将端口号转为网络字节序
        int isbind = ::bind(_serversocket, (struct sockaddr *) &echoserver,sizeof(echoserver));
        if(isbind<0){
            cout<<"绑定端口<"<<port<<">失败:"<<isbind<<endl;
        }
        return isbind;
    }
    //监听端口号
    int Listen(int maxConnectNum = 5) const{
        int islisten = listen(getServerSocket(), maxConnectNum);
        if(islisten<0){
            cout<<"监听失败"<<endl;
        }
        else{
            cout<<"监听成功"<<endl;
        }
        return islisten;
    }
    //接收客户连接
    int AcceptClientConnect() const{
        //接收客户连接
        struct sockaddr_in echoclient;
        unsigned int echoclient_len = sizeof(echoclient);
        int clientsock = accept(getServerSocket(), (struct sockaddr *) &echoclient,&echoclient_len);
        if(clientsock<0){
            cout<<"accept失败"<<endl;
        }
        cout<<"客户端sock:"<<clientsock<<" 客户端地址:"<<inet_ntoa(echoclient.sin_addr)<<endl;
        return clientsock;
    }
    //关闭socket
    void Close_Socket() {
        //关闭客户端socket
        for(int i = (int)_clients.size()-1;i>=0;i--){
            close(_clients[i]->getSocket());
            delete _clients[i];
        }
        _clients.clear();
        //关闭服务端socket
        if(getServerSocket()>0){
            close(getServerSocket());
            setServerSocket(-1);
        }
    }
    //处理网络消息
    bool onRun(){
        if(isRun()){
            fd_set fd_Read;
            fd_set fd_Write;
            fd_set fd_Except;
            int maxfdp1 = 0;
            //置空文件描述符
            __DARWIN_FD_ZERO(&fd_Read);
            __DARWIN_FD_ZERO(&fd_Write);
            __DARWIN_FD_ZERO(&fd_Except);
            //设置文件描述符
            __DARWIN_FD_SET(getServerSocket(), &fd_Read);
            __DARWIN_FD_SET(getServerSocket(), &fd_Write);
            __DARWIN_FD_SET(getServerSocket(), &fd_Except);
            for(int i = (int)_clients.size()-1;i>=0;i--){
                //设置客户端soceket的文件描述符
                __DARWIN_FD_SET(_clients[i]->getSocket(), &fd_Read);
                maxfdp1 = max(maxfdp1,_clients[i]->getSocket());
            }
            timeval time_val= {0,0};
            int ret = select(max(maxfdp1,getServerSocket())+1, &fd_Read, &fd_Write, &fd_Except, &time_val);
            //        int ret = select(max(maxfdp1,server_sock)+1, &fd_Read, &fd_Write, &fd_Except, NULL);
            if(ret<0){
                cout<<"select 出错，返回-1，结束"<<endl;
                return false;
            }
            //判断当前是否有新的客户端发起请求
            if(__DARWIN_FD_ISSET(getServerSocket(), &fd_Read)){
                __DARWIN_FD_CLR(getServerSocket(), &fd_Read);
                //accept 处理请求
                int clientsock = AcceptClientConnect();
                //向客户端发送新用户加入的消息
                NewUserJoin new_user_join;
                new_user_join.new_user_socket = clientsock;
                new_user_join.length = sizeof(NewUserJoin);
                sendMsgToAll(&new_user_join);
                ClientSocket * new_client = new ClientSocket(clientsock);
                _clients.push_back(new_client);
            }
            //处理客户端socket连接请求
            for(int i = (int)_clients.size()-1;i>=0;i--){
                if(__DARWIN_FD_ISSET(_clients[i]->getSocket(),&fd_Read)){
                    if(-1==RecvMessage(_clients[i])){
                        //删除当前套接字
                        //__DARWIN_FD_CLR(gclient[i], &fd_Read);
                        delete _clients[i];
                        _clients.erase(_clients.begin()+i);
                    }
                }
            }
            return true;
        }
        else return false;  
    }
    //是否工作
    bool isRun() const{
        return getServerSocket()>0;
    }
    //接收数据，处理粘包、拆包
    char szRecv[recv_buffer_size];
    int RecvMessage(ClientSocket* clientsock) {
        //设置接收缓冲区
        int received_len = recv(clientsock->getSocket(), szRecv, recv_buffer_size, 0);
        header* received_header = (header*) szRecv;
        //处理buffer
        if(received_len<=0){
            cout<<"客户端: "<<clientsock<<" 退出"<<endl;
            return -1;
        }
        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        memcpy(clientsock->getRecvMsg()+clientsock->getLastPos(),szRecv,received_len);
        clientsock->setLastPos(clientsock->getLastPos()+received_len);
        while(clientsock->getLastPos()>=sizeof(header)){
            header* received_header = (header*)clientsock->getRecvMsg();
            if(clientsock->getLastPos()>=received_header->length){
                int n_size = clientsock->getLastPos()-received_header->length;
                cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
                OnNetMsg(clientsock->getSocket(),received_header);
                memcpy(clientsock->getRecvMsg(), clientsock->getRecvMsg()+received_header->length, n_size);
                clientsock->setLastPos(n_size);
            }
            else break;
        }
        
        return received_len;
    }
    //响应网络消息
    virtual void OnNetMsg(const int clientsock,const header* received_header) {
        switch (received_header->cmd) {
            case CMD_LOGIN:{
                LoginMsg *received_loginMsg = (LoginMsg*)received_header;
                cout<<"username:"<<received_loginMsg->username<<" password"<<received_loginMsg->password<<endl;
                //判断登陆的信息
                //登陆成功
                LoginResult login_result;
                login_result.res = 1;
                login_result.cmd = CMD_LOGIN_RESULT;
                login_result.length = sizeof(LoginResult);
                sendMsg(clientsock,&login_result);
            }
                break;
            case CMD_LOGOUT:{
                LogoutMsg *received_logoutMsg = (LogoutMsg*)received_header;
                cout<<"username:"<<received_logoutMsg->username<<endl;
                //判断登出的信息
                //登出成功
                LogoutResult logout_result;
                logout_result.res = 1;
                logout_result.cmd = CMD_LOGOUT_RESULT;
                logout_result.length = sizeof(LogoutResult);
                sendMsg(clientsock,&logout_result);
            }
                break;
            default:{
                //错误情况
                header errorheader;
                errorheader.cmd = ERROR;
                errorheader.length = sizeof(errorheader);
                sendMsg(clientsock,&errorheader);
            }
                break;
        }
    }
    //向指定socket发送数据
    int sendMsg(const int clientsocket,const header* sendheader) const{
        if(isRun()&&sendheader){
            return send(clientsocket,(const char*)sendheader, sendheader->length, 0);
        }
        return -1;
    }
    //向所有socket发送数据
    void sendMsgToAll(const header* sendheader) const{
        for(int i = (int)_clients.size()-1;i>=0;i--){
            sendMsg(_clients[i]->getSocket(),sendheader);
        }
    }
};

#endif /* Tcp_Server_h */
