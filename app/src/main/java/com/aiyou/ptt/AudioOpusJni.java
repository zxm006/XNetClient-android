package com.aiyou.ptt;

import java.util.ArrayList;

import android.util.Log;

public class AudioOpusJni {
	private final static String TAG = AudioOpusJni.class.getSimpleName();
	
	static {
		System.loadLibrary("XNet");
		System.loadLibrary("MultvoiceEngine");
		System.loadLibrary("opus");
//		System.loadLibrary("VideoEngine");
	}
	
	public native int audioModuleInit(Object context);
	
	public native int pttLogin(String IP, String userName, String psw, 
							String gameID, String gameServID, String roomID, String groupID,
							int iChanal, int sampleRate, int iBitRate, boolean bListenInRoom, 
							String expend, boolean isencrypt,
							String headUrl, String nickName, double jingdu, double weidu);
	public native int pttLogout();
	public native int setVolumeEnable(boolean isInRoom);
	
	public native int startSendAudio(boolean isInRoom);
	public native int stopSendAudio();
	
	public native int startRecvAudio();
	public native int stopRecvAudio();
	
	public native ArrayList<ConfUserInfo> pttGetUpUserList();
	
	private static TalkEventInterface mTalkInterface = null;
	public static void setTalkStatusCb(TalkEventInterface talkCb)
	{
		mTalkInterface = talkCb;
	}
	
	public void onVolumeChanged(int voice)
	{
		Log.i(TAG, "on Volume Changed in:"+voice);
		if(mTalkInterface != null)
		{
			mTalkInterface.volumeChanged(voice);
		}
	}
	
	public void onStatusChanged(int msgcode, boolean isSuccess, String funtype, String status, String username, String isInRoom)
	{
		Log.i(TAG, "onStatusChanged in:"+msgcode+","+isSuccess+","+funtype+","+status+","+username+","+isInRoom);
		if(mTalkInterface != null)
		{
			mTalkInterface.voiceStatusChanged(msgcode, isSuccess, funtype, status, username, isInRoom);
		}
	}
	
	public void onNetworkChanged(int networkStatus)
	{
		Log.i(TAG, "onNetwork Changed in:"+networkStatus);
		if(mTalkInterface != null)
		{
			mTalkInterface.voiceNetworkChanged(networkStatus);
		}
	}
	
	public void onTalkInviteIn(int userid, boolean isVideo, String username)
	{
		Log.i(TAG, "onTalk InviteIn in:"+userid+","+isVideo+","+username);
		if(mTalkInterface != null)
		{
			mTalkInterface.OnInviteIn(userid, isVideo, username);
		}
	}
	
	public void onTalkConnected()
	{
		Log.i(TAG, "onTalk Connected");
		if(mTalkInterface != null)
		{
			mTalkInterface.OnConnected();
		}
	}
	
	public void onTalkDisConnected(int iReason, String info)
	{
		Log.i(TAG, "onTalk Dis Connected in:"+iReason+",info:"+info);
		if(mTalkInterface != null)
		{
			mTalkInterface.OnDisConnected(iReason, info);
		}
	}
	
	//对讲用户列表上报
	public void onPttUserList()
	{
		Log.i(TAG, "ptt user list in");
		ArrayList<ConfUserInfo> list = pttGetUpUserList();
		if(mTalkInterface != null)
		{
			Log.i(TAG, "user list size:"+(list != null ? list.size() : "empty"));
			mTalkInterface.onUserListUped(list);
		}
	}
	
	//用户状态变更，目前就只是登录或退出登录的状态
	public void onPttUserStatusChanged(String userid, String head, String name, String jingdu, String weidu, String tm, int status)
	{
		Log.i(TAG, "ptt user status in, status:"+status+",user:"+userid);
		if(mTalkInterface != null)
		{
			ConfUserInfo info = new ConfUserInfo(userid, head, name, jingdu, weidu, tm);
			mTalkInterface.onUserStatusChanged(info, status);
		}
	}
	
	
	/*********************视频通话*****************************/
	public native int talkCallout(boolean isVideo, String username, Object localVideo, Object peerVideo);
	public native int talkAccept(Object localVideo, Object peerVideo);
	public native int talkCancel();
	public native int talkRefuse();
	public native int talkHangup();
	
	
	/***********************连麦****************************/
	public native int LinkMicLogin(String IP, String userName, String psw, 
			String gameID, String gameServID, String roomID, String groupID,
			int iChanal, int sampleRate, int iBitRate, boolean isencrypt, boolean isAnchor, String expend);
	public native int LinkMicLogout();
	public native int LinkMicOpenRemoteVideo(String remoteName, Object remoteView);
	public native int LinkMicResetRemoteVideo(String remoteName, Object remoteView);
	public native int LinkMicCloseRemoteVideo(String remoteName);
	public native int LinkMicOpenLocalVideo(Object localView);
	public native int LinkMicCloseLocalVideo();
	
	private static LinkMicEventInterface mLinkMicCb = null;
	public static void setLinkMicCb(LinkMicEventInterface cb)
	{
		mLinkMicCb = cb;
	}
	
	//自己的网络状态变化通知
	public void onLinkMicNetworkChanged(int networkStatus)
	{
		Log.i(TAG, "link mic onNetwork Changed in:"+networkStatus);
		if(mLinkMicCb != null)
		{
			mLinkMicCb.LinkMicNetworkChanged(networkStatus);
		}
	}
	
	//连麦的用户登录成功
	public void onLinkMicUserLogin(int peerUserID, String peerName, int seraudioID)
	{
		Log.i(TAG, String.format("link mic user login:%d,%s,%d",peerUserID, peerName, seraudioID));
		if(mLinkMicCb != null)
		{
			mLinkMicCb.LinkMicUserLogin(peerUserID, peerName, seraudioID);
		}
	}
	
	//连麦的用户退出登录
	public void onLinkMicUserLogout(int peerUserID, String peerName)
	{
		Log.i(TAG, String.format("link mic user logout:%d,%s",peerUserID, peerName));
		if(mLinkMicCb != null)
		{
			mLinkMicCb.LinkMicUserLogout(peerUserID, peerName);
		}
	}
	
	/********************视频会议*********************/
	public native int ConfLogin(String IP, String userName, String psw, 
			String gameID, String gameServID, String roomID, String groupID,
			int iChanal, int sampleRate, int iBitRate, boolean isencrypt, String expend,
			String headUrl, String nickName, double jingdu, double weidu);
	public native int ConfLogout();
	public native int ConfCreate(String confid, boolean isOpenCamera, int type);
	public native int ConfJoin(String confid, boolean isOpenCamera);
	public native int ConfExit();
	public native int ConfDestroy();
	public native int ConfInviteMember(String confid, String userid);
	public native int ConfKickMember(String confid, String userid);
	public native int ConfLocalVideoSetParam(int width, int height, int bitrate, int framerate);
	public native int ConfLocalVideoOpen(Object localView);
	public native int ConfLocalVideoClose();
	public native int ConfPeerVideoOpen(Object peerView, String peerName);
	public native int ConfPeerVideoClose(String peerName);
	
	public native int ConfLocalMicMute(boolean isMute);
	public native int ConfPeerMicMute(String peerName, boolean isMute);
	public native int ConfLocalVideoSwitch();
	public native int ConfLocalNetworkChanged();
	public native ArrayList<ConfUserInfo> ConfGetUpUserList();
	
	private static ConfEventInterface mConfCb = null;
	public static void setConfCb(ConfEventInterface cb)
	{
		mConfCb = cb;
	}
	
	public void onConfStatus(int confStatus, String peerName, String head, String nickName, String jingdu, String weidu, String tm)
	{
		Log.i(TAG, "conf status Changed in:"+confStatus+",name:"+peerName);
		if(mConfCb != null)
		{
			mConfCb.onConfStatusChanged(confStatus, peerName, head, nickName, jingdu, weidu, tm);
		}
	}
	
	//自己的网络状态变化通知
	public void onConfNetworkChanged(int networkStatus)
	{
		Log.i(TAG, "conf onNetwork Changed in:"+networkStatus);
		if(mConfCb != null)
		{
			mConfCb.onConfNetworkChanged(networkStatus);
		}
	}
	
	public void onConfUserLogin(int peerUserID, String peerName, int seraudioID)
	{
		Log.i(TAG, String.format("conf user login:%d,%s,%d",peerUserID, peerName, seraudioID));
		if(mConfCb != null)
		{
			mConfCb.onConfUserLogin(peerUserID, peerName, seraudioID);
		}
	}
	
	public void onConfUserLogout(int peerUserID, String peerName)
	{
		Log.i(TAG, String.format("conf user logout:%d,%s",peerUserID, peerName));
		if(mConfCb != null)
		{
			mConfCb.onConfUserLogout(peerUserID, peerName);
		}
	}
	
	//需要显示视频窗口的回调
	public void onConfVideoWndNeed(String userid)
	{
		Log.i(TAG, "conf video wnd need, id:"+userid);
		if(mConfCb != null)
		{
			mConfCb.onConfVideoWndNeed(userid);
		}
	}
	
	//用户媒体状态发生变化通知
	public void onConfUserMediaStatus(String userid, boolean isvideo, boolean isopen)
	{
		Log.i(TAG, "conf user status in:"+userid+","+isvideo+","+isopen);
		if(mConfCb != null)
		{
			mConfCb.onConfUserMediaStatus(userid, isvideo, isopen);
		}
	}
	
	//收到会议邀请
	public void onConfInviteIn(String userid, String confid)
	{
		Log.i(TAG, "conf invite in:"+userid+","+confid);
		if(mConfCb != null)
		{
			mConfCb.onConfInviteIn(userid, confid);
		}
	}
	
	//被提出会议
	public void onConfBeKicked(String userid, String confid)
	{
		Log.i(TAG, "conf bekicked in:"+userid+","+confid);
		if(mConfCb != null)
		{
			mConfCb.onConfBeKicked(userid, confid);
		}
	}
	
	//会议用户列表上报
	public void onConfUserList()
	{
		Log.i(TAG, "conf user list in");
		ArrayList<ConfUserInfo> list = ConfGetUpUserList();
		if(mConfCb != null)
		{
			mConfCb.onConfUserListUped(list);
		}
	}
	
}
