//
//  CELLSemaphore.h
//  webserver
//
//  Created by wu hao on 2020/6/29.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLSemaphore_h
#define CELLSemaphore_h
#include <chrono>
#include <thread>
#include <condition_variable>
//信号量
class CELLSemaphore{
private:
    bool _isWaitExit = false;
    std::mutex _mutex;
    std::condition_variable _cv;
    int _waitCount = 0;
    int _wakeupCount = 0;
public:
    CELLSemaphore(){
        
    }
    ~CELLSemaphore(){
        
    }
    //阻塞当前进程
    void wait(){
        //阻塞等待onRun退出
        std::unique_lock<std::mutex> lock(_mutex);
        if(--_waitCount<0){
            //当前该信号量没有wait
            _cv.wait(lock,[this]()->bool{
                //判断当前是否有wakeup在运行
                return _wakeupCount>0;
            });
            --_wakeupCount;
        }
    }
    //唤醒当前进程
    void wakeup(){
        std::lock_guard<std::mutex> lock(_mutex);
        if(++_waitCount <=0){
            //保证必有wait在运行
            ++_wakeupCount;
            _cv.notify_one();
        }
    }
};
#endif /* CELLSemaphore_h */
