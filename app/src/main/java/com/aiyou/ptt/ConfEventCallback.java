package com.aiyou.ptt;

import java.util.ArrayList;

public interface ConfEventCallback {
	public int onConfNetworkChanged(int networkStatus);
	public int onConfStatusChanged(int status, String peerName, String head, String nick, String jingdu, String weidu, String tm);
	public int onConfInviteIn(String userid, String confid);
	public int onConfBeKicked(String userid, String confid);
	public int onConfVideoWndNeed(String userid);
	public int onConfUserMediaStatus(String userid, boolean isvideo, boolean isopen);
	public int onConfUserListUped(ArrayList<ConfUserInfo> list);
}
