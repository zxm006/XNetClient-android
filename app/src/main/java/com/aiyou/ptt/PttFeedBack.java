package com.aiyou.ptt;

import java.util.HashMap;
import java.util.Map;
import org.json.JSONObject;

import android.text.TextUtils;
import android.util.Log;

public class PttFeedBack extends AiyouBaseClass{
	private static final String TAG = "rtmpFeedBack";  
    
	private static PttFeedBack mInstance;
	private PttFeedBack(){}
	
	private static final String FB_2U3D_ERRCODE 	= "code";
	private static final String FB_2U3D_ERRINFO 	= "message";
	private static final String FB_2U3D_RETMSG 	= "retMsg";
	private static final String FB_2U3D_CBFUN 	= "callBackFun";
	
	public static PttFeedBack getInstance()
	{
		if(mInstance == null)
		{
			mInstance = new PttFeedBack();
		}
		return mInstance;
	}
	
	/**
	 * 说话状态回调
	 */
	public void statusFeedback(EpttCallBackType callBackType, boolean canSpeak, String speaker, String inroom) 
    {
		JSONObject jsonObj = new JSONObject();
    	try
    	{
    		JSONObject objInner = new JSONObject();
    		objInner.put("status", canSpeak ? "1" : 0);
    		objInner.put("username", speaker);
    		objInner.put("inRoom", inroom);
    		
    		jsonObj.put(FB_2U3D_RETMSG, objInner);
	    	jsonObj.put(FB_2U3D_CBFUN, callBackType.name());
    	}
    	catch(Exception ex)
    	{
    		Log.e(TAG, "build json failed");
    		ex.printStackTrace();
    	}
    	
    	String strConen = jsonObj.toString();
		feedBackSuccess(strConen);
	}
	
	/**
	 * 网络状态回调
	 */
	public void networkFeedback(EpttCallBackType callBackType, int networkStatus) 
    {
		JSONObject jsonObj = new JSONObject();
    	try
    	{
    		JSONObject objInner = new JSONObject();
    		objInner.put("status", networkStatus);
	
    		jsonObj.put(FB_2U3D_RETMSG, objInner);
	    	jsonObj.put(FB_2U3D_CBFUN, callBackType.name());
    	}
    	catch(Exception ex)
    	{
    		Log.e(TAG, "build json failed");
    		ex.printStackTrace();
    	}
    	
    	String strConen = jsonObj.toString();
		feedBackSuccess(strConen);
	}
	
	/**
	 * 失败回调
	 * @param callBackType
	 * @param iErrcode
	 * @param exter
	 */
	public void errorFeedback(EpttCallBackType callBackType, int iErrcode, String exter) 
    {
		Map<String, Object> rtMap = new HashMap<String, Object>();
		rtMap.put(FB_2U3D_ERRCODE, iErrcode);
		rtMap.put(FB_2U3D_ERRINFO, TextUtils.isEmpty(exter) ? "" : exter);
		String strConent = getRtMessageForJson(callBackType, rtMap);
		feedBackError(strConent);
	}
	
	/**
	 * 成功回调
	 * @param callBackType
	 * @param iErrcode
	 * @param exter
	 */
	public void successFeedback(EpttCallBackType callBackType, int iErrcode, String exter) 
    {
		Map<String, Object> rtMap = new HashMap<String, Object>();
		rtMap.put(FB_2U3D_ERRCODE, iErrcode);
		rtMap.put(FB_2U3D_ERRINFO, TextUtils.isEmpty(exter) ? "" : exter);
		String strConent = getRtMessageForJson(callBackType, rtMap);
		feedBackSuccess(strConent);
	}
	
	public void feedBackError(String fbContext)
	{
//		super.feedBackError(fbContext);
	}
	
	public void feedBackSuccess(String fbContext)
	{
//		super.feedBackSuccess(fbContext);
	}
	
	/*
	 * 构造回调json串
	 */
	private static String getRtMessageForJson(EpttCallBackType callBackType, Map<String, Object> map) 
    {
    	JSONObject jsonObj = new JSONObject();
    	try
    	{
    		JSONObject objInner = new JSONObject();
	    	for (String key : map.keySet()) {
				objInner.put(key, map.get(key));
			}
	
	    	if(objInner.length() > 0)
	    	{
	    		jsonObj.put(FB_2U3D_RETMSG, objInner);
	    	}
	    	jsonObj.put(FB_2U3D_CBFUN, callBackType.name());
    	}
    	catch(Exception ex)
    	{
    		Log.e(TAG, "build json failed");
    		ex.printStackTrace();
    	}
    	
    	String strJson = jsonObj.toString();
    	Log.i(TAG, "jsonRlt is:"+strJson);
    	return strJson;
	}
}
