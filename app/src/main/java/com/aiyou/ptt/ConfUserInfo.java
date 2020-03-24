package com.aiyou.ptt;

/**
 * Created by Aiyou on 2017/10/17.
 */

public class ConfUserInfo {
    private String mUserName;
    private String mHeadUrl;
    private String mNickName;
    private String mJingdu;
    private String mWeidu;
    private String mGtTime;

    public ConfUserInfo()
    {

    }
    
    public ConfUserInfo(String mUserName, String mHeadUrl, String mNickName, String mJingdu, String mWeidu, String mGtTime) {
        this.mUserName = mUserName;
        this.mHeadUrl = mHeadUrl;
        this.mNickName = mNickName;
        this.mJingdu = mJingdu;
        this.mWeidu = mWeidu;
        this.mGtTime = mGtTime;
    }
    public void setmUserName(String mUserName) {
        this.mUserName = mUserName;
    }

    public void setmHeadUrl(String mHeadUrl) {
        this.mHeadUrl = mHeadUrl;
    }

    public void setmNickName(String mNickName) {
        this.mNickName = mNickName;
    }

    public void setmJingdu(String mJingdu) {
        this.mJingdu = mJingdu;
    }

    public void setmWeidu(String mWeidu) {
        this.mWeidu = mWeidu;
    }

    public void setmGtTime(String mGtTime) {
        this.mGtTime = mGtTime;
    }

    public String getmUserName() {
        return mUserName;
    }

    public String getmHeadUrl() {
        return mHeadUrl;
    }

    public String getmNickName() {
        return mNickName;
    }

    public String getmJingdu() {
        return mJingdu;
    }

    public String getmWeidu() {
        return mWeidu;
    }

    public String getmGtTime() {
        return mGtTime;
    }
}
