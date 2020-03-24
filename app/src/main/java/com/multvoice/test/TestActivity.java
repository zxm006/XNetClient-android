package com.multvoice.test;

import java.util.ArrayList;

import com.aiyou.ptt.AudioOpusJni;
import com.aiyou.ptt.ConfUserInfo;
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

public class TestActivity extends Activity implements OnClickListener, TalkEventInterface{
	private static final String TAG = TestActivity.class.getSimpleName();
	
	private Button mBtnLogin;
	private Button mBtnSpeak;
	private TextView mTvSpeakerShow;
	private TextView mTvSpeakerInRoom;
	private boolean mbIsRoomSpeak = true;

	private AudioOpusJni mAudioJni;
	
	private Button mBtnTalkCall;
	private CALL_STATUS mCallStatus = CALL_STATUS.CS_INITED;
	private Button mBtnSpeakOk;
	private boolean mbIsSpeakOk = false;
	private Button mBtnSwitchCamera;
	
	private SurfaceView mSfLocal;
	private SurfaceView mSfPeer;
	
	private SurfaceHolder holderLocal;
	private SurfaceHolder holderPeer;
	private String mMyNumb;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		
		setContentView(R.layout.activity_fullscreen);
		
		mBtnLogin = (Button)findViewById(R.id.btn_login);
		mBtnLogin.setOnClickListener(this);
		
		mBtnSpeak = (Button)findViewById(R.id.btn_speak);
		mBtnSpeak.setOnClickListener(this);
		
		mBtnTalkCall = (Button)findViewById(R.id.btn_talk_callout);
		mBtnTalkCall.setOnClickListener(this);
		
		mBtnSpeakOk = (Button)findViewById(R.id.btn_talk_speakphone_on);
		mBtnSpeakOk.setOnClickListener(this);
		
		mBtnSwitchCamera = (Button)findViewById(R.id.btn_talk_switch_camera);
		mBtnSwitchCamera.setOnClickListener(this);
		
		mTvSpeakerShow = (TextView)findViewById(R.id.textView_speaker);
		mTvSpeakerInRoom = (TextView)findViewById(R.id.textView_in_room);
		
		mSfLocal = (SurfaceView)findViewById(R.id.sv_local);
		mSfLocal.getHolder().addCallback(new SurfaceHolder.Callback() 
		{
			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				Log.i(TAG, "local holder destroyed");
				holderLocal = null;
			}
			
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				Log.i(TAG, "local holder created");
				holderLocal = holder;
			}
			
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			}
		});
		mSfPeer  = (SurfaceView)findViewById(R.id.sv_peer);
		mSfPeer.getHolder().addCallback(new SurfaceHolder.Callback() 
		{
			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				Log.i(TAG, "peer holder destroyed");
				holderPeer = null;
			}
			
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				Log.i(TAG, "peer holder created");
				holderPeer = holder;
			}
			
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			}
		});
		
		mAudioJni = new AudioOpusJni();
		AudioOpusJni.setTalkStatusCb(this);
		
		initUser();
		
		RadioGroup group = (RadioGroup)this.findViewById(R.id.radioGroup_ptt);
        //绑定一个匿名监听器
        group.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            
            @Override
            public void onCheckedChanged(RadioGroup arg0, int arg1) {
                //获取变更后的选中项的ID
            	if(arg0.getCheckedRadioButtonId() == findViewById(R.id.radioGroup_room).getId())
            	{
            		mbIsRoomSpeak = true;
            	}
            	else
            	{
            		mbIsRoomSpeak = false;
            	}
            }
        });
        RadioButton rb = (RadioButton)findViewById(R.id.radioGroup_room);
        rb.setChecked(true);
        rb = (RadioButton)findViewById(R.id.radioGroup_group);
        rb.setChecked(false);
        
        String myNumb = LocalShareStorage.getMyNumber(this);
        String peerNumb = LocalShareStorage.getPeerNumber(this);
        ((EditText)findViewById(R.id.editText_userName)).setText(myNumb);
        ((EditText)findViewById(R.id.editText_talk_peer_numb)).setText(peerNumb);
        
        VideoCapture.setContext(getApplicationContext());
        
        //横屏情况下将视频显示控件的宽高对调
		int iRote = getDeviceOrientation();
		if(iRote == 90 || iRote == 270)
		{
			LayoutParams para1;
			para1 = mSfLocal.getLayoutParams();
			int iTmp = para1.width;
			para1.width  = para1.height;
			para1.height = iTmp;
			mSfLocal.setLayoutParams(para1);
			
			para1 = mSfPeer.getLayoutParams();
			iTmp = para1.width;
			para1.width  = para1.height;
			para1.height = iTmp;
			mSfPeer.setLayoutParams(para1);
		}
	}
	
	private int getDeviceOrientation() 
	{
	    int orientation = 0;
	    WindowManager wm = (WindowManager) this.getSystemService(Context.WINDOW_SERVICE);
	      switch(wm.getDefaultDisplay().getRotation()) {
	        case Surface.ROTATION_90:
	          orientation = 90;
	          break;
	        case Surface.ROTATION_180:
	          orientation = 180;
	          break;
	        case Surface.ROTATION_270:
	          orientation = 270;
	          break;
	        case Surface.ROTATION_0:
	        default:
	          orientation = 0;
	          break;
	      }
	    return orientation;
	}
	
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
				String name = "111";//et.getText().toString();
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
				
				CheckBox cb = (CheckBox)findViewById(R.id.checkBox_listen_inroom);
				boolean bIsListenInRoom = cb.isChecked();
				
				et = (EditText)findViewById(R.id.editText_expend);
				String exPend = et.getText().toString();
				
				mAudioJni.audioModuleInit(this);
				
				mMyNumb = name;
				mAudioJni.pttLogin(ip, name, name, gameID, gameServerID, roomID, groupID, 
						iChannels, iSampleRate, 16000, bIsListenInRoom, exPend, false,
						"", "", 0f, 0f);
				
				LocalShareStorage.setMyNumber(this, name);
			}
			else
			{
				mBtnSpeak.setEnabled(true);
				mBtnSpeak.setText("开始发言");
				mTvSpeakerShow.setText("无人发言");
				
				mAudioJni.pttLogout();
				mBtnLogin.setText("登录");
				mbIsLogined = false;
			}
		}
		else if(view.getId() == mBtnSpeak.getId())
		{
			if(!mbIsSending)
			{
				if(!mbIsLogined)
				{
					Toast.makeText(this, "请先登录", Toast.LENGTH_LONG).show();
					return;
				}

				int iRet = mAudioJni.startSendAudio(mbIsRoomSpeak);
				if(iRet != 0)
				{
					if(mToast != null)
					{
						mToast.cancel();
						mToast = null;
					}
					
					mToast = Toast.makeText(this, "发言失败", Toast.LENGTH_LONG);
					mToast.show();
					return;
				}
				mBtnSpeak.setText("停止发言");
				mTvSpeakerShow.setText("发言人:本机");
			}
			else
			{
				mAudioJni.stopSendAudio();
				mBtnSpeak.setText("开始发言");
				mTvSpeakerShow.setText("无人发言");
			}
			mbIsSending = !mbIsSending;
		}
		else if(view.getId() == mBtnTalkCall.getId())
		{
			if(mCallStatus == CALL_STATUS.CS_INITED)
			{//要发起呼叫了
				String strout = ((EditText)findViewById(R.id.editText_talk_peer_numb)).getText().toString();
				if(TextUtils.isEmpty(strout))
				{
					Toast.makeText(this, "请输入被叫号码", Toast.LENGTH_SHORT).show();
					return;
				}
				
				CheckBox ck = (CheckBox)findViewById(R.id.checkBox_talk_isvideo);
				boolean isVideo = ck.isChecked();
				
				mAudioJni.talkCallout(isVideo, strout, holderLocal, holderPeer);
				mCallStatus = CALL_STATUS.CS_CALLOUT;
				mBtnTalkCall.setText("取消呼叫");
				LocalShareStorage.setPeerNumber(this, strout);
			}
			else if(mCallStatus == CALL_STATUS.CS_CALLOUT
					|| mCallStatus == CALL_STATUS.CS_PEERRING)
			{//取消呼叫
				mAudioJni.talkCancel();
				mCallStatus = CALL_STATUS.CS_INITED;
				mBtnTalkCall.setText("发起呼叫");
			}
			else if(mCallStatus == CALL_STATUS.CS_RINGING)
			{//拒绝来电
				mAudioJni.talkRefuse();
				mCallStatus = CALL_STATUS.CS_INITED;
				mBtnTalkCall.setText("发起呼叫");
			}
			else if(mCallStatus == CALL_STATUS.CS_CONNECTED)
			{//挂断通话
				mAudioJni.talkHangup();
				mCallStatus = CALL_STATUS.CS_INITED;
				mBtnTalkCall.setText("发起呼叫");
			}
			else if(mCallStatus == CALL_STATUS.CS_TERMINAL)
			{//拒绝来电
				mCallStatus = CALL_STATUS.CS_INITED;
				mBtnTalkCall.setText("发起呼叫");
			}
		}
		else if(view.getId() == mBtnSpeakOk.getId())
		{
			SetPlayoutSpeaker(this, !mbIsSpeakOk);
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

	private Toast mToast = null;
	
	boolean mbIsLogined = false;
	boolean mbIsSending = false;
	

	private final int HAND_WHAT_MULT_VOICE_LOGIN = 1;
	private final int HAND_WHAT_MULT_VOICE_STATUS = 2;
	
	private String mSpeaker;
	private String mIsInRoom;
	@Override
	public int voiceStatusChanged(int msgcode, boolean isSuccess, String funtype, String status, String username, String isInRoom) {
		Log.e(TAG, "voiceStatusChanged in:"+msgcode+","+isSuccess+","+funtype+","+status+","+username+","+isInRoom);
		if(funtype.equals("MultiVoiceLogin"))
		{
			Message msg = new Message();
			msg.what = HAND_WHAT_MULT_VOICE_LOGIN;
			if(status.equals("SUCCESS") || status.equals("OK"))
			{
				msg.arg1 = 1;
				mAudioJni.startRecvAudio();
			}
			else
			{
				msg.arg1 = 0;
			}
			mVideoSizeChangedHandler.sendMessage(msg);
		}
		else if(funtype.equals("MultiVoiceStatus"))
		{
			Message msg = new Message();
			msg.what = HAND_WHAT_MULT_VOICE_STATUS;
			if(status.equals("1"))
			{
				Log.i(TAG, "there has no speaker");
				msg.arg1 = 1;
			}
			else
			{
				msg.arg1 = 0;
				Log.i(TAG, "there has one speaker:"+username);
			}
			mSpeaker = username;
			mIsInRoom = isInRoom;
			mVideoSizeChangedHandler.sendMessage(msg);
		}
		return 0;
	}
	
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
	   		if(msgWhat == HAND_WHAT_MULT_VOICE_LOGIN)
	   		{
	   			int loginOk = msg.arg1;
	   			if(loginOk == 1)
	   			{
	   				mbIsLogined = true;
	   				mBtnLogin.setText("注销");
	   				Toast.makeText(getApplicationContext(), "登录成功", Toast.LENGTH_SHORT).show();
	   				
	   				mAudioJni.setVolumeEnable(true);
	   			}
	   			else 
	   			{
	   				Toast.makeText(getApplicationContext(), "登录失败", Toast.LENGTH_SHORT).show();
				}
	   		}
	   		else if(msgWhat == HAND_WHAT_MULT_VOICE_STATUS)
	   		{
	   			if(msg.arg1 == 1)
	   			{
	   				//Toast.makeText(getApplicationContext(), "无人发言", Toast.LENGTH_SHORT).show();
	   				mTvSpeakerShow.setText("无人发言");
	   				mTvSpeakerInRoom.setText("");
	   				mBtnSpeak.setEnabled(true);
	   			}
	   			else 
	   			{
	   				
	   				Toast.makeText(getApplicationContext(), "发言人:"+mSpeaker, Toast.LENGTH_SHORT).show();
	   				if(!TextUtils.equals(mMyNumb, mSpeaker))
	   				{
	   					mBtnSpeak.setEnabled(false);
	   				}
	   				
	   				mTvSpeakerShow.setText("发言人:"+mSpeaker);
	   				
	   				if(!TextUtils.isEmpty(mIsInRoom))
	   				{
	   					if("true".equalsIgnoreCase(mIsInRoom))
	   					{
	   						mTvSpeakerInRoom.setText("房间发言");
	   					}
	   					else
	   					{
	   						mTvSpeakerInRoom.setText("组发言");
	   					}
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

	@Override
	public int voiceNetworkChanged(int networkStatus) {
		Log.i(TAG, "voice Network Changed in, status:"+networkStatus);
		return 0;
	}
	
	private AlertDialog mDlgCallin;
	
	@SuppressLint("DefaultLocale")
	private void showTalkCallinDialog(long numb, boolean isVideo, String name)
	{
		String title = "来电提醒";
		String dlgContent = String.format("%s(%d)来电，邀请你进行%s通话", name, numb, isVideo ? "视频" : "音频");
		
		AlertDialog.Builder builder = new AlertDialog.Builder(TestActivity.this);
		builder.setTitle(title);
		builder.setMessage(dlgContent);
		builder.setPositiveButton("接听", new DialogInterface.OnClickListener()
		{
             @Override
             public void onClick(DialogInterface dialog, int which)
             {     
            	 mAudioJni.talkAccept(holderLocal, holderPeer);
 				 mCallStatus = CALL_STATUS.CS_CONNECTED;
 				 mBtnTalkCall.setText("挂断");
                 dialog.dismiss();
             }
         });
		 builder.setNegativeButton("拒绝", new DialogInterface.OnClickListener()
         {
        	 @Override
        	 public void onClick(DialogInterface dialog, int which)
        	 {
        		 mAudioJni.talkRefuse();
 				 mCallStatus = CALL_STATUS.CS_INITED;
 				 mBtnTalkCall.setText("发起呼叫");
	        	 dialog.cancel();
        	 }
         });
		 mDlgCallin = builder.show();
	}

	@Override
	public int OnInviteIn(final long userid, final boolean isVideo, final String username) {
		 mCallStatus = CALL_STATUS.CS_RINGING;
		 runOnUiThread(new Runnable() 
		 {  
             @Override  
             public void run() 
             {
            	 showTalkCallinDialog(userid, isVideo, username);
             }  
         });  
		return 0;
	}

	@Override
	public int OnConnected() {
		mCallStatus = CALL_STATUS.CS_CONNECTED;
		runOnUiThread(new Runnable() 
		{  
            @Override  
            public void run() 
            {
            	mBtnTalkCall.setText("挂断");
            }  
        });  
		return 0;
	}

	@Override
	public int OnDisConnected(final int iReason, String info) {
		mCallStatus = CALL_STATUS.CS_INITED;
		
		runOnUiThread(new Runnable() 
		{
           @Override  
           public void run() 
           {
        	   if(iReason == 0)
        	   {//对方取消
	       			if(mDlgCallin != null && mDlgCallin.isShowing())
	       			{
	       				mDlgCallin.dismiss();
	       				mDlgCallin = null;
	       			}
	       			Toast.makeText(TestActivity.this, "对方取消呼叫", Toast.LENGTH_SHORT).show();
        	   }
        	   else if(iReason == 1)
        	   {//对方拒绝
	       			Toast.makeText(TestActivity.this, "对方拒绝接听", Toast.LENGTH_SHORT).show();
        	   }
        	   else if(iReason == 2)
        	   {//对方拒绝
	       			Toast.makeText(TestActivity.this, "对方已挂断", Toast.LENGTH_SHORT).show();
        	   }
        	   
        	   mBtnTalkCall.setText("发起呼叫");
           }  
        });
		return 0;
	}
	
	public static int SetPlayoutSpeaker(Context context, boolean loudspeakerOn) 
	{
		Log.i(TAG, "set speakphone:"+loudspeakerOn);
		AudioManager audioManager = null;
        if (context != null) {
        	audioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        }

        if (audioManager == null) {
            Log.e(TAG, "Could not change audio routing - no audio manager");
            return -1;
        }

        int apiLevel = android.os.Build.VERSION.SDK_INT;

        if ((3 == apiLevel) || (4 == apiLevel)) {
            // 1.5 and 1.6 devices
            if (loudspeakerOn) {
                // route audio to back speaker
            	audioManager.setMode(AudioManager.MODE_NORMAL);
            } else {
                // route audio to earpiece
            	audioManager.setMode(AudioManager.MODE_IN_CALL);
            }
        } else {
            // 2.x devices
            if ((android.os.Build.BRAND.equals("Samsung") ||
                            android.os.Build.BRAND.equals("samsung")) &&
                            ((5 == apiLevel) || (6 == apiLevel) ||
                            (7 == apiLevel))) {
                // Samsung 2.0, 2.0.1 and 2.1 devices
                if (loudspeakerOn) {
                    // route audio to back speaker
                	audioManager.setMode(AudioManager.MODE_IN_CALL);
                	audioManager.setSpeakerphoneOn(loudspeakerOn);
                } else {
                    // route audio to earpiece
                	audioManager.setSpeakerphoneOn(loudspeakerOn);
                	audioManager.setMode(AudioManager.MODE_NORMAL);
                }
            } else {
                // Non-Samsung and Samsung 2.2 and up devices
            	audioManager.setSpeakerphoneOn(loudspeakerOn);
            }
        }

        return 0;
    }

	@Override
	public int volumeChanged(int networkStatus) {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public int onUserListUped(ArrayList<ConfUserInfo> list) {
		try
		{
			for(int i = 0; list != null && i < list.size(); i++)
			{
				ConfUserInfo item = list.get(i);
				Log.i(TAG, String.format("user info:%s,%s,%s,%s,%s,%d", item.getmUserName(), item.getmNickName(), item.getmHeadUrl(),
						item.getmJingdu(), item.getmWeidu(), item.getmGtTime()));
			}
		}
		catch(Exception ex)
		{
			ex.printStackTrace();
		}
		return 0;
	}

	@Override
	public int onUserStatusChanged(ConfUserInfo user, int status) {
		ConfUserInfo item = user;
		try
		{
			Log.i(TAG, String.format("user info:%s,%s,%s,%s,%s,%d, status:%d", item.getmUserName(), item.getmNickName(), item.getmHeadUrl(),
					item.getmJingdu(), item.getmWeidu(), item.getmGtTime(), status));
		}
		catch(Exception ex)
		{
			ex.printStackTrace();
		}
		return 0;
	}
}
