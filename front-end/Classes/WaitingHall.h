#pragma once

#include "cocos2d.h"

USING_NS_CC;

class WaitingHall : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();
	Size visibleSize;
	Vec2 origin;

    virtual bool init();
    
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
    
    // implement the "static create()" method manually
    CREATE_FUNC(WaitingHall);
};
