package com.aiyou.ptt;

import java.util.ArrayList;

import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

public class PttManageUtil implements TalkEventInterface{

	private static String TAG = PttManageUtil.class.getSimpleName();
	private AudioOpusJni 	mJniObj;
	
	private static PttManageUtil mInstance;
	
	public static PttManageUtil getInstance()
	{
		if(mInstance == null)
		{
			mInstance = new PttManageUtil();
		}
		return mInstance;
	}
	
	private PttManageUtil()
	{
		if(mJniObj == null)
		{
			mJniObj = new AudioOpusJni();
			AudioOpusJni.setTalkStatusCb(this);
		}
	}
	
	public void setContext(Context context)
	{
		mJniObj.audioModuleInit(context);
		//mContext = context;
	}
	
	/**
	 * 登录
	 * @param serverIp:服务器地址
	 * @param userName:用户名
	 * @param gameID:gameid
	 * @param gameServID:gameServerid
	 * @param roomID
	 * @param groupID
	 * @param bListenInRoom
	 */
	public int MultiVoiceLogin(String serverIp, String userName, String gameID, String gameServID, 
								String roomID, String groupID,boolean bListenInRoom, boolean isencrypt,
								String headUrl, String nickName, double jingdu, double weidu)
	{
		Log.i(TAG, "MultiVoice Login in, server ip:"+serverIp+",name:"+userName+",gameid:"+gameID
				+",gameServID:"+gameServID+",roomID:"+roomID+",groupid:"+groupID+",listenroom:"+bListenInRoom);
		if(TextUtils.isEmpty(serverIp) || TextUtils.isEmpty(userName))
		{
			String strNote = "input param is empty";
			Log.i(TAG, strNote);
			return -1;
		}
		
		int iChannels = 1;
		int iSampleRate = 16000;	//采样率
		int iBitRate = 8000;   		//opus编码速率,500-512000
		int iRet = mJniObj.pttLogin(serverIp, userName, "", gameID, gameServID, roomID, groupID, 
									iChannels, iSampleRate, iBitRate, bListenInRoom, "", isencrypt,
									headUrl, nickName, jingdu, weidu);
		if(iRet < 0)
		{
			Log.e(TAG, "ptt login failed, ret:"+iRet);
		}
		return iRet;
	}
	
	/**
	 * 注销
	 */
	public int MultiVoiceLogoff()
	{
		Log.i(TAG, "login out");
		mJniObj.pttLogout();
		String strNote = "login out ok";
		Log.i(TAG, strNote);
		
		return 0;
	}
	
	/**
	 * 开始发语音
	 */
	public int MultiVoiceSendAudio(boolean bInRoom)
	{
		Log.i(TAG, "sart send audio, inroom:"+bInRoom);
		int iRet = mJniObj.startSendAudio(bInRoom);
		if(iRet != 0)
		{
			String strNote = "cannot send audio";
			Log.i(TAG, strNote);
		}
		else
		{
			String strNote = "start send audio ok";
			Log.i(TAG, strNote);
		}
		
		if(mEventCallback != null)
		{
			mEventCallback.pttEventMyselSpeakerRlt((iRet == 0) ? true : false);
		}
		
		return iRet;
	}

	/**
	 * 停止发语音
	 */
	public int MultiVoiceStopAudio()
	{
		Log.i(TAG, "stop send audio");
		int iRet = mJniObj.stopSendAudio();
		if(iRet != 0)
		{
			String strNote = "stop send audio failed";
			Log.i(TAG, strNote);
		}
		else
		{
			String strNote = "stop send audio ok";
			Log.i(TAG, strNote);
		}
		return iRet;
	}
	
	public int MultiVoiceEnableVolume(boolean bEnable)
	{
		Log.i(TAG, "enable volume flag:"+bEnable);
		int iRet = mJniObj.setVolumeEnable(bEnable);
		return iRet;
	}

	/**
	 * 状态通知
	 */
	@Override
	public int voiceStatusChanged(int msgcode, boolean isSuccess, String funtype,
								String status, String username, String inRoom) 
	{
		Log.i(TAG, "voiceStatusChanged in:"+msgcode+","+isSuccess+","+funtype+","+status+","+username+","+inRoom);
		if(funtype.equals("MultiVoiceLogin"))
		{
			if(status.equals("SUCCESS") || status.equals("OK"))
			{
				Log.i(TAG, "login ok");
				mJniObj.startRecvAudio();
				
				if(mEventCallback != null)
				{
					mEventCallback.pttEventloginRlt(true, 0, "login ok");
				}
			}
			else
			{
				Log.i(TAG, "login failed");
				if(mEventCallback != null)
				{
					mEventCallback.pttEventloginRlt(false, -1, "login failed");
				}
			}
		}
		else if(funtype.equals("MultiVoiceStatus"))
		{
			boolean canSpeak = false;
			if(status.equals("1"))
			{
				Log.i(TAG, "there has no speaker");
				canSpeak = true;
			}
			else
			{
				canSpeak = false;
				Log.i(TAG, "there has one speaker:"+username);
			}
			if(mEventCallback != null)
			{
				mEventCallback.pttEventSpeakerStatus(!canSpeak, username, inRoom);
			}
		}
		return 0;
	}

	@Override
	public int voiceNetworkChanged(int networkStatus) {
		Log.i(TAG, "voice Network Changed in:"+networkStatus);
		if(mEventCallback != null)
		{
			mEventCallback.pttEventNetworkChanged(networkStatus);
		}
		return 0;
	}
	
	private PttEventCallback mEventCallback;
	public void setEventCallback(PttEventCallback cb)
	{
		Log.i(TAG, "set event cb");
		mEventCallback = cb;
	}

	@Override
	public int OnInviteIn(long userid, boolean isVideo, String username) {
		return 0;
	}

	@Override
	public int OnConnected() {
		return 0;
	}

	@Override
	public int OnDisConnected(int iReason, String info) {
		return 0;
	}

	@Override
	public int volumeChanged(int voice) {
		if(mEventCallback != null)
		{
			mEventCallback.onVolumeChanged(voice);
		}
		return 0;
	}

	@Override
	public int onUserListUped(ArrayList<ConfUserInfo> list) {
		if(mEventCallback != null)
		{
			mEventCallback.onUserListUped(list);
		}
		return 0;
	}

	@Override
	public int onUserStatusChanged(ConfUserInfo user, int status) {
		if(mEventCallback != null)
		{
			mEventCallback.onUserStatusChanged(user, status);
		}
		return 0;
	}
}
