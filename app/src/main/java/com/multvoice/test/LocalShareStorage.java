package com.multvoice.test;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.util.Log;

public class LocalShareStorage {

	private static final String TAG = LocalShareStorage.class.getSimpleName();
	
	private static final String SPS_NAME_CFG = TAG;
	
	public static String getMyNumber(Context context)
	{
		String strCode = "";
		try
		{
			SharedPreferences setsp = context.getSharedPreferences(SPS_NAME_CFG,0);
			strCode = setsp.getString("MyNumber", "");
		}
		catch(Exception ex)
		{
			ex.printStackTrace();
		}
		return strCode;
	}
	
	public static void setMyNumber(Context context, String code)
	{
		Log.i(TAG, "save code:"+code);
		SharedPreferences setsp = context.getSharedPreferences(SPS_NAME_CFG, android.content.Context.MODE_PRIVATE);
		Editor spEdit = setsp.edit();
		spEdit.putString("MyNumber", code);
		spEdit.commit();
	}
	
	public static String getPeerNumber(Context context)
	{
		String strCode = "";
		try
		{
			SharedPreferences setsp = context.getSharedPreferences(SPS_NAME_CFG,0);
			strCode = setsp.getString("PeerNumber", "");
		}
		catch(Exception ex)
		{
			ex.printStackTrace();
		}
		return strCode;
	}
	
	public static void setPeerNumber(Context context, String code)
	{
		Log.i(TAG, "save code:"+code);
		SharedPreferences setsp = context.getSharedPreferences(SPS_NAME_CFG, android.content.Context.MODE_PRIVATE);
		Editor spEdit = setsp.edit();
		spEdit.putString("PeerNumber", code);
		spEdit.commit();
	}
}
