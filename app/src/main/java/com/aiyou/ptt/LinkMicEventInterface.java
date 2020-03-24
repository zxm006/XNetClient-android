package com.aiyou.ptt;

public interface LinkMicEventInterface {
	public int LinkMicNetworkChanged(int networkStatus);
	public int LinkMicUserLogin(int peerUserID, String peerName, int seraudioID);
	public int LinkMicUserLogout(int peerUserID, String peerName);
	/*public int OnInviteIn(long userid, boolean isVideo, String username);
	public int OnConnected();
	public int OnDisConnected(int iReason, String info);*/
}
