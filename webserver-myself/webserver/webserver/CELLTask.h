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
//任务类型基类
class  CellTask{
private:
    
public:
    CellTask(){
        
    }
    virtual ~CellTask(){
        
    }
    //执行任务
    virtual void doTask(){
        
    }
};
class CellTaskServer{
private:
    //任务数据
    std::list<CellTask*> _tasks;
    //任务数据缓冲区
    std::list<CellTask*> _tasksBuffer;
    //改变数据缓冲区是需要加锁
    std::mutex _mutex;
public:
    CellTaskServer(){
        
    }
    virtual ~CellTaskServer(){
        
    }
    void addTask(CellTask* task){
        std::lock_guard<std::mutex> lock(_mutex);
        //加入到任务数据缓冲区
        _tasksBuffer.push_back(task);
    }
    //启动工作线程
    void Start(){
        std:: thread t(std::mem_fn(&CellTaskServer::onRun),this);
        t.detach();
    }
    void onRun(){
        while(true){
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
                    (*itor)->doTask();
                    delete (*itor);
                    itor = _tasks.erase(itor);
                }
            }
        }
    }
};
#endif /* CELLTask_h */
