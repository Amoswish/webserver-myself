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
#include "CELLThread.h"
#include "CELLSemaphore.h"
using namespace std;
class CellServer{
private:
    //正式客户队列
    int _serversocket;
    int _id = -1;
    std::vector<CellClient*> _clients;
    //缓冲客户队列
    std::vector<CellClient*> _clientsBuffer;
    //缓冲队列锁
    std::mutex _mutex;
    //std::thread _thread;
    //网络事件对象
    INetEvent* _pNetEvent;
    //    备份客户端fdread
    fd_set _fdRead_bak;
    //    客户端列表是否有变化
    int maxfdp1;
    CellTaskServer _taskServer;
    time_t _oldTime = CELLTime::getNowInMilliSec();
    bool _clients_change = false;
     //是否工作
    CELLThread _thread;
public:
    CellServer(int serversocket = INVALID_SOCKET,int cellserver_id = -1):_serversocket(serversocket),_pNetEvent(NULL),maxfdp1(0){
        _id = cellserver_id;
        _taskServer._serverId =_id;
    }
    virtual ~CellServer(){
        cout<<"~CellServer"<<_serversocket<<" start"<<endl;
        //关闭所有客户端socket
        Close_Socket();
        cout<<"~CellServer end"<<endl;
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
        _taskServer.Start();
        _thread.Start(
            //onCreate
            NULL,
            //onRun
            [this](CELLThread* thread){
                this->onRun(thread);
            },
            //onClose
            [this](CELLThread* thread){
                this->ClearClients();
                this->_taskServer.Close();
            });
    }
    //关闭socket
    void Close_Socket() {
        cout<<"cellserver"<<_id<<"close start"<<endl;
        _thread.Close();
        cout<<"cellserver"<<_id<<"close end"<<endl;
    }
    bool onRun(CELLThread* thisThread){
        while(thisThread->isRun()){
            if(!_clientsBuffer.empty()){
                //自解锁
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto item_client:_clientsBuffer){
                    if(_pNetEvent){
                        _pNetEvent->onNetJoin(item_client);
                    }
                    _clients.push_back(item_client);
                }
                _clients_change = true;
                _clientsBuffer.clear();
            }
            if(_clients.empty()) {
                //如果客户端为空，跳过
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
                _oldTime = CELLTime::getNowInMilliSec();
                continue;
            }
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
                cout<<"CELLServer onRun select error"<<endl;
                thisThread->Exit();
                break;
            }
            ReadData(fd_Read);
            CheckTime();
        }
        return false;
    }
    
    void CheckTime(){
        time_t nowTime = CELLTime::getNowInMilliSec();
        time_t dt = nowTime-_oldTime;
        _oldTime = nowTime;
        for(auto iter = _clients.begin();iter!=_clients.end();){
            //心跳检测
            if((*iter)->checkHeart(dt)){
                //删除当前套接字
                _clients_change = true;
                if(_pNetEvent){
                    _pNetEvent->onNetLeave((*iter));
                }
                close((*iter)->getSocket());
                delete (*iter);
                iter = _clients.erase(iter);
                continue;
            }
            //定时发送数据检测
            (*iter)->checkSend(dt);
            iter++;
        }
    }
    void ReadData(fd_set& fd_Read){
        //处理客户端socket连接请求
        for(auto iter = _clients.begin();iter!=_clients.end();){
            if(__DARWIN_FD_ISSET((*iter)->getSocket(),&fd_Read)){
                if(SOCKET_ERROR==RecvMessage((*iter))){
                    //删除当前套接字
                    _clients_change = true;
                    if(_pNetEvent){
                        _pNetEvent->onNetLeave((*iter));
                    }
                    close((*iter)->getSocket());
                    delete (*iter);
                    iter = _clients.erase(iter);
                    continue;
                }
            }
            iter++;
        }
    }
    
    //接收数据，处理粘包、拆包
    int RecvMessage(CellClient* client) {
        //设置接收缓冲区
        int received_len = recv(client->getSocket(), client->getRecvMsg()+client->getLastRecvPos(), RECV_BUFFER_SIZE, 0);
        //处理buffer
        if(received_len<=0){
            return SOCKET_ERROR;
        }
        _pNetEvent->onNetRecv(client);
        NetMsg_Header* received_header = (NetMsg_Header*) client->getRecvMsg();
        //        cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
        client->setLastRecvPos(client->getLastRecvPos()+received_len);
        while(client->getLastRecvPos()>=sizeof(NetMsg_Header)){
            NetMsg_Header* received_header = (NetMsg_Header*)client->getRecvMsg();
            if(client->getLastRecvPos()>=received_header->length){
                int n_size = client->getLastRecvPos()-received_header->length;
                OnNetMsg(client,received_header);
                memcpy(client->getRecvMsg(), client->getRecvMsg()+received_header->length, n_size);
                client->setLastRecvPos(n_size);
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
        _taskServer.addTask([pClient,ret](){
            pClient->sendMsg(ret);
            delete ret;
        });
    }
private:
    void ClearClients(){
        if(getServerSocket()!=SOCKET_ERROR){
            for(auto item:_clients){
                delete item;
            }
            _clients.clear();
            for(auto item:_clientsBuffer){
                delete item;
            }
            _clientsBuffer.clear();
        }
    }
};


#endif /* CELLServer_h */
