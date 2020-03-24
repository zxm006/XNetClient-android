package com.multvoice.test;

import java.util.ArrayList;
import java.util.HashMap;

import com.aiyou.ptt.ConfEventCallback;
import com.aiyou.ptt.ConfManageUtil;
import com.aiyou.ptt.ConfUserInfo;
import com.multvoice.sdk.R;
import com.uutodo.sdk.VideoCapture;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.View.OnClickListener;
import android.view.View.OnLongClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class TestConfActivity extends Activity implements OnClickListener {
	private final String TAG = TestConfActivity.class.getSimpleName();

	private ConfManageUtil mConfMgr;
	private Button mBtnLogin;

	private Button mBtnConfCreate;
	private TextView mTvConfID;
	private String mStrConfId;

	private SurfaceView mSfLocal;

	private Button mBtnInviteMemb; // 邀请好友
	private Button mBtnMianti; // 免提
	private boolean mbIsSpeakOk = false; // 免提开关

	private boolean mbIsLogined = false;
	private boolean mIsMaster = true; // 是否主持人
	private String mMyNumber;

	private HashMap<String, SurfaceView> mMapDisplay = new HashMap<String, SurfaceView>();

	private MyDisplayView[] mArrDisView = new MyDisplayView[4];

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		setContentView(R.layout.activity_conf);

		mBtnLogin = (Button) findViewById(R.id.btn_login);
		mBtnLogin.setOnClickListener(this);

		mBtnConfCreate = (Button) findViewById(R.id.btn_talk_create_conf);
		mBtnConfCreate.setOnClickListener(this);

		mBtnInviteMemb = (Button) findViewById(R.id.btn_invite_member);
		mBtnInviteMemb.setOnClickListener(this);

		mBtnMianti = (Button) findViewById(R.id.btn_mianti);
		mBtnMianti.setOnClickListener(this);

		mTvConfID = (TextView) findViewById(R.id.tv_confid);

		int index = 0;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView) findViewById(R.id.sv_peer1);
		mArrDisView[index].mTextView = (TextView) findViewById(R.id.tv_peer1);

		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView) findViewById(R.id.sv_peer2);
		mArrDisView[index].mTextView = (TextView) findViewById(R.id.tv_peer2);

		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView) findViewById(R.id.sv_peer3);
		mArrDisView[index].mTextView = (TextView) findViewById(R.id.tv_peer3);

		index++;
		mArrDisView[index] = new MyDisplayView();
		mArrDisView[index].mSurface = (SurfaceView) findViewById(R.id.sv_peer4);
		mArrDisView[index].mTextView = (TextView) findViewById(R.id.tv_peer4);
		for (int i = 0; i < mArrDisView.length; i++) {
			mArrDisView[i].mSurface.getHolder().addCallback(mSurfaceCb);
			mArrDisView[i].mSurface.setOnLongClickListener(mLongClidkCb);
		}

		mSfLocal = (SurfaceView) findViewById(R.id.sv_local);
		mSfLocal.getHolder().addCallback(mSurfaceCb);

		mConfMgr = ConfManageUtil.getInstance();
		mConfMgr.setContext(this);
		mConfMgr.setCallback(mCallback);

		// initUser();

		String myNumb = LocalShareStorage.getMyNumber(this);
		((EditText) findViewById(R.id.editText_userName)).setText(myNumb);

		VideoCapture.setContext(getApplicationContext());
	}

	OnLongClickListener mLongClidkCb = new OnLongClickListener() {
		@Override
		public boolean onLongClick(View v) {
			for (int i = 0; i < mArrDisView.length; i++) {
				if (mArrDisView[i].mSurface.getId() == v.getId()) {
					final String peerNumb = mArrDisView[i].mUserName;
					if (TextUtils.isEmpty(peerNumb)) {
						return false;
					}

					if (!mIsMaster) {
						myToast("你不是主持人，不能踢人");
						return false;
					}

					final TextView tv = new TextView(TestConfActivity.this);
					String tmp = "确定要踢出" + peerNumb + "吗?";
					tv.setText(tmp);

					new AlertDialog.Builder(TestConfActivity.this).setTitle("踢出成员")
							.setIcon(android.R.drawable.ic_dialog_info).setView(tv)
							.setPositiveButton("确定", new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int which) {
									mConfMgr.confKickMember(mStrConfId, peerNumb);
									for (int i = 0; i < mArrDisView.length; i++) {
										if (TextUtils.equals(mArrDisView[i].mUserName, peerNumb)) {
											Log.i(TAG, "this will proc peer wnd:" + peerNumb);
											mArrDisView[i].mSurface.invalidate();
											mMapDisplay.remove(peerNumb);
											mArrDisView[i].mTextView.setText("无人");
											mArrDisView[i].mUserName = "";
											break;
										}
									}
								}
							}).setNegativeButton("取消", null).show();
					break;
				}
			}
			return false;
		}
	};

	ConfEventCallback mCallback = new ConfEventCallback() {
		@Override
		public int onConfVideoWndNeed(final String userid) {
			Handler hand = new Handler(getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {
					if (TextUtils.equals(userid, mMyNumber)) {
						myToast("加入会议成功");
						if (mMapDisplay.get(mMyNumber) == null) {
							mConfMgr.confLocalVideoOpen(mSfLocal.getHolder());
							mMapDisplay.put(mMyNumber, mSfLocal);
							mBtnConfCreate.setText("退出会议");
						}
						mIsMaster = false;
					}
				}
			};
			hand.sendEmptyMessage(1);

			return 0;
		}

		@Override
		public int onConfUserMediaStatus(String userid, boolean isvideo, boolean isopen) {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public int onConfNetworkChanged(int networkStatus) {
			// TODO Auto-generated method stub
			Message msg = new Message();
			msg.what = HAND_WHAT_CONF_LOGIN;
			msg.arg1 = networkStatus;
			mHandler.sendMessage(msg);
			return 0;
		}

		@Override
		public int onConfInviteIn(String userid, String confid) {
			final String fUserID = userid;
			final String fConfID = confid;
			if (!TextUtils.isEmpty(mStrConfId)) {
				Log.w(TAG, "已经在会议中，不再处理会议邀请");
				return 0;
			}

			Handler hand = new Handler(getMainLooper()) {
				@Override
				public void handleMessage(Message msg) {
					final TextView tv = new TextView(TestConfActivity.this);
					String tmp = fUserID + "邀请你参加会议" + "\n";
					tmp = tmp + "会议号:" + fConfID;
					tv.setText(tmp);

					new AlertDialog.Builder(TestConfActivity.this).setTitle("会议邀请")
							.setIcon(android.R.drawable.ic_dialog_info).setView(tv)
							.setPositiveButton("确定", new DialogInterface.OnClickListener() {
								public void onClick(DialogInterface dialog, int which) {
									mStrConfId = fConfID;
									mTvConfID.setText(fConfID);
									mConfMgr.confJoin(fConfID, true);
								}
							}).setNegativeButton("取消", null).show();
				}
			};
			hand.sendEmptyMessage(1);

			return 0;
		}

		@Override
		public int onConfStatusChanged(int status, String peerName, String head, String nick, String jingdu, String weidu, String tm) {
			Message msg = new Message();
			Log.i(TAG, "conf statu cb:" + status + "," + peerName);
			msg.what = HAND_WHAT_CONF_STATUS;
			Bundle bld = new Bundle();
			bld.putInt("status", status);
			bld.putString("name", peerName);
			msg.setData(bld);
			mHandler.sendMessage(msg);

			return 0;
		}

		@Override
		public int onConfBeKicked(String userid, String confid) {
			Message msg = new Message();
			Log.i(TAG, "conf statu cb:" + userid + "," + confid);
			msg.what = HAND_WHAT_CONF_BE_KICKED;
			Bundle bld = new Bundle();
			bld.putString("confid", confid);
			bld.putString("name", userid);
			msg.setData(bld);
			mHandler.sendMessage(msg);
			return 0;
		}

		@Override
		public int onConfUserListUped(ArrayList<ConfUserInfo> list) {
			
			for (int i = 0; i < list.size(); i++) {
				ConfUserInfo item = list.get(i);
				Log.i(TAG, String.format("user list:%d,%s,%s,%s", i, item.getmUserName(), 
								item.getmNickName(), item.getmHeadUrl()));
			}
			return 0;
		}
	};

	SurfaceHolder.Callback mSurfaceCb = new SurfaceHolder.Callback() {
		@Override
		public void surfaceDestroyed(SurfaceHolder holder) {
			Log.i(TAG, "local holder destroyed");

			for (int i = 0; i < mArrDisView.length; i++) {
				if (!TextUtils.isEmpty(mArrDisView[i].mUserName) && mArrDisView[i].mSurface.getHolder() == holder) {

					break;
				}
			}
		}

		@Override
		public void surfaceCreated(SurfaceHolder holder) {
			Log.i(TAG, "local holder created");
			if (mSfLocal.getHolder() == holder) {

			}
		}

		@Override
		public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			Log.i(TAG, "local holder changed");
		}
	};

	@Override
	public void onClick(View view) {
		if (view.getId() == mBtnLogin.getId()) {
			if (!mbIsLogined) {
				EditText et = (EditText) findViewById(R.id.editText_ipaddr);
				String ip = et.getText().toString();
				et = (EditText) findViewById(R.id.editText_userName);
				mMyNumber = et.getText().toString();
				if (TextUtils.isEmpty(ip) || TextUtils.isEmpty(mMyNumber)) {
					Toast.makeText(this, "请输入IP地址或用户名", Toast.LENGTH_LONG).show();
					return;
				}
				int iChannels = 1;
				int iSampleRate = 16000; // 采样率
				int iSampleBits = 16; // 采样深度
				int iInterval = 20; // 20ms
				int iBytes = iChannels * iSampleRate * iSampleBits * iInterval / (8 * 1000);
				Log.i(TAG, "buffer is:" + iBytes);

				et = (EditText) findViewById(R.id.editText_gameid);
				String gameID = et.getText().toString();

				et = (EditText) findViewById(R.id.editText_game_server_id);
				String gameServerID = et.getText().toString();

				et = (EditText) findViewById(R.id.editText_room_id);
				String roomID = et.getText().toString();

				et = (EditText) findViewById(R.id.editText_group_id);
				String groupID = et.getText().toString();

				et = (EditText) findViewById(R.id.editText_expend);

				mConfMgr.confLogin(ip, mMyNumber, gameID, gameServerID, roomID, groupID, iChannels, iSampleRate, 16000,
						false, "", "", 0.0f, 0.0f);

				LocalShareStorage.setMyNumber(this, mMyNumber);
			} else {
				for (int i = 0; i < mArrDisView.length; i++) {
					if (!TextUtils.isEmpty(mArrDisView[i].mUserName)) {
						mArrDisView[i].mUserID = -1;
						mArrDisView[i].mUserName = "";
						mArrDisView[i].mTextView.setText("无人");
					}
				}

				mConfMgr.confLogout();
				mBtnLogin.setText("登录");
				mbIsLogined = false;
			}
		} else if (view.getId() == mBtnConfCreate.getId()) {
			if (!mbIsLogined) {
				myToast("请先登录!");
				return;
			}

			if (TextUtils.isEmpty(mStrConfId)) {
				CheckBox cb = (CheckBox) findViewById(R.id.checkBox_opencamera);
				boolean isOpenCam = cb.isChecked();
				mStrConfId = mConfMgr.confCreate(isOpenCam, null, 0);
				mBtnConfCreate.setText("结束会议");
				mTvConfID.setText(mStrConfId);
			} else {
				if (mIsMaster) {
					mConfMgr.confDestroy();
				} else {
					mConfMgr.confExit();
				}
				mStrConfId = "";
				mBtnConfCreate.setText("创建会议");
				mTvConfID.setText("");
				mIsMaster = false;
				mMapDisplay.clear();
				for (int i = 0; i < mArrDisView.length; i++) {
					mArrDisView[i].mTextView.setText("无人");
					mArrDisView[i].mUserName = "";
				}
			}
		} else if (view.getId() == mBtnInviteMemb.getId()) {
			if (TextUtils.isEmpty(mStrConfId)) {
				myToast("请先创建或加入会议!");
				return;
			}

			final EditText et = new EditText(this);
			et.setText("6008");
			new AlertDialog.Builder(this).setTitle("邀请好友").setIcon(android.R.drawable.ic_dialog_info).setView(et)
					.setPositiveButton("确定", new DialogInterface.OnClickListener() {
						public void onClick(DialogInterface dialog, int which) {
							String input = et.getText().toString();
							if (!TextUtils.isEmpty(input)) {
								mConfMgr.confInviteMember(mStrConfId, input);
							}
						}
					}).setNegativeButton("取消", null).show();
		} else if (view.getId() == mBtnMianti.getId()) {
			TestActivity.SetPlayoutSpeaker(this, !mbIsSpeakOk);
			if (mbIsSpeakOk) {
				mBtnMianti.setText("打开免提");
			} else {
				mBtnMianti.setText("关闭免提");
			}
			mbIsSpeakOk = !mbIsSpeakOk;
		}
	}

	private final int HAND_WHAT_CONF_LOGIN = 1;
	private final int HAND_WHAT_CONF_STATUS = 5;
	private final int HAND_WHAT_CONF_BE_KICKED = 6;

	private MyHandl mHandler = new MyHandl(Looper.getMainLooper());

	@SuppressLint("HandlerLeak")
	class MyHandl extends Handler {
		public MyHandl(Looper looper) {
			super(looper);
		}

		@Override
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			int msgWhat = msg.what;
			if (msgWhat == HAND_WHAT_CONF_LOGIN) {
				int argcs = msg.arg1;
				if (argcs == 5 || argcs == 8) {
					mbIsLogined = true;
					mBtnLogin.setText("注销");
					myToast("登录成功," + argcs);
				} else if (argcs == 1 || argcs == 4 || argcs == 5 || argcs == 9) {
					myToast("连接失败," + argcs);
				} else if (argcs == 3) {
					myToast("断开连接," + argcs);
				}else if (argcs == 11) {
					myToast("被踢下线," + argcs);
				}
				
			} else if (msgWhat == HAND_WHAT_CONF_STATUS) {
				Bundle bld = msg.getData();
				int status = bld.getInt("status");
				String peerName = bld.getString("name");
				if (status == 0) {// 创建会议成功
					Log.i(TAG, "conf created ok");
					myToast("创建会议成功");
					mIsMaster = true;
					mConfMgr.confLocalVideoOpen(mSfLocal.getHolder());
					mMapDisplay.put(mMyNumber, mSfLocal);
				} else if (status == 1) {// 创建会议失败
					mTvConfID.setText("");
					mStrConfId = "";
					myToast("创建会议失败");
				} else if (status == 2) {// 加入会议成功
					Log.i(TAG, "conf joined:" + peerName);
					if (TextUtils.equals(peerName, mMyNumber)) {// 自己
						myToast("加入会议成功");
						mConfMgr.confLocalVideoOpen(mSfLocal.getHolder());
						mMapDisplay.put(mMyNumber, mSfLocal);
					} else {// 别人
						if (mMapDisplay.get(peerName) == null) {
							for (int i = 0; i < mArrDisView.length; i++) {
								if (TextUtils.isEmpty(mArrDisView[i].mUserName)) {
									Log.i(TAG, "this will proc peer wnd:" + peerName);
									mConfMgr.confPeerVideoOpen(mArrDisView[i].mSurface.getHolder(), peerName);
									mMapDisplay.put(peerName, mArrDisView[i].mSurface);
									mArrDisView[i].mTextView.setText(peerName);
									mArrDisView[i].mUserName = peerName;
									break;
								}
							}
						}
					}
				} else if (status == 3) {// 加入会议失败
					mTvConfID.setText("");
					mStrConfId = "";
					myToast("加入会议失败");
				} else if (status == 4) {// 退出会议
					if (TextUtils.equals(peerName, mMyNumber)) {// 自己
						myToast("你已经退出会议");
						mMapDisplay.remove(mMyNumber);
					} else {// 别人
						for (int i = 0; i < mArrDisView.length; i++) {
							if (TextUtils.equals(mArrDisView[i].mUserName, peerName)) {
								myToast(peerName + "已退出会议");
								Log.i(TAG, "this will proc peer wnd:" + peerName);
								mArrDisView[i].mSurface.invalidate();
								mMapDisplay.remove(peerName);
								mArrDisView[i].mTextView.setText("无人");
								mArrDisView[i].mUserName = "";
								break;
							}
						}
					}
				} else if (status == 5) {// 会议被解散
					myToast("会议被解散");

					for (int i = 0; i < mArrDisView.length; i++) {
						if (!TextUtils.isEmpty(mArrDisView[i].mUserName)) {
							mArrDisView[i].mSurface.invalidate();
							mArrDisView[i].mTextView.setText("无人");
							mArrDisView[i].mUserName = "";
						}
					}

					mStrConfId = "";
					mBtnConfCreate.setText("创建会议");
					mTvConfID.setText("");
					mIsMaster = false;
					mMapDisplay.clear();
				}
			} else if (msgWhat == HAND_WHAT_CONF_BE_KICKED) {
				Bundle bld = msg.getData();
				String confid = bld.getString("confid");
				String peerName = bld.getString("name");
				myToast(String.format("已经被%s请出会议", peerName));

				mConfMgr.confExit();
				mStrConfId = "";
				mBtnConfCreate.setText("创建会议");
				mTvConfID.setText("");
				mIsMaster = false;
				mMapDisplay.clear();
				for (int i = 0; i < mArrDisView.length; i++) {
					mArrDisView[i].mTextView.setText("无人");
					mArrDisView[i].mUserName = "";
				}
			}
		}
	}

	private Toast mToast;

	private void myToast(String content) {
		if (mToast != null) {
			mToast.cancel();
		}
		mToast = Toast.makeText(this, content, Toast.LENGTH_SHORT);
		mToast.show();
	}

}
