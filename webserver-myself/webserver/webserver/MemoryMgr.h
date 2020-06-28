//
//  MemoryMgr.h
//  webserver
//
//  Created by wu hao on 2020/6/27.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef MemoryMgr_h
#define MemoryMgr_h
#include <stdlib.h>
#include <assert.h>
//内存池
const int __ALIGN = 64;
const int __MAX_BYTES = 1024;
const int __NFREELISTS = __MAX_BYTES/__ALIGN;
const int __DEFAULT_NOBJS = 10;
class MemoryAlloc{
private:
    static size_t ROUND_UP(size_t bytes){
        return (((bytes)+__ALIGN-1)& ~(__ALIGN-1));
    }
    struct obj{
        obj* free_list_link;
    };
    static obj*  volatile free_list[__NFREELISTS];
    static size_t FREELIST_INDEX(size_t bytes){
        //找到x在freelist中的index
        return (((bytes)+__ALIGN-1)/__ALIGN-1);
    }
    //    chunk_alloc state
    static char* start_free;
    static char* end_free;
    static size_t heap_size;
    static char* chunk_alloc(size_t size,int &nobjs){
        char* result;
        size_t total_bytes = size *nobjs;
        size_t bytes_left =end_free-start_free;
        if(bytes_left>=total_bytes){
            //pool大小满足所申请的nobjs个块
            result = start_free;
            start_free += total_bytes;
            return (result);
        }else if(bytes_left>=size){
            //pool大小满足所申请（含）一块以上的空间
            nobjs = bytes_left/size;
            total_bytes = size * nobjs;
            result = start_free;
            start_free += total_bytes;
            return (result);
        }else{
            //pool大小不满足所申请一块的空间
            ssize_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >>4);
            if(bytes_left > 0){
                //充分利用剩余空间
                obj* volatile *myfree_list =  free_list+FREELIST_INDEX(bytes_left);//找到申请的空间在free_list的位置
                ((obj*)start_free)->free_list_link = *myfree_list;
                *myfree_list = ((obj*)start_free);
            }
            start_free = (char*)malloc(bytes_to_get);
            if(0 == start_free){
                //申请失败
                obj* volatile *my_free_list,*p;
                for(size_t i = size;i<=__MAX_BYTES;i+=__ALIGN){
                    my_free_list = free_list+FREELIST_INDEX(bytes_left);
                    p = *my_free_list;
                    if(NULL!=p){
                        *my_free_list = p->free_list_link;
                        start_free = (char*)p;
                        end_free = start_free + i;
                        return (chunk_alloc(size,nobjs));//重新查找
                    }
                }
                end_free = NULL;//memory山穷水尽 改用第一级进行尝试
                start_free = (char*)malloc(bytes_to_get);
            }
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;
            return (chunk_alloc(size,nobjs));//重新查找
        }
    };
    static void* refill(size_t n){//n已调整为8的倍数
        int nobjs = __DEFAULT_NOBJS;
        char* chunk = chunk_alloc(n,nobjs);//nobjs is passed by reference
        obj* volatile *my_free_list;
        obj* result;
        obj* current_obj;
        obj* next_obj;
        if(1==nobjs){
            return (chunk);
        }
        //将所得chunk挂到free_list上
        my_free_list = free_list+FREELIST_INDEX(n);//找到申请的空间在free_list的位置
        //在chunk内建立 free_list
        result = (obj*)chunk;
        *my_free_list = next_obj = (obj*)(chunk+n);
        for(int i = 1;;++i){
            current_obj = next_obj;
            next_obj = (obj*)((char*)next_obj+n);
            if(nobjs-1 ==i){
                //为最后一个
                current_obj->free_list_link = NULL;
                break;
            }
            else{
                current_obj->free_list_link = next_obj;
            }
        }
        return (result);
    }
public:
    MemoryAlloc(){}
    static void* allocate(size_t n){
        obj* volatile *my_free_list;
        obj* result;
        if(n>__MAX_BYTES){
            //使用第一级
            return malloc(n);
        }
        my_free_list = free_list+FREELIST_INDEX(n);//找到申请的空间在free_list的位置
        result = *my_free_list;
        if(result == NULL){
            //当前free_list中没有空闲块
            void* r = refill(ROUND_UP(n));
            return r;
        }
        //将空闲块拿掉
        *my_free_list = result->free_list_link;
        return result;
    }
    static void deallocate(void* p,size_t n){
        obj* q = (obj*)p;
        obj* volatile *my_free_list;
        if(n>__MAX_BYTES){
            //使用第一级
            return free(p);
        }
        my_free_list = free_list+FREELIST_INDEX(n);
        //头插法
        q->free_list_link = *my_free_list;
        *my_free_list = q;
    }
    
};
char* MemoryAlloc::start_free = NULL;
char* MemoryAlloc::end_free = NULL;
size_t MemoryAlloc::heap_size = 0;
MemoryAlloc::obj*  volatile MemoryAlloc::free_list[__NFREELISTS] = {NULL};
class MemoryMgr:MemoryAlloc {
    //单例模式-懒汉式-静态
private:
    MemoryMgr(){}
public:
    //禁用拷贝构造和拷贝赋值
    MemoryMgr(const MemoryMgr&)=delete;
    MemoryMgr& operator=(const MemoryMgr&)=delete;
    ~MemoryMgr(){}
    static MemoryMgr& getInstance(){
        static MemoryMgr instance;
        return instance;
    }
    void* allocMem(size_t size){
        return getInstance().allocate(size);
    }
    void freeMem(void* p,size_t size){
        getInstance().deallocate(p, size);
    }
};
#endif /* MemoryMgr_h */
