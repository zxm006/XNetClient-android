#ifndef __UU_VIDEO_EXPORT_H__
#define __UU_VIDEO_EXPORT_H__

#ifdef VIDEO_EXPORT
#define VIDEO_API _declspec(dllexport)
#elif VIDEO_DLL
#define VIDEO_API _declspec(dllimport)
#else
#define VIDEO_API
#endif

#if  (defined LINUX) || (defined ANDROID) 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h> 
#include <netinet/in.h>
#include <net/if.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>

#include <pthread.h>
#include <semaphore.h>

#endif
#endif