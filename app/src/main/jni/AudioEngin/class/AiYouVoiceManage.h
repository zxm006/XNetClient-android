//
//  AiYouVoiceManage.h
//  VoiceTalk
//
//  Created by zhangxinming on 16/7/21.
//  Copyright © 2016年 zhangxinming. All rights reserved.
//

#ifndef AiYouVoiceManage_h
#define AiYouVoiceManage_h


#import <Foundation/Foundation.h>

#ifdef __cplusplus
extern "C" {
#endif
    typedef int (*UNITYSENDMSG)(const char *cunityObj,const char* unityFunSucces,const char* msg);
    void setUnityVoiceSendmsg(UNITYSENDMSG unitymsg);
    
    //初始化视频回调
    void initVoiceManage(const char* unityObject,const char* UnityFunSucces,const char* UnityFunFail);
   
     //登录对讲服务器
    void  loginVoiceServer(const char* szIP,const char* szName,const char* szGameId,const char* szGameServerId,const char* szRoomId,const char* szGroupId,BOOL listenInRoom);
 
    void  logoutServer();
    
    void  startSendAudio(BOOL isspeakinroom);
    void  stopSendAudio();
 
    //底层测试接口，和上层应用无关
    void  setNetWorkCallback(void *netWorkCallback);

    
#ifdef __cplusplus
}
#endif

#endif /* AiYouVoiceManage_h */
