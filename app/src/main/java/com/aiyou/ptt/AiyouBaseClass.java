package com.aiyou.ptt;

import java.text.SimpleDateFormat;
import java.util.Date;

import android.annotation.SuppressLint;
import android.content.Context;
import android.util.Log;

public class AiyouBaseClass {

	private final String TAG = AiyouBaseClass.class.getSimpleName();

	protected String CST_STR_NETERR = "network cannot used";

	protected Context mContext;
	protected long mInterval = 0;  //录制的文件的时长
	protected Date mDStart = null; //录制的开始时间

	protected void jishiStart()
	{
		mDStart = new Date();
	}

	protected void jishiEnd()
	{
		Date dTmp = new Date();
		long tmInter = dTmp.getTime() - mDStart.getTime();
		mInterval = (tmInter < 1000) ? 1 : (tmInter/1000 + 1);
	}

	public void setContext(Context context)
	{
		mContext = context;
	}

	public long getInteval(){return mInterval;}

	public String getTag(){return TAG;}


	public interface callbackInterface {
		public int errorCallback(String endToObj, String errCb, String cbContent);
		public int successCallback(String endToObj, String succCb, String cbContent);
	}

	@SuppressLint("SimpleDateFormat")
	public static String buildFileName(EnFileType fType, String custom)
	{
		//创建文件名
		String name = "";
		String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
		switch(fType)
		{
		case EFT_Jpg:
			name = "pic_"+timeStamp+custom+".jpg";
			break;
		case EFT_Amr:
			name = "audio_"+timeStamp+custom+".amr";
			break;
		case EFT_Aac:
			name = "audio_"+timeStamp+custom+".aac";
			break;
		case EFT_Mp4_Camer:
			name = "vid_cam_"+timeStamp+custom+".mp4";
			break;
		case EFT_Mp4_Screen:
			name = "vid_scrn_"+timeStamp+custom+".mp4";
			break;
		default:
			name = "unknown_"+timeStamp+custom;
			break;
		}
		Log.i("basecalss", "build name:"+name);
		return name;
	}

	public enum EnFileType
	{
		EFT_UNKNOWN,
		EFT_Jpg,
		EFT_Aac,
		EFT_Amr,
		EFT_Mp4_Camer,
		EFT_Mp4_Screen
	}
}
