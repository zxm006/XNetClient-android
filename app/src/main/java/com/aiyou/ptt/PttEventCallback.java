package com.aiyou.ptt;

import java.util.ArrayList;

public interface PttEventCallback {
	public int pttEventloginRlt(boolean isLoginOk, int iErrCode, String errInfo);
	public int pttEventSpeakerStatus(boolean hasSpeaker, String speaker, String inroom);
	public int pttEventMyselSpeakerRlt(boolean rlt);
	public int pttEventNetworkChanged(int networkStatus);
	public int onVolumeChanged(int voice);
	public int onUserListUped(ArrayList<ConfUserInfo> list);
	public int onUserStatusChanged(ConfUserInfo user, int status);
}
