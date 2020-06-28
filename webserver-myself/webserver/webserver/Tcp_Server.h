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
#include <mutex>
#include <thread>
#include "Cell.h"
#include "CellClient.h"
#include "CellServer.h"
#include "InetEvent.h"

using namespace std;

class Tcp_Server:public INetEvent{
private:
    int _serversocket;
    vector<CellServer*> _cellServers;
public:
    CELLTimestamp _tTime;
    std::atomic_int _recvCount;
    //每秒接收到的消息计数
    std::atomic_int _msgCount;
    //客户端计数
    std::atomic_int _clientCount;
public:
    Tcp_Server():_serversocket(-1),_recvCount(0),_clientCount(0),_msgCount(0){}
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
    void Start(int thread_num = 1){
        for(int i = 0;i<thread_num;i++){
            auto ser = new CellServer(getServerSocket());
            //注册网络事件
            ser->setInetEvent(this);
            _cellServers.push_back(ser);
            //启动消息处理线程
            ser->Start();
        }
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
//        cout<<"客户端sock:"<<clientsock<<" 客户端地址:"<<inet_ntoa(echoclient.sin_addr)<<endl;
        return clientsock;
    }
    //关闭socket
    void Close_Socket() {
        //关闭服务端socket
        if(getServerSocket()>0){
            close(getServerSocket());
            setServerSocket(-1);
        }
    }
    //处理网络消息
    bool onRun(){
        if(isRun()){
            time4msg();
            fd_set fd_Read;
            //置空文件描述符
            __DARWIN_FD_ZERO(&fd_Read);
            //设置文件描述符
            __DARWIN_FD_SET(getServerSocket(), &fd_Read);
            timeval time_val= {1,0};
            int ret = select(getServerSocket()+1, &fd_Read, NULL,NULL, &time_val);
            if(ret<0){
                cout<<"select 出错，返回-1，结束"<<endl;
                return false;
            }
            //判断当前是否有新的客户端发起请求
            if(__DARWIN_FD_ISSET(getServerSocket(), &fd_Read)){
                __DARWIN_FD_CLR(getServerSocket(), &fd_Read);
                //accept 处理请求
                int clientsock = AcceptClientConnect();
                addClientToCellServer(new CellClient(clientsock));
            }
            return true;
        }
        else return false;  
    }
    void addClientToCellServer(CellClient* new_client){
        //将客户端加入到连接数最小的线程中
        auto min_connect_cellServers = _cellServers[0];
        for(auto item:_cellServers){
            if(min_connect_cellServers->getClientCount()>item->getClientCount()) min_connect_cellServers = item;
        }
        min_connect_cellServers->addClient(new_client);
        onNetJoin(new_client);
    }
    //是否工作
    virtual void onNetLeave(CellClient* pClient){
        cout<<"客户端: "<<pClient->getSocket()<<" 退出"<<endl;
        _clientCount--;
    }
    virtual void onNetJoin(CellClient* pClient){
        cout<<"加入客户端sock:"<<pClient->getSocket()<<endl;
        _clientCount++;
    }
    virtual void onNetMsg(CellServer* pCellServer,CellClient* pClient,const NetMsg_Header* received_header){
        _msgCount++;
    }
    virtual void onNetRecv(CellClient* pClient){
         _recvCount++;
    };
    //网络消息计数
    void time4msg(){
        auto t1 = _tTime.getElapsedSecond();
        if(t1>=1.0){
            cout<<"time:"<<t1<<" socket:"<<_serversocket<<" client num:"<<_clientCount<<"recvCount:"<<_recvCount<<"msgCount:"<<_msgCount<<endl;
            _tTime.update();
             _recvCount = 0;
            _msgCount = 0;
        }
    }
    bool isRun() const{
        return getServerSocket()>0;
    }
};

#endif /* Tcp_Server_h */
