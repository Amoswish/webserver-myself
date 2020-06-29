//
//  CELLThread.h
//  webserver
//
//  Created by wu hao on 2020/6/29.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLThread_h
#define CELLThread_h
#include "CELLSemaphore.h"
class CELLThread{
private:
    typedef std::function<void(CELLThread*)> EventCall;
    EventCall _event_onCreate;
    EventCall _event_onRun;
    EventCall _event_onDestory;
    CELLSemaphore _sem;
    std::mutex _mutex;
    bool _isRun = false;
public:
    bool isRun() const{
        return _isRun;
    }
    void Start(EventCall event_onCreate = NULL,EventCall event_onRun = NULL,EventCall event_onDestory = NULL){
        std::lock_guard<std::mutex>lock(_mutex);
        if(event_onCreate){
            _event_onCreate = event_onCreate;
        }
        if(event_onRun){
            _event_onRun = event_onRun;
        }
        if(event_onDestory){
            _event_onDestory = _event_onDestory;
        }
        _isRun = true;
        if(_event_onCreate){
            _event_onCreate(this);
        }
        std::thread t(std::mem_fn(&CELLThread::OnRun),this);
        t.detach();
    }
    void Close(){
        std::lock_guard<std::mutex>lock(_mutex);
        if(_isRun){
            _isRun = false;
            _sem.wait();
            if(_event_onDestory){
                _event_onDestory(this);
            }
        }
    }
    //在工作函数中退出 不需要使用信号量 如果使用会造成死锁
    void Exit(){
        if(_isRun){
            _isRun = false;
            if(_event_onDestory){
                _event_onDestory(this);
            }
        }
    }
protected:
    void OnRun(){
        if(_event_onRun){
            _event_onRun(this);
        }
        _sem.wakeup();
    }
};
#endif /* CELLThread_h */
