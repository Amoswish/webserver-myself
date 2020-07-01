//
//  CELLClient.h
//  webserver
//
//  Created by wu hao on 2020/6/28.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLClient_h
#define CELLClient_h

#include "CELL.h"
#include "CELLBuffer.h"
//客户端心跳检测计时时间 1000表示1s
#define CLIENT_HESRT_DEAD_TIME 6000
//客户端定时发送间隔时间 1000表示1s
#define CLIENT_SENDBUFFER_TIME 200
using namespace std;
//客户端数据类型
class CellClient{
private:
    int _socket;
    //消息接收缓冲区
    CELLBuffer _recvBuffer;
    //消息发送缓冲区
    CELLBuffer _sendBuffer;
    //心跳死亡时间
    time_t _dtHeart;
    //发送间隔时间
    time_t _dtSend;
public:
    CellClient(int socket = -1):_socket(socket),_recvBuffer(RECV_BUFFER_SIZE),_sendBuffer(SEND_BUFFER_SIZE*100),_dtHeart(0){
    }
    virtual ~CellClient(){
        cout<<"~CellClient"<<_socket<<" start"<<endl;
        close(getSocket());
        cout<<"~CellClient "<<_socket<<" end"<<endl;
        _socket = INVALID_SOCKET;
    }
    int getSocket() const{
        return _socket;
    }
    void setSocket(const int socket) {
        _socket = socket;
    }
    //立即将缓冲区数据发送给客户端
    int sendMsgReal(){
//        cout<<"sendMsgReal"<<endl;
        resetDTSend();
        return _sendBuffer.sendData(getSocket());
    }
    int sendMsg( NetMsg_Header* sendheader) {
        int ret = SOCKET_ERROR;
        //要发送的数据
        
        const char* pSendData = (const char*) sendheader;
        ret = _sendBuffer.pushData(pSendData,sendheader->length);
        //cout<<_socket<<"pushData "<<ret<<endl;
        return ret;
    }
    int recvMsg(){
        return _recvBuffer.readData(getSocket());
    }
    int hasMsg(){
        return _recvBuffer.hasMsg();
    }
    NetMsg_Header* getFrontData(){
        return (NetMsg_Header*)_recvBuffer.getBuffer();
    }
    void pop_front_msg(){
        if(hasMsg()){
            _recvBuffer.popFrontData(getFrontData()->length);
        }
    }
    void resetDTHeart(const int dtHeart = 0){
        _dtHeart = dtHeart;
    }
    void resetDTSend(const int dtSend = 0){
        _dtSend = dtSend;
    }
    bool checkHeart(const time_t dt){
        _dtHeart += dt;
        if(_dtHeart>= CLIENT_HESRT_DEAD_TIME){
            std::cout<<"checkHeart dead.time:"<<_dtHeart<<"socket:"<<getSocket()<<std::endl;
            return true;
        }
        return false;
    }
    bool checkSend(const time_t dt){
        _dtSend += dt;
        if(_dtSend>= CLIENT_SENDBUFFER_TIME){
            std::cout<<"checkSend.time:"<<_dtHeart<<"socket:"<<getSocket()<<std::endl;
            sendMsgReal();
            resetDTSend();
            return true;
        }
        return false;
    }
};

#endif /* CELLClient_h */
