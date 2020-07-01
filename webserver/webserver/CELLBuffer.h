//
//  CELLBuffer.h
//  webserver
//
//  Created by wu hao on 2020/6/30.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLBuffer_h
#define CELLBuffer_h
#include "CELL.h"
class CELLBuffer{
private:
    //缓冲区
    char* _buffer;
    //缓冲区数据尾部位置
    int _lastPos;
    //缓冲区大小
    int _bufferSize;
    //缓冲区写满计数
    int _fullCount;
    std::mutex _mutex;
    std::mutex _mutex2;
public:
    CELLBuffer(int bufferSize = 8192):_bufferSize(bufferSize),_fullCount(0),_lastPos(0){
        _buffer = new char[bufferSize];
    }
    ~CELLBuffer(){
        if(_buffer!=NULL){
            delete[] _buffer;
        }
    }
    int getLastPos() const{
        return _lastPos;
    }
    void setLastPos(int lastPos) {
         _lastPos = lastPos;
    }
    int getBufferSize() const{
        return _bufferSize;
    }
    char* getBuffer(){
        return _buffer;
    }
    bool pushData(const char* data,const int data_length){
         std::lock_guard<std::mutex>lock(_mutex);
        if((getLastPos()+ data_length) < getBufferSize()){
            memcpy(_buffer+getLastPos(), data, data_length);
            setLastPos(getLastPos()+ data_length);
            if(getLastPos() == getBufferSize()){
                _fullCount++;
            }
            
            return true;
        }
        else{
            _fullCount++;
            return false;
        }
    }
    int sendData(const int socket_id){
        std::lock_guard<std::mutex>lock(_mutex2);
        int ret = 0;
        if(getLastPos()>0&&INVALID_SOCKET!=socket_id){
            //发送数据
            //cout<<"向客户端"<<socket_id<<"sendData_lastPos:"<<getLastPos()<<endl;
            ret = send(socket_id, getBuffer(), getLastPos(), 0);
            setLastPos(0);
            _fullCount = 0;
        }
        if(ret <0){
            cout<<"sendData fail"<<endl;
        }
        return ret;
    }
    int readData(const int socket_id){
        int received_len = -1;
        if(getBufferSize()-getLastPos()>0){
            received_len = recv(socket_id, getBuffer()+getLastPos(), getBufferSize()-getLastPos(), 0);
            if(received_len<=0) return received_len;
            setLastPos(getLastPos()+received_len);
        }
        return received_len;
    }
    bool hasMsg(){
        if(getLastPos()>=sizeof(NetMsg_Header)){
            NetMsg_Header* received_header = (NetMsg_Header*)getBuffer();
//            cout<<"接收的命令："<<received_header->cmd<<"接收的长度："<<received_header->length<<endl;
            if(getLastPos()>=received_header->length){
                return true;
            }
        }
        return false;
    }
    
    void popFrontData(int length){
        if(getLastPos()>length){
            memcpy(getBuffer(), getBuffer()+length, getLastPos()-length);
        }
        setLastPos(getLastPos()-length);
        if(_fullCount>0){
            --_fullCount;
        }
    }
};

#endif /* CELLBuffer_h */
