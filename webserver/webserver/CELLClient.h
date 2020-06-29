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
//客户端心跳检测计时时间 1000表示1s
#define CLIENT_HESRT_DEAD_TIME 60000
//客户端定时发送间隔时间 1000表示1s
#define CLIENT_SENDBUFFER_TIME 200
using namespace std;
//客户端数据类型
class CellClient{
private:
    int _socket;
    //消息接收缓冲区
    char _recvMsgBuffer[RECV_BUFFER_SIZE*10];
    int _lastRecvPos;
    //消息发送缓冲区
    char _SendBuffer[RECV_BUFFER_SIZE*10];
    int _lastSendPos;
    //心跳死亡时间
    time_t _dtHeart;
    //发送间隔时间
    time_t _dtSend;
public:
    CellClient(int socket = -1):_socket(socket),_lastRecvPos(0),_lastSendPos(0),_dtHeart(0){
        memset(_recvMsgBuffer,'\0',RECV_BUFFER_SIZE);
        memset(_SendBuffer,'\0',SEND_BUFFER_SIZE);
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
    //立即发送数据
    int sengMsgReal(const NetMsg_Header* sendheader){
        int ret = SOCKET_ERROR;
        if(getLastSendPos()>0&&SOCKET_ERROR!=getSocket()){
            ret = send(getSocket(),sendheader, sendheader->length, 0);
            resetDTSend();
        }
        return ret;
    }
    //立即将缓冲区数据发送给客户端
    int sengMsgReal(){
        int ret = SOCKET_ERROR;
        if(getLastSendPos()>0&&SOCKET_ERROR!=getSocket()){
            ret = send(getSocket(),_SendBuffer, getLastSendPos(), 0);
            setLastSendPos(0);
            resetDTSend();
        }
        return ret;
    }
    int sendMsg(const NetMsg_Header* sendheader) {
        int ret = SOCKET_ERROR;
        //要发送的数据长度
        int nSendLen = sendheader->length;
        //要发送的数据
        const char* pSendData = (const char*) sendheader;
        while(true){
            if(getLastSendPos()+nSendLen >= SEND_BUFFER_SIZE){
                //计算可拷贝的数据长度
                int nCopyLen = SEND_BUFFER_SIZE-getLastSendPos();
                memcpy(_SendBuffer+getLastSendPos(),pSendData,nCopyLen);
                //计算剩余的数据位置
                pSendData += nCopyLen;
                //计算剩余的数据长度
                nSendLen -= nCopyLen;
                //发送数据
                ret = send(getSocket(),_SendBuffer, SEND_BUFFER_SIZE, 0);
                resetDTSend();
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
        return ret;
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
            //std::cout<<"checkSend.time:"<<_dtHeart<<"socket:"<<getSocket()<<std::endl;
            sengMsgReal();
            resetDTSend();
            return true;
        }
        return false;
    }
};

#endif /* CELLClient_h */
