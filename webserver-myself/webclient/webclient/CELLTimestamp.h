//
//  CELLTimestamp.hpp
//  webclient
//
//  Created by wu hao on 2020/6/26.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef CELLTimestamp_hpp
#define CELLTimestamp_hpp

#include<chrono>
using namespace std::chrono;
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
