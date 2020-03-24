package com.multvoice.test;

import java.util.ArrayList;

import com.aiyou.ptt.AudioOpusJni;
import com.aiyou.ptt.LinkMicEventInterface;
import com.aiyou.ptt.TalkEventInterface;
import com.multvoice.sdk.R;
import com.uutodo.sdk.VideoCapture;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.TextView;
import android.widget.Toast;

public class TestLinkMicActivity extends Activity implements OnClickListener, LinkMicEventInterface{
	private final String TAG = TestLinkMicActivity.class.getSimpleName();
	
	private Button mBtnLogin;

	private AudioOpusJni mAudioJni;
	
	private boolean mbIsSpeakOk = false; //免提开关
	private Button mBtnSwitchCamera;
	private Button mBtnSpeakOk;
	
	private SurfaceView mSfLocal;
	
	private boolean mbIsLogined = false;
	private boolean mIsAnchor = false;   //是否直播发送端
	
	private MyDisplayView[] mArrDisView = new MyDisplayView[4];
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		
		setContentView(R.layout.activity_linkmic);
		
		mBtnLogin = (Button)findViewById(R.id.btn_login);
		mBtnLogin.setOnClickListener(this);
		
		mBtnSpeakOk = (Button)findViewById(R.id.btn_talk_speakphone_on);
		mBtnSpeakOk.setOnClickListener(this);
		
		mBtnSwitchCamera = (Button)findViewById(R.id.btn_talk_switch_camera);
		mBtnSwitchCamera.setOnClickListener(this);
		
		int index = 0;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView)findViewById(R.id.sv_peer1);
		mArrDisView[index].mTextView = (TextView)findViewById(R.id.tv_peer1);
		
		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView)findViewById(R.id.sv_peer2);
		mArrDisView[index].mTextView = (TextView)findViewById(R.id.tv_peer2);
		
		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView)findViewById(R.id.sv_peer3);
		mArrDisView[index].mTextView = (TextView)findViewById(R.id.tv_peer3);
		
		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView)findViewById(R.id.sv_peer4);
		mArrDisView[index].mTextView = (TextView)findViewById(R.id.tv_peer4);
		for(int i = 0; i < mArrDisView.length; i++)
		{
			mArrDisView[i].mSurface.getHolder().addCallback(mSurfaceCb);
		}
		
		mSfLocal = (SurfaceView)findViewById(R.id.sv_local);
		
		mAudioJni = new AudioOpusJni();
		AudioOpusJni.setLinkMicCb(this);
		
		initUser();
		
        String myNumb = LocalShareStorage.getMyNumber(this);
        ((EditText)findViewById(R.id.editText_userName)).setText(myNumb);
        
        VideoCapture.setContext(getApplicationContext());
        
	}
	
	SurfaceHolder.Callback mSurfaceCb = new SurfaceHolder.Callback() 
	{
		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			Log.i(TAG, "local holder destroyed");
			
			for(int i = 0; i < mArrDisView.length; i++)
   			{
   				if(!TextUtils.isEmpty(mArrDisView[i].mUserName)
   					&& mArrDisView[i].mSurface.getHolder() == holder)
   				{
   					mAudioJni.LinkMicResetRemoteVideo(mArrDisView[i].mUserName, null);
   					break;
   				}
   			}
			
		}
		
		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			Log.i(TAG, "local holder created");
			for(int i = 0; i < mArrDisView.length; i++)
   			{
   				if(!TextUtils.isEmpty(mArrDisView[i].mUserName)
   					&& mArrDisView[i].mSurface.getHolder() == holder)
   				{
   					Message msg = new Message();
   					msg.what = HAND_WHAT_LINK_MIC_PEER_WND_RESET;
   					msg.arg1 = i;
   					mVideoSizeChangedHandler.sendMessage(msg);
   					break;
   				}
   			}
		}
		
		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			Log.i(TAG, "local holder changed");
		}
	};
	
	@Override
	public void onClick(View view) 
	{
		if(view.getId() == mBtnLogin.getId())
		{
			if(!mbIsLogined)
			{
				EditText et = (EditText)findViewById(R.id.editText_ipaddr);
				String ip = et.getText().toString();
				et = (EditText)findViewById(R.id.editText_userName);
				String name = et.getText().toString();
				if(TextUtils.isEmpty(ip) || TextUtils.isEmpty(name))
				{
					Toast.makeText(this, "请输入IP地址或用户名", Toast.LENGTH_LONG).show();
					return;
				}
				int iChannels = 1;
				int iSampleRate = 16000;	//采样率
				int iSampleBits = 16;   //采样深度
				int iInterval = 20;		//20ms
				int iBytes = iChannels * iSampleRate * iSampleBits * iInterval/ (8*1000);
				Log.i(TAG, "buffer is:"+iBytes);
				
				et = (EditText)findViewById(R.id.editText_gameid);
				String gameID = et.getText().toString();
				
				et = (EditText)findViewById(R.id.editText_game_server_id);
				String gameServerID = et.getText().toString();
				
				et = (EditText)findViewById(R.id.editText_room_id);
				String roomID = et.getText().toString();
				
				et = (EditText)findViewById(R.id.editText_group_id);
				String groupID = et.getText().toString();
				
				CheckBox cb = (CheckBox)findViewById(R.id.checkBox_link_mic);
				mIsAnchor = cb.isChecked();
				
				et = (EditText)findViewById(R.id.editText_expend);
				String exPend = et.getText().toString();
				
				mAudioJni.audioModuleInit(this);
				
				mAudioJni.LinkMicLogin(ip, name, name, 
						gameID, gameServerID, roomID, groupID, 
						iChannels, iSampleRate, 16000, false, mIsAnchor, exPend);
				
				LocalShareStorage.setMyNumber(this, name);
			}
			else
			{
				if(!mIsAnchor)
   				{
   					//连麦的人，关闭自己的视频
   					mAudioJni.LinkMicCloseLocalVideo();
   				}
				
				for(int i = 0; i < mArrDisView.length; i++)
	   			{
	   				if(!TextUtils.isEmpty(mArrDisView[i].mUserName))
	   				{
	   					mAudioJni.LinkMicCloseRemoteVideo(mArrDisView[i].mUserName);
	   					mArrDisView[i].mUserID = -1;
	   					mArrDisView[i].mUserName = "";
	   					mArrDisView[i].mTextView.setText("无人");
	   				}
	   			}
				
				mAudioJni.LinkMicLogout();
				mBtnLogin.setText("登录");
				mbIsLogined = false;
			}
		}
		else if(view.getId() == mBtnSpeakOk.getId())
		{
			TestActivity.SetPlayoutSpeaker(this, !mbIsSpeakOk);
			if(mbIsSpeakOk)
			{
				mBtnSpeakOk.setText("打开免提");
			}
			else
			{
				mBtnSpeakOk.setText("关闭免提");
			}
			mbIsSpeakOk = !mbIsSpeakOk;
		}
		else if(view.getId() == mBtnSwitchCamera.getId())
		{
			VideoCapture inst = VideoCapture.getInstance();
			if(inst != null)
			{
				inst.SwitchCamera();
			}
		}
	}

	/*typedef enum {
		CS_CONNECTING=0,		//正在连接
		CS_FAILED = 1,				//无法连接
		CS_CONNECTED = 2, 			//已经连接
		CS_DISCONNECTED = 3,		//断开连接
		CS_BUSY = 4,				//网络忙(已断开正重连)
		CS_RECONNECTED = 5,			//重连成功
		CS_IDLE = 6,				//空闲
		CS_RESTARTED = 7,			//连接重置了【连接断开了，并且又重新连接上了，但是换了一个新连接】
	    CS_LOGINED = 8,             //登陆服务器成功
	    CS_LOGINFAILED = 9,         //登陆服务器失败
	} CONNECT_STATUS;*/
		

	private final int HAND_WHAT_LINK_MIC_LOGIN = 1;
	private final int HAND_WHAT_LINK_MIC_PEER_LOGIN = 2;
	private final int HAND_WHAT_LINK_MIC_PEER_WND_RESET = 3;
	private final int HAND_WHAT_LINK_MIC_PEER_LOGOUT = 4;
	
	private MyHandl mVideoSizeChangedHandler = new MyHandl(Looper.getMainLooper());
	@SuppressLint("HandlerLeak")
	class MyHandl extends Handler 
	{
		public MyHandl(Looper looper)
		{
			super(looper);
		}
		
		@Override
	   	public void handleMessage(Message msg) 
	   	{
	   		super.handleMessage(msg);
	   		int msgWhat = msg.what;
	   		if(msgWhat == HAND_WHAT_LINK_MIC_LOGIN)
	   		{
	   			int argcs = msg.arg1;
	   			if(argcs == 5 || argcs == 8)
	   			{
	   				mbIsLogined = true;
	   				mBtnLogin.setText("注销");
	   				showNotice("登录成功,"+argcs);
	   				
	   				if(!mIsAnchor)
	   				{
	   					//不是直播发送者的话，就启动自己的视频
	   					//mAudioJni.LinkMicOpenLocalVideo(mSfLocal.getHolder());
	   				}
	   			}
	   			else if(argcs == 1||argcs == 4 ||argcs == 5 ||argcs == 9)
	   			{
	   				showNotice("连接失败,"+argcs);
	   			}
	   			else if(argcs == 3)
	   			{
	   				showNotice("断开连接,"+argcs);
	   			}
	   		}
	   		else if(msgWhat == HAND_WHAT_LINK_MIC_PEER_LOGIN)
	   		{
	   			Bundle bundle = msg.getData();
	   			int iUserid = bundle.getInt("userid");
	   			int iAudioID = bundle.getInt("audioid");
	   			String susername = bundle.getString("username");
	   			for(int i = 0; i < mArrDisView.length; i++)
	   			{
	   				if(TextUtils.isEmpty(mArrDisView[i].mUserName))
	   				{
	   					mArrDisView[i].mUserID = iUserid;
	   					mArrDisView[i].mUserName = susername;
	   					mArrDisView[i].mTextView.setText(susername);
	   					mAudioJni.LinkMicOpenRemoteVideo(susername, mArrDisView[i].mSurface.getHolder());
	   					TextureView view;
	   					break;
	   				}
	   			}
	   		}
	   		else if(msgWhat == HAND_WHAT_LINK_MIC_PEER_WND_RESET)
	   		{
	   			int index = msg.arg1;
	   			if(index >= 0 && index < mArrDisView.length)
	   			{
	   				Log.i(TAG, "will reset surface holder,i:"+index);
   					mAudioJni.LinkMicResetRemoteVideo(mArrDisView[index].mUserName, mArrDisView[index].mSurface.getHolder());
	   			}
	   		}
	   		else if(msgWhat == HAND_WHAT_LINK_MIC_PEER_LOGOUT)
	   		{
	   			Bundle bundle = msg.getData();
	   			int iUserid = bundle.getInt("userid");
	   			for(int i = 0; i < mArrDisView.length; i++)
	   			{
	   				if(mArrDisView[i].mUserID == iUserid)
	   				{
	   					Log.i(TAG, "will call close remote vieo");
	   					mAudioJni.LinkMicCloseRemoteVideo(mArrDisView[i].mUserName);
	   					
	   					mArrDisView[i].mTextView.setText("无人");
	   					mArrDisView[i].mUserID = -1;
	   					mArrDisView[i].mUserName = "";
	   					break;
	   				}
	   			}
	   		}
		}
	}
	
	private ArrayList<String> mUserList = new ArrayList<String>();
	
	private void initUser()
	{
		String strTmp;
		for(int i = 1; i < 11; i++)
		{
			strTmp = "t"+i;
			mUserList.add(strTmp);
		}
	}

	private Toast mToast;
	private void showNotice(String content)
	{
		if(mToast != null)
		{
			mToast.cancel();
		}
		mToast = Toast.makeText(this, content, Toast.LENGTH_SHORT);
		mToast.show();
	}
	
	@Override
	public int LinkMicNetworkChanged(int networkStatus) {
		Log.i(TAG, "link mic network status:"+networkStatus);
		Message msg = new Message();
		msg.what = HAND_WHAT_LINK_MIC_LOGIN;
		msg.arg1 = networkStatus;
		mVideoSizeChangedHandler.sendMessage(msg);
		return 0;
	}

	@Override
	public int LinkMicUserLogin(int peerUserID, String peerName, int seraudioID) {
		Log.i(TAG, "link mic user longin:"+peerUserID+","+peerName+","+seraudioID);
		if(!mIsAnchor)
		{
			Log.e(TAG, "you are not Anchor, not proc peer login");
			return 0;
		}
		
		Message msg = new Message();
		msg.what = HAND_WHAT_LINK_MIC_PEER_LOGIN;
		Bundle bundle = new Bundle();
		bundle.putInt("userid", peerUserID);
		bundle.putString("username", peerName);
		bundle.putInt("audioid", seraudioID);
		msg.setData(bundle);
		mVideoSizeChangedHandler.sendMessage(msg);
		return 0;
	}

	@Override
	public int LinkMicUserLogout(int peerUserID, String peerName) {
		Log.i(TAG, "link mic user logout:"+peerUserID);
		if(!mIsAnchor)
		{
			Log.e(TAG, "you are not Anchor, not proc peer logout");
			return 0;
		}
		
		Message msg = new Message();
		msg.what = HAND_WHAT_LINK_MIC_PEER_LOGOUT;
		Bundle bundle = new Bundle();
		bundle.putInt("userid", peerUserID);
		bundle.putString("username", peerName);
		msg.setData(bundle);
		mVideoSizeChangedHandler.sendMessage(msg);
		return 0;
	}
}
