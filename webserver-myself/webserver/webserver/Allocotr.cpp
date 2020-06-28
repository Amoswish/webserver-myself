//
//  Allocotr.cpp
//  webserver
//
//  Created by wu hao on 2020/6/27.
//  Copyright © 2020 wu hao. All rights reserved.
//

#include "Allocotr.hpp"
#include "MemoryMgr.h"

//class Allocotr{
//    void* operator new(size_t size){
//        return MemoryMgr::getInstance().allocMem(size);
//    }
//    void* operator new[](size_t size){
//        return MemoryMgr::getInstance().allocMem(size);
//    }
//    void operator delete(void* p,size_t size){
//        MemoryMgr::getInstance().freeMem(p,size);
//    }
//    void operator delete[](void* p,size_t size){
//        MemoryMgr::getInstance().freeMem(p,size);
//    }
//};
