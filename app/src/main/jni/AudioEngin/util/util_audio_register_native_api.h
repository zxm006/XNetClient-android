#ifndef __AUDIO_REGISTER_NATIVE_API_H__
#define __AUDIO_REGISTER_NATIVE_API_H__

#include <jni.h>

extern "C" int LoadAndroidJavaVM(JavaVM *vm);
extern "C" int UnLoadAndroidVM(JavaVM *vm);

//extern "C" JavaVM*  getJavaVM();

extern "C" int LoadAudioTrackerAndroidVM(JNIEnv* env);
extern "C" int UnLoadAudioTrackerAndroidVM(JNIEnv* env);

#endif