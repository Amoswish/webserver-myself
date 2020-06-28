//
//  InetEvent.h
//  webserver
//
//  Created by wu hao on 2020/6/28.
//  Copyright © 2020 wu hao. All rights reserved.
//

#ifndef InetEvent_h
#define InetEvent_h
//网络事件接口
#include "Cell.h"
#include "CellServer.h"
#include "CellClient.h"
class INetEvent{
private:
public:
    virtual void onNetLeave(CellClient* pClient) = 0;
    virtual void onNetJoin(CellClient* pClient) = 0;
    virtual void onNetMsg(CellServer* pCellServer,CellClient* pClient,const NetMsg_Header* received_header) = 0;
    virtual void onNetRecv(CellClient* pClient) = 0;
};

#endif /* InetEvent_h */
