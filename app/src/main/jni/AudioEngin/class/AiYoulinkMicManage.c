//
//  AiYouLinkMicManage.c
//  LinkMicTalk
//
//  Created by zhangxinming on 16/7/21.
//  Copyright © 2016年 zhangxinming. All rights reserved.
//

#include "AiYoulinkMicManage.h"
#include "CLinkMicManage.h"
#include <string.h>
#import <UIKit/UIKit.h>
#import "TouchMoveView.h"
#import <AVFoundation/AVFoundation.h>

id<LinkMicNetWorkDelegate> m_NetWorkDelegate = nil;

static CLinkMicManage   *m_LinkMicManage    =  NULL;
static TouchMoveView  *m_touchMoveView  =  nil;
static UIView         *m_localVideo     =  nil;
static UIView         *m_peerVideo      =  nil;

class ILinkMic
:public ILinkMicCallback
{
public:
    virtual void IConnectStatusCallback(LINKMIC_STATUS cs);
    virtual void INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList);
    virtual void INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long useraudioId);
    virtual void INetReceiveUserLogOut(unsigned long uPeerUserID);
    virtual void INetReceiveData(unsigned long uPeerUserID, std::string strName, std::string strData, unsigned long nLen);
    virtual void INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen);
 
 
    
};

static ILinkMic * m_ILinkMic =NULL;


void ILinkMic::IConnectStatusCallback(LINKMIC_STATUS cs)
{
   
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(IConnectStatusCallback:)]) {
        
        [m_NetWorkDelegate IConnectStatusCallback:cs];
    
    }
    
   
}



void ILinkMic::INetReceiveUserList(CLIENTUSERINFOLIST_MAP& UserInfoList)
{
    
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(INetReceiveUserList:)]) {
        
        [m_NetWorkDelegate INetReceiveUserList:UserInfoList];
        
    }
   
}

void ILinkMic::INetReceiveUserLogin(unsigned long uPeerUserID, std::string strName,unsigned long userAudioID)
{
   
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(INetReceiveUserLogin:strName:useraudioID:)]) {
        
        [m_NetWorkDelegate INetReceiveUserLogin:uPeerUserID strName:strName useraudioID:userAudioID];
        
    }
    
}

void ILinkMic::INetReceiveUserLogOut(unsigned long uPeerUserID)
{
    
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(INetReceiveUserLogOut:)]) {
        
        [m_NetWorkDelegate INetReceiveUserLogOut:uPeerUserID ];
        
    }
   
}

void ILinkMic::INetReceiveData(unsigned long uPeerUserID, std::string strName, std::string strData, unsigned long nLen)
{
    
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(INetReceiveData:strName:strData:nLen:)]) {
        
        [m_NetWorkDelegate INetReceiveData:uPeerUserID strName:strName.c_str() strData:strData.c_str() nLen:nLen];
        
    }
    
    
}
void ILinkMic::INetBroadcastData(unsigned long uPeerUserID, const char* pData, unsigned long nLen)
{
   
    
    if (m_NetWorkDelegate &&[m_NetWorkDelegate respondsToSelector:@selector(INetBroadcastData:pData:nLen:)]) {
        
        [m_NetWorkDelegate INetBroadcastData:uPeerUserID pData:pData  nLen:nLen];
        
    }
}

bool checkLinkMicAudio()
{
    AVAudioSession *avSession = [AVAudioSession sharedInstance];
    
    __block bool ret=YES;
    
    if ([avSession respondsToSelector:@selector(requestRecordPermission:)]) {
        
        [avSession requestRecordPermission:^(BOOL available) {
         ret= available;
         if (available) {
     
         }
         else
         {
         dispatch_async(dispatch_get_main_queue(), ^{
            [[[UIAlertView alloc] initWithTitle:@"无法录音" message:@"请在“设置-隐私-麦克风”选项中允许融企信访问你的麦克风" delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil] show];
            //        return available;
        });
         }
         }];
        
    }
    return ret;
}

bool checkLinkMicVideo()
{
    NSString *mediaType = AVMediaTypeVideo;// Or AVMediaTypeAudio
    AVAuthorizationStatus authStatus = [AVCaptureDevice authorizationStatusForMediaType:mediaType];
    
    // This status is normally not visible—the AVCaptureDevice class methods for discovering devices do not return devices the user is restricted from accessing.
    if(authStatus ==AVAuthorizationStatusAuthorized){
        NSLog(@"Restricted");
        
        return YES;
        
    }else if(authStatus == AVAuthorizationStatusDenied){
        // The user has explicitly denied permission for media capture.
        NSLog(@"Denied");     //应该是这个，如果不允许的话
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"提示"
                                                        message:@"请在设备的\"设置-隐私-相机\"中允许访问相机。"
                                                       delegate:nil
                                              cancelButtonTitle:@"确定"
                                              otherButtonTitles:nil];
        [alert show];
        return NO;
    }
    return YES;
}
 

void loginLinkMicServer(const char* szServerHostIp,const char* szUserName,const char* szRoomId,bool isencrypt, bool isAnchor)
{
    
    
    if(!isAnchor)
    {
        if(!checkLinkMicAudio())
            return;
        
        
        if(!checkLinkMicVideo())
            return;
        
    }
    
   if(m_LinkMicManage)
    {
        m_LinkMicManage->logoutServer();
        usleep(500);
        m_LinkMicManage->disconnect();
        delete  m_LinkMicManage;
        
        m_LinkMicManage = NULL;
    }
    
  
 
    
    m_LinkMicManage = new CLinkMicManage();
    
    if (m_ILinkMic) {
        delete m_ILinkMic;
        m_ILinkMic =NULL;
    }
    
    m_ILinkMic = new ILinkMic;
    
    m_LinkMicManage->setNetWorkCallback(m_ILinkMic);
    
    if(m_LinkMicManage)
    {
 
        m_LinkMicManage->loginLinkMicServer(szServerHostIp, szUserName,"LinkMic-AY","LinkMic-AY",szRoomId,"AYLinkMic-AY","NULL",isencrypt,NO,isAnchor);
    }
}


void  openLinkMicVideoWtihName(const char *name)
{
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        if(m_LinkMicManage)
        {
            m_LinkMicManage->OpenRemoteUserVideo(name);
            
        }
    });
    
}

void  closeLinkMicVideoWtihName(const char *name)
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        
        if(m_LinkMicManage)
        {
            m_LinkMicManage->closeRemoteUserVideo(name);
            
        }
    });
    
}


void  logoutLinkMicServer()
{
    if(m_LinkMicManage)
    {
        m_LinkMicManage->logoutServer();
        usleep(500000);
        m_LinkMicManage->disconnect();
        delete  m_LinkMicManage;
        m_LinkMicManage = NULL;
    }
    if (m_ILinkMic) {
        delete m_ILinkMic;
        m_ILinkMic =NULL;
    }
 
    
}

int SendDataToUser(unsigned long uPeerUserID ,const char* pData, unsigned long nLen)
{
    if(m_LinkMicManage)
    {
        m_LinkMicManage->SendDataToUser(uPeerUserID,pData,nLen);
        
    }
    return 0;
}



    void  setLinkMicNetWorkDelegate( id<LinkMicNetWorkDelegate> delegate)
{
    m_NetWorkDelegate = delegate;
}

void creatVideoWindow(int x,int y,int w,int h)
{
    UIViewController *rootvc = nil;
    
    UIWindow * window = [[UIApplication sharedApplication] keyWindow];
    if (window.windowLevel != UIWindowLevelNormal)
    {
        NSArray *windows = [[UIApplication sharedApplication] windows];
        for(UIWindow * tmpWin in windows)
        {
            if (tmpWin.windowLevel == UIWindowLevelNormal)
            {
                window = tmpWin;
                break;
            }
        }
    }
    
    UIView *frontView = [[window subviews] objectAtIndex:0];
    id nextResponder = [frontView nextResponder];
    
    if ([nextResponder isKindOfClass:[UIViewController class]])
        rootvc = nextResponder;
    else
        rootvc = window.rootViewController;
    
    if (m_touchMoveView) {
        
        [m_touchMoveView removeFromSuperview];
        m_touchMoveView = nil;
    }
    
    m_touchMoveView = [[TouchMoveView alloc]initWithFrame:CGRectMake(x, y, w, h) ];
    m_touchMoveView.userInteractionEnabled=YES;

    CGRect frame = m_touchMoveView.frame;
    
    m_touchMoveView.kWidth = frame.size.width;
    m_touchMoveView.kHeight = frame.size.height;
    
    m_localVideo =[[UIView alloc]init];
    m_peerVideo =[[UIView alloc]init];
    

    m_localVideo.translatesAutoresizingMaskIntoConstraints=NO;
    [m_peerVideo setFrame:frame];
    [m_peerVideo setBackgroundColor:[UIColor greenColor]];
    
    [m_touchMoveView addSubview:m_peerVideo];
    
        UIInterfaceOrientation  tion=  [[UIApplication sharedApplication] statusBarOrientation];
    
    
        if (tion == UIInterfaceOrientationLandscapeLeft )
        {
 
             CGRect tframe = m_touchMoveView.frame;
            
            frame.size.height =tframe.size.width*2/5;
            
            frame.size.width= tframe.size.height*2/5;
            int ab=fabs(frame.size.height - frame.size.width );
 
             frame.origin.x= tframe.origin.x + tframe.size.width-frame.size.width-ab;
            
             frame.origin.y= tframe.origin.y + tframe.size.height-frame.size.height+ab;

            NSLog(@"w=%f,h=%f",frame.size.width,frame.size.height);
     
        }
    
        else if (tion == UIInterfaceOrientationLandscapeRight )
        {
            CGRect tframe = m_touchMoveView.frame;
            frame.size.height =tframe.size.width*2/5;
            frame.size.width= tframe.size.height*2/5;
            int ab=fabs(frame.size.height - frame.size.width );
            
            frame.origin.y= tframe.origin.y + tframe.size.height-frame.size.height-ab;
            
            frame.origin.x= tframe.origin.x + tframe.size.width-frame.size.width+ab;
  
        }
        else
        {
            
            frame.origin.x= frame.origin.x + frame.size.width*3/5;
            frame.origin.y= frame.origin.y + frame.size.height*3/5;
            frame.size.width =frame.size.width*2/5;
            frame.size.height= frame.size.height*2/5;
        }
 
    [m_touchMoveView addSubview:m_localVideo];
    
    [m_localVideo setFrame:frame];
    m_localVideo.userInteractionEnabled=YES;
    
    [ rootvc.view addSubview:m_touchMoveView];
    
    [m_touchMoveView setHidden:YES];
}

void resetLocalVideo()
{
    if (m_LinkMicManage) {
        m_LinkMicManage->resetLocalVideo();
    }
}


