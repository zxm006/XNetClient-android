package com.aiyou.ptt;

public interface PttAudioInterface {
	public int voiceStatusChanged(int msgcode, boolean isSuccess, String funtype, String status, String username, String isInRoom);
	public int voiceNetworkChanged(int networkStatus);
}
