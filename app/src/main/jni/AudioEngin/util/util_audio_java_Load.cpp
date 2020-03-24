#include <jni.h>
#include "util_audio_register_native_api.h"


JavaVM*  g_androidJVM = 0;

#ifdef __cplusplus
extern "C"  {
#endif

/*
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	g_androidJVM = vm;
	JNIEnv *pEnv;
	g_androidJVM->GetEnv((void**)&pEnv, JNI_VERSION_1_6);

	LoadAudioTrackerAndroidVM(pEnv);

	//LoadAndroidVM(pEnv);
	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
	JNIEnv *pEnv;
	vm->GetEnv((void**)&pEnv, JNI_VERSION_1_6);

	UnLoadAudioTrackerAndroidVM(pEnv);
	//UnLoadAndroidVM(pEnv);
}*/


#ifdef __cplusplus
}
#endif


extern "C" int LoadAndroidJavaVM(JavaVM *vm)
{
	g_androidJVM = vm;
	JNIEnv *pEnv;
	g_androidJVM->GetEnv((void**)&pEnv, JNI_VERSION_1_6);

	LoadAudioTrackerAndroidVM(pEnv);
	return 0;
}

extern "C" int UnLoadAndroidVM(JavaVM *vm)
{
	JNIEnv *pEnv;
	vm->GetEnv((void**)&pEnv, JNI_VERSION_1_6);

	UnLoadAudioTrackerAndroidVM(pEnv);
    	return 0;
}


