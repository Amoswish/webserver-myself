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
public:
    CellClient(int socket = -1):_socket(socket),_lastRecvPos(0),_lastSendPos(0){
        memset(_recvMsgBuffer,'\0',RECV_BUFFER_SIZE);
        memset(_SendBuffer,'\0',SEND_BUFFER_SIZE);
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

#endif /* CELLClient_h */
