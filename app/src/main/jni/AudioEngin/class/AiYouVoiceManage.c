//
//  AiYouVoiceManage.c
//  VoiceTalk
//
//  Created by zhangxinming on 16/7/21.
//  Copyright © 2016年 zhangxinming. All rights reserved.
//

#include "AiYouVoiceManage.h"
#include "CVoiceManage.h"
#include <string.h>
#import <UIKit/UIKit.h>

#ifdef __OBJC__

#import<CocoaLumberjack/CocoaLumberjack.h>

#endif


#ifdef DEBUG

static const DDLogLevel ddLogLevel =DDLogLevelVerbose;

#else

static const DDLogLevel ddLogLevel = DDLogLevelOff;

#endif

UNITYSENDMSG m_unitysendmsg = NULL;

void setUnityVoiceSendmsg(UNITYSENDMSG unitymsg)
{
    m_unitysendmsg=unitymsg;
}

@interface UnityVoiceMsgObj: NSObject
{
    
}
@property(nonatomic,strong)NSString *functionName;
@property(nonatomic,strong)NSString *UnityObject;
@property(nonatomic,strong)NSString *UnityFunSucces;
@property(nonatomic,strong)NSString *UnityFunFail;
@property(nonatomic,strong)NSString *serverip;
@property(nonatomic,strong)NSString *username;

@end

@implementation UnityVoiceMsgObj

@end

static const  NSDictionary *errcodedic = @{@"10001":@"没有摄像头访问权限",@"10002":@"没有读文件的权限",@"10003":@"没有写文件的权限",@"10004":@"没有使用闪光灯权限",@"10005":@"没有外部存储卡读写权限",@"10006":@"没有使用麦克风权限",@"10008":@"没有访问网络的权限",@"11001":@"文件不存在",@"11002":@"文件格式错误",@"11003":@"文件有破损，无法解析",@"11004":@"存储空间不足，无法写入",@"12001":@"网络请求异常，状态码",@"12002":@"返回数据的格式错误",@"40001":@"不合法的调用凭证",@"40002":@"不合法的grant_type",@"40003":@"不合法的OpenID",@"40004":@"不合法的媒体文件类型",@"40007":@"不合法的media_id",@"40008":@"不合法的message_type",@"40009":@"不合法的图片大小",@"40010":@"不合法的语音大小",@"40011":@"不合法的视频大小",@"40012":@"不合法的缩略图大小",@"40013":@"不合法的AppID",@"40014":@"不合法的access_token",@"40020":@"不合法的url长度",@"41001":@"缺失access_token参数",@"41002":@"缺失appid参数",@"41003":@"缺失refresh_token参数",@"41004":@"缺失secret参数",@"41005":@"缺失二进制媒体文件",@"41006":@"缺失media_id参数",@"50001":@"接口未授权"};


NSMutableDictionary *unityMsgdict=[[NSMutableDictionary alloc]init];

static UnityVoiceMsgObj *m_UnityVoiceMsgObj=[[UnityVoiceMsgObj alloc]init];

void sendUnityVoiceSendmsg(bool issucceed,const char *msg)
{
    const char *cunityObj = [m_UnityVoiceMsgObj.UnityObject UTF8String];
    const char *unityFunSucces = [m_UnityVoiceMsgObj.UnityFunSucces UTF8String];
    const char *unityFunFail = [m_UnityVoiceMsgObj.UnityFunFail UTF8String];
    
    if (m_unitysendmsg)
    {
        if (issucceed)
        {
            m_unitysendmsg(cunityObj,unityFunSucces, msg);
        }
        else
        {
            
            m_unitysendmsg(cunityObj,unityFunFail, msg);
        }
        
    }
    else
    {
        //        NSLog(@"liveRTMP m_unitysendmsg=NULL");
        
    }
}

void initDDlog()
{
    [DDLog addLogger:[DDTTYLogger sharedInstance]];
    
    [[DDTTYLogger sharedInstance] setColorsEnabled:YES];
    
    DDFileLogger *fileLogger = [[DDFileLogger alloc] init];
    
    fileLogger.rollingFrequency = 60 * 60 * 24; // 24 hour rolling
    
    fileLogger.logFileManager.maximumNumberOfLogFiles = 7;
    
    [DDLog addLogger:fileLogger];
    
    //    DDLogError(@"Paper jam%@", @"dsfsd");
    //
    //    DDLogWarn(@"Toner is low");
    //
    //    DDLogInfo(@"Warming up printer (pre-customization)");
    //
    //    DDLogVerbose(@"Intializing protcol x26 (pre-customization)");
    //     DDLogError(@"Paper jam");
    //
    //    DDLogWarn(@"Toner is low");
    
#if TARGET_OS_IPHONE
    
    UIColor *pink = [UIColor colorWithRed:(255/255.0) green:(58/255.0) blue:(159/255.0) alpha:1.0];
    
#else
    
    NSColor *pink = [NSColor colorWithCalibratedRed:(255/255.0) green:(58/255.0) blue:(159/255.0) alpha:1.0];
    
#endif
    
    [[DDTTYLogger sharedInstance] setForegroundColor:pink backgroundColor:nil forFlag:DDLogFlagInfo];
    
    //    DDLogInfo(@"Warming up printer (post-customization)");
    //
    //    DDLogDebug(@"DDLogDebug");
    
#if TARGET_OS_IPHONE
    
    UIColor *gray = [UIColor grayColor];
    
#else
    
    NSColor *gray = [NSColor grayColor];
    
#endif
    
    [[DDTTYLogger sharedInstance] setForegroundColor:gray backgroundColor:nil forFlag:DDLogFlagVerbose];
    
    //    DDLogVerbose(@"Intializing protcol x26 (post-customization)");
    
}

class INetWork
:public INetWorkCallback
{
public:
    virtual void IConnectStatusCallback(CONNECT_STATUS cs);
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList);
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName);
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID);
    virtual void INetReceiveData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
    virtual void INetAudioStatus(unsigned long uPeerUserID,bool isRoom,AUDIO_SEND_STATUS AudioStatus,std::string strName) ;
    
};

static INetWork * m_INetWork =NULL;
static INetWorkCallback * m_SuperINetWork =NULL;

void initVoiceManage(const char* unityObject,const char* UnityFunSucces,const char* UnityFunFail)
{

    m_SuperINetWork =NULL;
    m_UnityVoiceMsgObj.UnityObject=[NSString stringWithFormat:@"%s",unityObject];
    m_UnityVoiceMsgObj.UnityFunSucces=[NSString stringWithFormat:@"%s",UnityFunSucces];
    m_UnityVoiceMsgObj.UnityFunFail=[NSString stringWithFormat:@"%s",UnityFunFail];
    m_UnityVoiceMsgObj.functionName=[NSString stringWithFormat:@"VoiceManageCallback"];
    
    initDDlog();
    
    //    DDLogError(@"错误信息"); // 红色
    //    DDLogWarn(@"警告"); // 橙色
    //    DDLogInfo(@"提示信息"); // 默认是黑色
    //    DDLogVerbose(@"详细信息"); // 默认是黑色
}

int   VoiceManageCallback (int msgcode,bool issucceed,const char * funtype,const char *status,const char *username,const char *inRoom)
{
    
    NSString *smsg=nil;
    if (issucceed&&msgcode==0)
    {
        if(strcmp(status,"0")==0||strcmp(status,"1")==0)
        {
            smsg=[NSString stringWithFormat:@"{\"status\":\"%s\",\"username\":\"%s\",\"inRoom\":\"%s\"}",status,username,inRoom];
        }
        else
        {
            smsg=[NSString stringWithFormat:@"{\"code\":\"%@\",\"message\":\"%@\"}", @"0",@"OK"];
        }
        
    }
    else
    {
        NSString *strmsgcode=[NSString stringWithFormat:@"%d",msgcode];
        NSString *strmsg = [errcodedic objectForKey:strmsgcode];
        if(strmsg)
            smsg=[NSString stringWithFormat:@"{\"code\":\"%@\",\"message\":\"%@\"}", strmsgcode,strmsg];
        else
        {
            smsg=[NSString stringWithFormat:@"{\"code\":\"%@\",\"message\":\"%@\"}",strmsgcode,@"未知错误"];
            
        }
        
    }
    NSString *strtomsg=[NSString stringWithFormat:@"{\"callBackFun\":\"%s\",\"retMsg\":%@}",funtype,smsg];
    sendUnityVoiceSendmsg(issucceed,[strtomsg UTF8String]);
    
    
    return 0;
}

void INetWork::IConnectStatusCallback(CONNECT_STATUS cs)
{
    
    if (m_SuperINetWork) {
        m_SuperINetWork->IConnectStatusCallback(cs);
    }
    
    NSString *strstus=nil;
    switch (cs)
    {
        case CS_CONNECTING:
            
            strstus = @"正在连接服务器。。。\n";
            
            break;
        case CS_FAILED:
            strstus = @"连接失败！\n";
            break;
        case CS_DISCONNECTED:
            strstus = @"断开连接！\n";
            break;
        case CS_CONNECTED:
        {
            strstus = @"已经连接！\n";
            
        }
            break;
        case CS_BUSY:
            strstus = @"服务器忙！\n";
            
            break;
        case CS_RECONNECTED:
            strstus = @"重新连接服务器！\n";
            
            break;
        case CS_RESTARTED:
            NSLog(@"已重新连接！\n");
            break;
        case CS_LOGINED:
        {
       
            strstus = @"登录 成功！\n";
        }
            break;
        case CS_LOGINFAILED:
            strstus = @"登录失败！\n";
            break;
        default:
            break;
    }
    
    NSString *stttus = [NSString stringWithFormat:@"%d",cs];
        const char *    strnull = "";
    VoiceManageCallback(0,YES,"MultiVoiceConnectStatus",[stttus UTF8String],[strstus UTF8String],strnull);
    DDLogInfo(@"服务器连接状态 = %d %@\n",cs,strstus);
}


void INetWork::INetAudioStatus(unsigned long uPeerUserID, bool isRoom, AUDIO_SEND_STATUS AudioStatus,std::string strName)
{
    
   if (m_SuperINetWork)
   {
        m_SuperINetWork->INetAudioStatus(uPeerUserID,isRoom,AudioStatus,strName);
    }
}

void INetWork::INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
    //
    //    if (m_viewController) {
    //           dispatch_async(dispatch_get_main_queue(), ^{
    //        [m_viewController INetReceiveUserList:UserInfoList];
    //           });
    //    }
}

void INetWork::INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName)
{
    //    if (m_viewController) {
    //           dispatch_async(dispatch_get_main_queue(), ^{
    //        [m_viewController INetReceiveUserLogin:uPeerUserID strName:strName];
    //                    });
    //}
    
}

void INetWork::INetReceiveUserLogOut(unsigned long uPeerUserID)
{
    
    //    if (m_viewController) {
    //        [m_viewController INetReceiveUserLogOut:uPeerUserID];
    //    }
    
}

void INetWork::INetReceiveData(unsigned long uPeerUserID, const char* pData, unsigned long nLen)
{
    //
    //    if (m_viewController) {
    //                   dispatch_async(dispatch_get_main_queue(), ^{
    //        [m_viewController INetReceiveData:uPeerUserID pData:pData nLen:nLen];
    //                   });
    //    }
}
void INetWork::INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen)
{
    //    if (m_viewController) {
    //                   dispatch_async(dispatch_get_main_queue(), ^{
    //        [m_viewController INetBroadcastData:uPeerUserID pData:pData nLen:nLen];
    //                   });
    //    }
    
}

static  CVoiceManage* 	m_voiceManage =  NULL;

void  loginVoiceServer(const char* szIP,const char* szName,const char* szGameId,const char* szGameServerId,const char* szRoomId,const char* szGroupId,BOOL listenInRoom)
{
    
    if(m_voiceManage)
    {
        m_voiceManage->logoutServer();
        m_voiceManage->disconnect();
        delete  m_voiceManage;
        
        m_voiceManage = NULL;
    }
     m_voiceManage = new CVoiceManage(VoiceManageCallback);
    
    if (m_INetWork) {
        delete m_INetWork;
        m_INetWork =NULL;
    }
    
    m_INetWork = new INetWork;
 
    m_voiceManage->setNetWorkCallback(m_INetWork);
    
    if(m_voiceManage)
    {
        m_UnityVoiceMsgObj.serverip =[NSString stringWithFormat:@"%s",szIP];
        m_UnityVoiceMsgObj.username =[NSString stringWithFormat:@"%s",szName];
        m_voiceManage->loginVoiceServer(szIP, szName,szGameId,szGameServerId,szRoomId,szGroupId,listenInRoom);
    }
}

void  logoutServer()
{
      if(m_voiceManage)
    {
        m_voiceManage->logoutServer();
        usleep(500000);
        m_voiceManage->disconnect();
        delete  m_voiceManage;
        m_voiceManage = NULL;
    }
    if (m_INetWork) {
        delete m_INetWork;
        m_INetWork =NULL;
    }
    m_UnityVoiceMsgObj.serverip = nil;
    m_UnityVoiceMsgObj.username = nil;
    
}

void  startSendAudio(BOOL isspeakinroom)
{
    if(m_voiceManage)
    {
        m_voiceManage->startSendAudio(isspeakinroom);
        
    }
     usleep(100000);
}

void  stopSendAudio()
{
    if(m_voiceManage)
    {
        m_voiceManage->stopSendAudio();
        
    }
    usleep(100000);
}

void  setNetWorkCallback(void*netWorkCallback)
{
    if(m_voiceManage&&netWorkCallback)
    {
       m_SuperINetWork = static_cast<INetWorkCallback *>(netWorkCallback);
        
//        m_SuperINetWork  = (INetWorkCallback*)netWorkCallback;
        
    }
}
