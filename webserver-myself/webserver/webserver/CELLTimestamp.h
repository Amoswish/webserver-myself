//
//  CELLTimestamp.h
//  webserver
//
//  Created by wu hao on 2020/6/8.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLTimestamp_h
#define CELLTimestamp_h

#include<chrono>
using namespace std::chrono;
class CELLTime{
public:
    //获取当前时间戳（毫秒）
    static time_t getNowInMilliSec(){
        auto t = high_resolution_clock::now().time_since_epoch();
        return duration_cast<milliseconds>(t).count();
    }
};
class CELLTimestamp{
public:
    CELLTimestamp(){
        update();
    }
    virtual ~CELLTimestamp(){

    }
    //获取当前秒
    double getElapsedSecond(){
        return this->getElapsedTimeMicroSec()*0.000001;
    }
    //获取当前毫秒
    double getElapsedInMillSec(){
        return this->getElapsedTimeMicroSec()*0.001;
    }
    void update(){
        _begin = high_resolution_clock::now();
    }
    long long getElapsedTimeMicroSec(){
        auto t = high_resolution_clock::now() - _begin;
        return duration_cast<microseconds>(t).count();
    }
protected:
    time_point<high_resolution_clock> _begin;
    
};
#endif /* CELLTimestamp_h */
