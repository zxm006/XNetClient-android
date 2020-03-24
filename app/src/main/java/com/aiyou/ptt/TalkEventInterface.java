package com.aiyou.ptt;

import java.util.ArrayList;

public interface TalkEventInterface {
	public int voiceStatusChanged(int msgcode, boolean isSuccess, String funtype, String status, String username, String isInRoom);
	public int voiceNetworkChanged(int networkStatus);
	public int OnInviteIn(long userid, boolean isVideo, String username);
	public int OnConnected();
	public int OnDisConnected(int iReason, String info);
	public int volumeChanged(int networkStatus);
	public int onUserListUped(ArrayList<ConfUserInfo> list);
	public int onUserStatusChanged(ConfUserInfo user, int status);
}
