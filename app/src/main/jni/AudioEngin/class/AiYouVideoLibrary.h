#ifndef _AIYOU_VIDEO_LIBRAY_H
#define _AIYOU_VIDEO_LIBRAY_H


typedef enum {
	VIDEC_CODEC_H261=0,		//unsupport
	VIDEC_CODEC_H263,
	VIDEC_CODEC_H263P,
	VIDEC_CODEC_H263PP,		//same to H263P
	VIDEC_CODEC_H264,
	VIDEC_CODEC_LSCC,		//unsupport
	VIDEC_CODEC_AH400,
	VIDEC_CODEC_MPEG4,		//unsupport
	VIDEC_CODEC_DH264,
	VIDEC_CODEC_HIKH,
	VIDEC_CODEC_H264_SVC,
	VIDEC_CODEC_HIKC,
	VIDEC_CODEC_COUNT
}VIDEC_CODEC_TYPE;

/*
class VideoCaptureDataCallBack
{
public:
    VideoCaptureDataCallBack(){};
    ~VideoCaptureDataCallBack(){};
public:
    virtual void On_MediaReceiverCallbackVideo(unsigned char*pData,int nLen, bool bKeyFrame, int nWidth, int nHeight) = 0;
};
*/

#endif
