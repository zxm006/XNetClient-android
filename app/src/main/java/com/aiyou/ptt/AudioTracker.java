package com.aiyou.ptt;

import java.nio.ByteBuffer;
import java.util.concurrent.locks.ReentrantLock;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

class AudioTracker 
{
	private static final String TAG = "AudioTracker";
    private AudioTrack mAudioTrack = null;

    private ByteBuffer mPlayBuffer;
    private byte[] mTempBufPlay;

    //private final ReentrantLock mPlayLock = new ReentrantLock();

    private boolean mIsPlayInit = true;
    private int mBufPlaySamples = 0;
    private int mPlayPos = 0;

    public AudioTracker()
    {
        try {
        	mPlayBuffer = ByteBuffer.allocateDirect(2 * 480); // Max 10 ms @ 48kHz
        } catch (Exception e) {
            DoLog(e.getMessage());
        }

        mTempBufPlay = new byte[2 * 480];
    }

    public int initPlayback(int sampleRate) 
    {
        int minPlayBufSize = AudioTrack.getMinBufferSize(
			            sampleRate,
			            AudioFormat.CHANNEL_OUT_MONO,
			            AudioFormat.ENCODING_PCM_16BIT);

        int playBufSize = minPlayBufSize;
        if (playBufSize < 6000) 
        {
            playBufSize *= 2;
        }
        mBufPlaySamples = 0;

        if (mAudioTrack != null) {
        	mAudioTrack.release();
        	mAudioTrack = null;
        }

        try {
        	mAudioTrack = new AudioTrack(
                            AudioManager.STREAM_VOICE_CALL,
                            sampleRate,
                            AudioFormat.CHANNEL_OUT_MONO,
                            AudioFormat.ENCODING_PCM_16BIT,
                            playBufSize, AudioTrack.MODE_STREAM);
        } catch (Exception e) {
            DoLog(e.getMessage());
            return -1;
        }

        // check that the audioRecord is ready to be used
        if (mAudioTrack.getState() != AudioTrack.STATE_INITIALIZED) {
            return -1;
        }

        Log.i(TAG, "audio track rate:"+sampleRate+", buffer:"+playBufSize);
        
        return 0;
    }

    public int startPlayback() {
        try {
        	mAudioTrack.play();
        } catch (Exception e) {
            e.printStackTrace();
            return -1;
        }

        return 0;
    }

    public int stopPlayback() 
    {
    	//mPlayLock.lock();
        try {
            if (mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING) {
                try {
                	mAudioTrack.stop();
                } catch (Exception e) {
                    e.printStackTrace();
                    return -1;
                }

                mAudioTrack.flush();
            }

            mAudioTrack.release();
            mAudioTrack = null;
        } finally {
        	mIsPlayInit = true;
            //mPlayLock.unlock();
        }

        return 0;
    }

    public int playAudio(int lengthInBytes) 
    {
    	//Log.i(logTag, "will play data len:"+lengthInBytes);
    	//mPlayLock.lock();
        try {
            if (mAudioTrack == null) {
                return -2; // We have probably closed down while waiting for play lock
            }

            // Set priority, only do once
            if (mIsPlayInit == true) {
                try {
                    android.os.Process.setThreadPriority(
                        android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
                } catch (Exception e) {
                    DoLog("Set play thread priority failed: " + e.getMessage());
                }
                mIsPlayInit = false;
            }

            int written = 0;
            mPlayBuffer.get(mTempBufPlay);
            written = mAudioTrack.write(mTempBufPlay, 0, lengthInBytes);
            mPlayBuffer.rewind(); // Reset the position to start of buffer

            // increase by number of written samples
            mBufPlaySamples += (written >> 1);

            // decrease by number of played samples
            int pos = mAudioTrack.getPlaybackHeadPosition();
            if (pos < mPlayPos) { // wrap or reset by driver
            	mPlayPos = 0; // reset
            }
            mBufPlaySamples -= (pos - mPlayPos);
            mPlayPos = pos;

            if (written != lengthInBytes) {
                return -1;
            }
        } finally {
        	//mPlayLock.unlock();
        }

        return mBufPlaySamples;
    }


    final String logTag = "mvAudioTracker";

    private void DoLog(String msg) {
        Log.i(logTag, msg);
    }
}
