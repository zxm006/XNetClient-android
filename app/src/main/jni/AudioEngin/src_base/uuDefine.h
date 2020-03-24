#ifndef __UU_DEFINE_H_
#define __UU_DEFINE_H_

/*
ï¿½ï¿½ï¿½å±¾ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ãµï¿½ï¿½Ä³ï¿½ï¿½ï¿½
*/

#include <string>
#include <map>

#include <assert.h>

using namespace std;


#define  LOCAL_MCUPort  5566
#define  REMOTE_MCUPort 5566

#ifndef YES
#define YES 1
#define NO  0
#endif

/*
typedef enum {
    CS_CONNECTING=0,		//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
    CS_FAILED,				//ï¿½Þ·ï¿½ï¿½ï¿½ï¿½ï¿½
    CS_CONNECTED,			//ï¿½Ñ¾ï¿½ï¿½ï¿½ï¿½ï¿½
    CS_DISCONNECTED,		//ï¿½Ï¿ï¿½ï¿½ï¿½ï¿½ï¿½
    CS_BUSY,				//ï¿½ï¿½ï¿½ï¿½Ã¦(ï¿½Ñ¶Ï¿ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½)
    CS_RECONNECTED,			//ï¿½ï¿½ï¿½ï¿½ï¿½É¹ï¿½
    CS_IDLE,				//ï¿½ï¿½ï¿½ï¿½
    CS_RESTARTED,			//ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë¡ï¿½ï¿½ï¿½ï¿½Ó¶Ï¿ï¿½ï¿½Ë£ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ë£ï¿½ï¿½ï¿½ï¿½Ç»ï¿½ï¿½ï¿½Ò»ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ó¡ï¿½
    CS_LOGINED,             //ï¿½ï¿½Â½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½É¹ï¿½
    CS_LOGINFAILED,         //ï¿½ï¿½Â½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½Ê§ï¿½ï¿½
    CS_LOGOUT         //ï¿½Ë³ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½
} CONNECT_NET_STATUS;
*/

typedef enum {
    MTG_CREAT=0,		//ÕýÔÚÁ¬½Ó
    MTG_JOIN,
    MTG_EXIT,
    MTG_DESTROY
} MTG_CMD;

typedef enum {
    MTG_Common=0,        //ÕýÔÚÁ¬½Ó
    MTG_MultIntercom,
  
} MTG_TYPE;

typedef enum
{
    AUDIO_SEND_ENABLE=0,    //¿ÉÒÔ·¢ËÍ
    AUDIO_SEND_DISABLE,     //²»ÄÜ·¢ËÍ
    AUDIO_SEND_SENDING         //ÕýÔÚ·¢ËÍ
    
} AUDIO_SEND_STATUS;

typedef struct _CLIENTUSERINFOLIST
{
    _CLIENTUSERINFOLIST():strUserName("")
    ,strNickName("")
    ,strHeadUrl("")
    ,ulLatitude(0)
    ,ulLongitude(0)
    ,ulUserId(0)
    ,ulUserAudioID(0)
    ,uiVideoCanSee(0)
    ,ulMtgTime("")
    {}
    std::string   strUserName;
    std::string strHeadUrl;
    std::string strNickName;
    double ulLatitude;
    double ulLongitude;
    std::string ulMtgTime;
	unsigned long ulUserId;
	unsigned long ulUserAudioID;
	unsigned int  uiVideoCanSee;
	std::string   strLocalIP;
	std::string   strNATIP;
}CLIENTUSERINFOLIST;

typedef std::map<unsigned long, CLIENTUSERINFOLIST> CLIENTUSERINFOLIST_MAP;

//#define LOGI printf

unsigned long XGetTimestamp(void);

#endif
