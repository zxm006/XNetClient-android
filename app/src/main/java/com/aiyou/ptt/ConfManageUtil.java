package com.aiyou.ptt;

import java.util.ArrayList;
import java.util.UUID;

import android.R.bool;
import android.content.Context;
import android.text.TextUtils;
import android.util.Log;
import android.view.TextureView;

public class ConfManageUtil implements ConfEventInterface{

	private static String TAG = ConfManageUtil.class.getSimpleName();
	private AudioOpusJni	mJniObj;
	private static ConfManageUtil mInstance;
	private Context mContext;
	
	private ArrayList<ConfEventCallback> mCallbackList = new ArrayList<ConfEventCallback>();
	
	public static ConfManageUtil getInstance()
	{
		if(mInstance == null)
		{
			mInstance = new ConfManageUtil();
		}
		return mInstance;
	}
	
	private ConfManageUtil()
	{
		checkJniObj();
	}
	
	private void checkJniObj()
	{
		if(mJniObj == null)
		{
			mJniObj = new AudioOpusJni();
			AudioOpusJni.setConfCb(this);
		}
	}
	
	public void setContext(Context context)
	{
		mJniObj.audioModuleInit(context);
		mContext = context;
	}
	
	public void setCallback(ConfEventCallback cb)
	{
		mCallbackList.add(cb);
	}
	
	public void confLogin(String serverAddr, String number, String gameid, String serverid, 
			String roomid, String groupid, int iChanal, int sampleRate, int iBitRate, boolean isEncry,
			String headUrl, String nickName, double jingdu, double weidu)
	{
		if(TextUtils.isEmpty(serverAddr) || TextUtils.isEmpty(number))
		{
			Log.e(TAG, "input para is invalid");
			return;
		}
		if(iChanal != 1 && iChanal != 2) 
		{
			iChanal = 1;
		}
		if(sampleRate != 8000 && sampleRate != 16000 && sampleRate != 24000 && sampleRate != 44100 && sampleRate != 48000) 
		{
			sampleRate = 8000;
		}
		
		if(iBitRate < 2000 || iBitRate > 128000) 
		{
			iBitRate = 8000;
		}
		
		mJniObj.ConfLogin(serverAddr, number, number, gameid, serverid, 
				roomid, groupid, iChanal, sampleRate, iBitRate, isEncry, "",
				headUrl, nickName, jingdu, weidu);
	}
	
	public void confLogout()
	{
		mJniObj.ConfLogout();
	}

	public String confCreate(boolean isOpenCamera, String confid, int type)
	{
		if(TextUtils.isEmpty(confid))
		{
			confid = UUID.randomUUID().toString();
		}
		Log.i(TAG, "create conf conf id:"+confid);
		mJniObj.ConfCreate(confid, isOpenCamera, type);
		return confid;
	}
	
	public void confJoin(String confid, boolean isOpenCamera)
	{
		if(TextUtils.isEmpty(confid))
		{
			Log.e(TAG, "join conf input para is invalid");
			return;
		}
		mJniObj.ConfJoin(confid, isOpenCamera);
	}
	
	public void confExit()
	{
		mJniObj.ConfExit();
	}
	
	public void confDestroy()
	{
		mJniObj.ConfLocalVideoClose();
		mJniObj.ConfDestroy();
	}
	
	public void confInviteMember(String confid, String userid)
	{
		if(TextUtils.isEmpty(confid) || TextUtils.isEmpty(userid))
		{
			Log.e(TAG, "invite member, input para is invalid");
			return;
		}
		mJniObj.ConfInviteMember(confid, userid);
	}
	
	public void confKickMember(String confid, String userid)
	{
		if(TextUtils.isEmpty(confid) || TextUtils.isEmpty(userid))
		{
			Log.e(TAG, "kick member, input para is invalid");
			return;
		}
		mJniObj.ConfKickMember(confid, userid);
	}
	
	public void confLocalVideoOpen(int width, int height, int bitrate, int framerate)
	{
		Log.i(TAG, "this will set local video param");
		mJniObj.ConfLocalVideoSetParam(width, height, bitrate, framerate);
	}
	
	public void confLocalVideoOpen(Object localView)
	{
		Log.i(TAG, "this will open local video");
		if(localView == null)
		{
			Log.e(TAG, "input para is invalid");
			return;
		}
		
		mJniObj.ConfLocalVideoOpen(localView);
	}
	
	public void confLocalVideoClose()
	{
		Log.i(TAG, "this will close local video");
		mJniObj.ConfLocalVideoClose();
	}
	
	public void confPeerVideoOpen(Object peerView, String peerName)
	{
		Log.i(TAG, "this will open peer video:"+peerName);
		if(TextUtils.isEmpty(peerName) || peerView == null)
		{
			Log.e(TAG, "input para is invalid");
			return;
		}
		
		mJniObj.ConfPeerVideoOpen(peerView, peerName);
	}
	
	public void confPeerVideoClose(String peerName)
	{
		Log.i(TAG, "this will close peer video:"+peerName);
		if(TextUtils.isEmpty(peerName))
		{
			Log.e(TAG, "input para is invalid");
			return;
		}
		
		mJniObj.ConfPeerVideoClose(peerName);
	}
	
	public void confLocalAudioPause(boolean bPause)
	{
		Log.i(TAG, "this will pause local audio:"+bPause);
		
		mJniObj.ConfLocalMicMute(bPause);
	}
	
	public void confPeerAudioPause(String peerName, boolean bPause)
	{
		Log.i(TAG, "this will pause peer audio:"+peerName+","+bPause);
		
		mJniObj.ConfPeerMicMute(peerName, bPause);
	}
	
	public void confLocalVideoSwtich()
	{
		Log.i(TAG, "this will switch local camera");
		
		mJniObj.ConfLocalVideoSwitch();
	}
	
	public void confNetworkReconnetced()
	{
		Log.i(TAG, "this network changed");
		mJniObj.ConfLocalNetworkChanged();
	}
	
	@Override
	public int onConfNetworkChanged(int networkStatus) {
		Log.i(TAG, "conf netwoke status:"+networkStatus);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfNetworkChanged(networkStatus);
		}
		return 0;
	}

	@Override
	public int onConfUserLogin(int peerUserID, String peerName, int seraudioID) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int onConfUserLogout(int peerUserID, String peerName) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int onConfInviteIn(String userid, String confid) {
		Log.i(TAG, "conf invite in:"+userid+","+confid);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfInviteIn(userid, confid);
		}
		return 0;
	}

	@Override
	public int onConfVideoWndNeed(String userid) {
		Log.i(TAG, "conf wnd need:"+userid);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfVideoWndNeed(userid);
		}
		return 0;
	}

	@Override
	public int onConfUserMediaStatus(String userid, boolean isvideo, boolean isopen) {
		Log.i(TAG, "conf media status:"+userid+","+isvideo+","+isopen);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfUserMediaStatus(userid, isvideo, isopen);
		}
		return 0;
	}

	@Override
	public int onConfStatusChanged(int status, String peerName, String head, String nick, String jingdu, String weidu, String tm) {
		Log.i(TAG, "conf status:"+status+","+peerName);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			Log.i(TAG, "will cb to ui");
			mCallbackList.get(i).onConfStatusChanged(status, peerName, head, nick, jingdu, weidu, tm);
		}
		return 0;
	}

	@Override
	public int onConfBeKicked(String userid, String confid) {
		Log.i(TAG, "conf bekicked in:"+userid+","+confid);
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfBeKicked(userid, confid);
		}
		return 0;
	}

	@Override
	public int onConfUserListUped(ArrayList<ConfUserInfo> list)
	{
		Log.i(TAG, "Conf User List Uped in, cnt:"+(list != null ? list.size() : 0));
		for(int i = 0; mCallbackList != null && i < mCallbackList.size(); i++)
		{
			mCallbackList.get(i).onConfUserListUped(list);
		}
		return 0;
	}
	
}
