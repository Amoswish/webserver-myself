//
//  CELLTask.h
//  webserver
//
//  Created by wu hao on 2020/6/27.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLTask_h
#define CELLTask_h
#include <thread>
#include <mutex>
#include <list>
#include <functional>
#include "CELLSemaphore.h"
#include "CELLThread.h"
using namespace std;
class CellTaskServer{
public:
    int _serverId = -1;
private:
    typedef std::function<void()> CellTask;
    //任务数据
    std::list<CellTask> _tasks;
    //任务数据缓冲区
    std::list<CellTask> _tasksBuffer;
    //改变数据缓冲区是需要加锁
    std::mutex _mutex;
    CELLThread _thread;
public:
    CellTaskServer(){
    }
    virtual ~CellTaskServer(){
        
    }
    void addTask(CellTask task){
        std::lock_guard<std::mutex> lock(_mutex);
        //加入到任务数据缓冲区
        _tasksBuffer.push_back(task);
    }
    //启动工作线程
    void Start(){
        _thread.Start(NULL,[this](CELLThread* thread){
            this->onRun(thread);
        });
    }
    void Close(){
        cout<<"CellTaskServer"<<_serverId<<"close start"<<endl;
        _thread.Close();
        cout<<"CellTaskServer"<<_serverId<<"close end"<<endl;
    }
    void onRun(CELLThread* thisThread){
        while(thisThread->isRun()){
            if(!_tasksBuffer.empty()){
                //从任务数据缓冲区去除数据
                std::lock_guard<std::mutex> lock(_mutex);
                for(auto ptask:_tasksBuffer){
                    _tasks.push_back(ptask);
                }
                _tasksBuffer.clear();
            }
            if(_tasks.empty()){
                std::chrono::milliseconds t(1);
                std::this_thread::sleep_for(t);
            }
            else{
                for(auto itor = _tasks.begin();itor!=_tasks.end();itor++){
                    (*itor)();
                    itor = _tasks.erase(itor);
                }
                _tasks.clear();
            }
        }
    }
};
#endif /* CELLTask_h */
