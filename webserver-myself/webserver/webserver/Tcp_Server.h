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
#include "Tcp_Server.h"
#include "TcpMessage.h"
#include "CELLTimestamp.h"
#define recv_buffer_size 10240*5
#define send_buffer_size 10240*5
#define SOCKET_ERROR -1
using namespace std;
//客户端数据类型
class ClientSocket{
private:
    int _socket;
    //消息接收缓冲区
    char _recvMsgBuffer[recv_buffer_size*10];
    int _lastRecvPos;
    //消息发送缓冲区
    char _SendBuffer[recv_buffer_size*10];
    int _lastSendPos;
public:
    ClientSocket(int socket = -1):_socket(socket),_lastRecvPos(0),_lastSendPos(0){
        memset(_recvMsgBuffer,'\0',recv_buffer_size);
        memset(_SendBuffer,'\0',send_buffer_size);
    }
    int getSocket() const{
        return _socket;
    }
    void setSocket(const int socket) {
        _socket = socket;
    }
    int getLastRecvPos() const{
        return _lastRecvPos;
    }
    void setLastRecvPos(const int lastRecvPos){
        _lastRecvPos = lastRecvPos;
    }
    char* getRecvMsg(){
        return _recvMsgBuffer;
    }
    int getLastSendPos() const{
        return _lastSendPos;
    }
    void setLastSendPos(const int lastSendPos){
        _lastSendPos = lastSendPos;
    }
    char* getSendBuffer(){
        return _SendBuffer;
    }
    int sendMsg(const header* sendheader) {
        int ret = SOCKET_ERROR;
        //要发送的数据长度
        int nSendLen = sendheader->length;
        //要发送的数据
        const char* pSendData = (const char*) sendheader;
        while(true){
            if(getLastSendPos()+nSendLen >= send_buffer_size){
                //计算可拷贝的数据长度
                int nCopyLen = send_buffer_size-getLastSendPos();
                memcpy(_SendBuffer+getLastSendPos(),pSendData,nCopyLen);
                //计算剩余的数据位置
                pSendData += nCopyLen;
                //计算剩余的数据长度
                nSendLen -= nCopyLen;
                //发送数据
                ret = send(getSocket(),_SendBuffer, send_buffer_size, 0);
                setLastSendPos(0);
                //如果发送错误
                if(ret==SOCKET_ERROR) return ret;
            }
            else{
                memcpy(_SendBuffer+getLastSendPos(),pSendData,nSendLen);
                setLastSendPos(getLastSendPos() + nSendLen);
                break;
            }
        }
//        if(sendheader){
//            ret = send(_socket,(const char*)sendheader, sendheader->length, 0);
//        }
        return ret;
    }
};
//网络事件接口
class INetEvent{
private:
public:
    virtual void onNetLeave(ClientSocket* pClient) = 0;
    virtual void onNetJoin(ClientSocket* pClient) = 0;
    virtual void onNetMsg(ClientSocket* pClient,const header* received_header) = 0;
    virtual void onNetRecv(ClientSocket* pClient) = 0;
};
class CellServer{
private:
    //正式客户队列
    int _serversocket;
    std::vector<ClientSocket*> _clients;
    //缓冲客户队列
    std::vector<ClientSocket*> _clientsBuffer;
    //缓冲队列锁
    std::mutex _mutex;
    std::thread _thread;
    //网络事件对象
    INetEvent* _pNetEvent;
public:
    CellServer(int serversocket = -1):_serversocket(serversocket),_pNetEvent(NULL){}
    virtual ~CellServer(){
        Close_Socket();
    }
    int getServerSocket() const{
        return _serversocket;
    }
    ssize_t getClientCount(){
        return _clientsBuffer.size() +_clients.size();
    }
    void setInetEvent(INetEvent* pNetEvent){
        _pNetEvent = pNetEvent;
    }
    //增加客户端
    void addClient(ClientSocket* clientsocket){
        //自解锁
        std::lock_guard<std::mutex> lock(_mutex);
        _clientsBuffer.push_back(clientsocket);
    }
    void Start(){
        _thread = std::thread(std::mem_fn(&CellServer::onRun),this);
    }
    //是否工作
    bool isRun() const{
        return getServerSocket()>0;
    }
    //关闭socket
    void Close_Socket() {
        //关闭客户端socket
        for(int i = (int)_clients.size()-1;i>=0;i--){
            close(_clients[i]->getSocket());
            delete _clients[i];
        }
        _clients.clear();
    }
//    备份客户端fdread
    fd_set _fdRead_bak;
//    客户端列表是否有变化
    bool _clients_change = false;
    int maxfdp1 = 0;
    bool onRun(){
        while(isRun()){
            if(!_clientsBuffer.empty()){
                //自解锁
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto item_client:_clientsBuffer){
                    _clients.push_back(item_client);
                }
                _clients_change = true;
                _clientsBuffer.clear();
            }
            if(_clients.empty()) continue;
            fd_set fd_Read;
           
            //置空文件描述符
            __DARWIN_FD_ZERO(&fd_Read);
            if(_clients_change){
                _clients_change = false;
                for(int i = (int)_clients.size()-1;i>=0;i--){
                    //设置客户端soceket的文件描述符
                    __DARWIN_FD_SET(_clients[i]->getSocket(), &fd_Read);
                    maxfdp1 = max(maxfdp1,_clients[i]->getSocket());
                }
                //备份
                memcpy(&_fdRead_bak,&fd_Read,sizeof(fd_set));
            }
            else{
                memcpy(&fd_Read,&_fdRead_bak,sizeof(fd_set));
            }
            int ret = select(max(maxfdp1,getServerSocket())+1, &fd_Read, NULL,NULL,NULL);
            if(ret<0){
                cout<<"select 出错，返回-1，结束"<<endl;
                return false;
            }
            //处理客户端socket连接请求
            for(int i = _clients.size()-1;i>=0;i--){
                if(__DARWIN_FD_ISSET(_clients[i]->getSocket(),&fd_Read)){
                    if(SOCKET_ERROR==RecvMessage(_clients[i])){
                        //删除当前套接字
                        _clients_change = true;
                        if(_pNetEvent){
                            _pNetEvent->onNetLeave(_clients[i]);
                            
                        }
                        delete _clients[i];
                        _clients.erase(_clients.begin()+i);
                        
                    }
                }
            }
        }
        return false;
    }
    //接收数据，处理粘包、拆包
    
    int RecvMessage(ClientSocket* clientsock) {
        //设置接收缓冲区
        int received_len = recv(clientsock->getSocket(), clientsock->getRecvMsg()+clientsock->getLastRecvPos(), recv_buffer_size, 0);
        //处理buffer
        if(received_len<=0){
            return SOCKET_ERROR;
        }
        _pNetEvent->onNetRecv(clientsock);
        header* received_header = (header*) clientsock->getRecvMsg();
//        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        clientsock->setLastRecvPos(clientsock->getLastRecvPos()+received_len);
        while(clientsock->getLastRecvPos()>=sizeof(header)){
            header* received_header = (header*)clientsock->getRecvMsg();
            if(clientsock->getLastRecvPos()>=received_header->length){
                int n_size = clientsock->getLastRecvPos()-received_header->length;
                OnNetMsg(clientsock,received_header);
                memcpy(clientsock->getRecvMsg(), clientsock->getRecvMsg()+received_header->length, n_size);
                clientsock->setLastRecvPos(n_size);
            }
            else break;
        }
        
        return received_len;
    }
    //响应网络消息
    virtual void OnNetMsg(ClientSocket* client,const header* received_header) {
        _pNetEvent->onNetMsg(client,received_header);
        //向指定socket发送数据
    };
};
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
                addClientToCellServer(new ClientSocket(clientsock));
            }
            return true;
        }
        else return false;  
    }
    void addClientToCellServer(ClientSocket * new_client){
        //将客户端加入到连接数最小的线程中
        auto min_connect_cellServers = _cellServers[0];
        for(auto item:_cellServers){
            if(min_connect_cellServers->getClientCount()>item->getClientCount()) min_connect_cellServers = item;
        }
        min_connect_cellServers->addClient(new_client);
        onNetJoin(new_client);
    }
    //是否工作
    virtual void onNetLeave(ClientSocket* pClient){}
    virtual void onNetJoin(ClientSocket* pClient){}
    virtual void onNetMsg(ClientSocket* pClient,const header* received_header){}
    virtual void onNetRecv(ClientSocket* pClient){};
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
