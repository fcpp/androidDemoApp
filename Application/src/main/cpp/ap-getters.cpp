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
    fcpp::start(env, c, uid, "traitor_detection");
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
    return fcpp::get_bool("im_weak");
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1some_1weak(JNIEnv *env, jclass thiz) {
    return fcpp::get_bool("some_weak");
}

extern "C"
JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1degree(JNIEnv *env, jclass clazz) {
    return fcpp::get_int("degree");
}

extern "C"
JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1diameter(JNIEnv *env, jclass clazz) {
    return fcpp::get_int("diameter");
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1diameter(JNIEnv *env, jclass clazz, jint diam) {
    fcpp::set_int("diameter", diam);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1retain_1time(JNIEnv *env, jclass clazz) {
    return fcpp::get_double("retain_time");
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1retain_1time(JNIEnv *env, jclass clazz, jfloat time) {
    fcpp::set_double("retain_time", time);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1round_1period(JNIEnv *env, jclass clazz) {
    return fcpp::get_double("round_period");
}

extern "C"
JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1round_1period(JNIEnv *env, jclass clazz, jfloat time) {
    fcpp::set_double("round_period", time);
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1nbr_1lags(JNIEnv *env, jclass clazz) {
    // Danger Will Robinson. TODO: refactor to new FCPP-API.
    return env->NewStringUTF(fcpp::get_string("nbr_lags"));
}

extern "C" JNIEXPORT jfloat JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1global_1clock(JNIEnv *env, jclass clazz) {
    return fcpp::get_double("global_clock");
}

extern "C" JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1hop_1dist(JNIEnv *env, jclass clazz) {
    return fcpp::get_int("hop_dist");
}

extern "C" JNIEXPORT jint JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1min_1uid(JNIEnv *env, jclass clazz) {
    return fcpp::get_int("min_uid");
}
extern "C" JNIEXPORT void JNICALL
Java_org_foldr_fcpp_androidDemo_AP_set_1latlong(JNIEnv *env, jclass clazz, jdouble latitude,
                                                jdouble longitude) {
    fcpp::set_double("position_latitude", latitude);
    fcpp::set_double("position_longitude", longitude);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_org_foldr_fcpp_androidDemo_AP_get_1bool(JNIEnv *env, jclass clazz, jstring attribute_name) {
    const char* str = env->GetStringUTFChars(attribute_name,0);
    bool result = fcpp::get_bool(str);
    env->ReleaseStringUTFChars(attribute_name, str);
    return result;
}
