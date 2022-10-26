/*
 *
 */
#include <jni.h>
#include <android/log.h>
#include <main.hpp>
#include <assert.h>

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "fcpp", __VA_ARGS__))

/* Definitions below are declared as `native` in AP.java. */

extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_fcpp_1start(JNIEnv *env, jclass thiz, jint uid, jint diam, jint per, jint end) {
    // https://stackoverflow.com/a/19657117/60462:
    auto c = static_cast<jclass>( env->NewGlobalRef( thiz) );
    fcpp::start(env, c, uid, diam, per, end);
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_fcpp_1stop(JNIEnv *env, jclass thiz) {
    fcpp::stop();
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_foldr_fcpp_androidDemo_AP_getMaxMsgSize(JNIEnv *env, jclass thiz) {
    return (jlong)(unsigned long long)fcpp::get_max_msg_size();
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_foldr_fcpp_androidDemo_AP_getRound(JNIEnv *env, jclass thiz) {
    uint16_t c = fcpp::get_round_count();
    return (jlong)(unsigned long long)c;
}

/* Note the escaped `_` below. */
extern "C" JNIEXPORT jboolean JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1im_1weak(JNIEnv *env, jclass thiz) {
    return fcpp::get_im_weak();
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1some_1weak(JNIEnv *env, jclass thiz) {
    return fcpp::get_some_weak();
}

extern "C"
JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1degree(JNIEnv *env, jclass clazz) {
    return fcpp::get_degree();
}