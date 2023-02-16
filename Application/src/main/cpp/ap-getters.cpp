/*
 *
 */
#include <jni.h>
#include <android/log.h>
#include <main.hpp>

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "fcpp", __VA_ARGS__))

/* Definitions below are declared as `native` in AP.java. */

extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_fcpp_1start(JNIEnv *env, jclass thiz, jint uid) {
    // https://stackoverflow.com/a/19657117/60462:
    auto c = static_cast<jclass>( env->NewGlobalRef( thiz) );
    fcpp::start(env, c, uid, "vulnerability_detection");
    return;
}

extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_fcpp_1stop(JNIEnv *env, jclass thiz) {
    fcpp::stop();
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1storage(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF(fcpp::get_storage());
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1round_1count(JNIEnv *env, jclass thiz) {
    uint16_t c = fcpp::get_int("round_count");
    return (jlong)(unsigned long long)c;
}

extern "C" JNIEXPORT jlong JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1max_1msg_1size(JNIEnv *env, jclass thiz) {
    return (jlong)(unsigned long long)fcpp::get_int("max_msg");
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

extern "C"
JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1diameter(JNIEnv *env, jclass clazz) {
    return fcpp::get_diameter();
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1diameter(JNIEnv *env, jclass clazz, jint diam) {
    fcpp::set_diameter(diam);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1retain_1time(JNIEnv *env, jclass clazz) {
    return fcpp::get_retain_time();
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1retain_1time(JNIEnv *env, jclass clazz, jfloat time) {
    fcpp::set_retain_time(time);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1round_1period(JNIEnv *env, jclass clazz) {
    return fcpp::get_round_period();
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1round_1period(JNIEnv *env, jclass clazz, jfloat time) {
    fcpp::set_round_period(time);
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1nbr_1lags(JNIEnv *env, jclass clazz) {
    return env->NewStringUTF(fcpp::get_nbr_lags());
}

extern "C" JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1global_1clock(JNIEnv *env, jclass clazz) {
    return fcpp::get_global_clock();
}

extern "C" JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1hop_1dist(JNIEnv *env, jclass clazz) {
    return fcpp::get_hop_dist();
}

extern "C" JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1min_1uid(JNIEnv *env, jclass clazz) {
    return fcpp::get_min_uid();
}
extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1latlong(JNIEnv *env, jclass clazz, jdouble latitude,
                                                jdouble longitude) {
    fcpp::set_position_latlong(latitude, longitude);
}
extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1accuracy(JNIEnv *env, jclass clazz, jfloat accuracy) {
    fcpp::set_position_accuracy(accuracy);
}