//
//  CELLServer.h
//  webserver
//
//  Created by wu hao on 2020/6/28.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLServer_h
#define CELLServer_h

#include "CELL.h"
#include "InetEvent.h"
#include "CELLTask.h"
using namespace std;
//网络消息发送任务
class CellSendMsgToClientTask:public CellTask{
private:
    CellClient* _pClient;
    NetMsg_Header* _pheader;
public:
    CellSendMsgToClientTask(CellClient* pClient,NetMsg_Header* pheader):_pClient(pClient),_pheader(pheader){
        
    }
    virtual void doTask(){
        _pClient->sendMsg(_pheader);
        delete _pheader;
    }
};
class CellServer{
private:
    //正式客户队列
    int _serversocket;
    std::vector<CellClient*> _clients;
    //缓冲客户队列
    std::vector<CellClient*> _clientsBuffer;
    //缓冲队列锁
    std::mutex _mutex;
    std::thread _thread;
    //网络事件对象
    INetEvent* _pNetEvent;
    CellTaskServer _taskServer;
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
    void addClient(CellClient* clientsocket){
        //自解锁
        std::lock_guard<std::mutex> lock(_mutex);
        _clientsBuffer.push_back(clientsocket);
    }
    void Start(){
        _thread = std::thread(std::mem_fn(&CellServer::onRun),this);
        _taskServer.Start();
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
    int RecvMessage(CellClient* clientsock) {
        //设置接收缓冲区
        int received_len = recv(clientsock->getSocket(), clientsock->getRecvMsg()+clientsock->getLastRecvPos(), RECV_BUFFER_SIZE, 0);
        //处理buffer
        if(received_len<=0){
            return SOCKET_ERROR;
        }
        _pNetEvent->onNetRecv(clientsock);
        NetMsg_Header* received_header = (NetMsg_Header*) clientsock->getRecvMsg();
        //        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        clientsock->setLastRecvPos(clientsock->getLastRecvPos()+received_len);
        while(clientsock->getLastRecvPos()>=sizeof(NetMsg_Header)){
            NetMsg_Header* received_header = (NetMsg_Header*)clientsock->getRecvMsg();
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
    virtual void OnNetMsg(CellClient* client,const NetMsg_Header* received_header) {
        _pNetEvent->onNetMsg(this,client,received_header);
        //向指定socket发送数据
    };
    void addSendTask(CellClient* pClient,NetMsg_Header* ret){
        CellSendMsgToClientTask *task = new CellSendMsgToClientTask(pClient, ret);
        _taskServer.addTask(task);
    }
};



#endif /* CELLServer_h */
