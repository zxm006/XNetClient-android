package com.multvoice.test;

public enum CALL_STATUS {
	CS_INITED,		//初始状态
	CS_CALLOUT,		//呼出状态
	CS_PEERRING,	//对方振铃
	CS_RINGING,		//呼入
	CS_CONNECTED,	//接通
	CS_TERMINAL,	//结束
}
