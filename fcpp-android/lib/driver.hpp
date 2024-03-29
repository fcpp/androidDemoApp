// Copyright © 2022 Giorgio Audrito and Volker Stolz. All Rights Reserved.

/**
 * @file driver.hpp
 * @brief Implementation of the OS interface for Android.
 */

#ifndef FCPP_ANDROID_DRIVER_H_
#define FCPP_ANDROID_DRIVER_H_

#include <cassert>
#include <cstdio>
#include <exception>
#include <random>
#include <vector>
#include <jni.h>
#include <android/log.h>

#include "lib/settings.hpp"
#include "lib/component/base.hpp"
#include "lib/deployment/os.hpp"

JavaVM* jvm;
jclass clazz;
jmethodID messageMe;
jmethodID postMsg;

constexpr bool enable_debugging = true;

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "fcpp", __VA_ARGS__))
# define LOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_DEBUG, "fcpp", __VA_ARGS__))
#define LOGD(...)


std::vector<char> pollData() {
    LOGD("Polling...(C)");
    std::vector<char> empty;
    JNIEnv *env;
    int attachResult = jvm->AttachCurrentThread(&env, NULL);
    assert (attachResult == JNI_OK);
    /* https://stackoverflow.com/a/8439341/60462: */
    jobject ptr = env->CallStaticObjectMethod(clazz, messageMe);
    jbyteArray *data = reinterpret_cast<jbyteArray *>(&ptr);
    if (*data == NULL) {
        LOGD("data==NULL");
        return empty; // XXX?
    }
    jsize length = env->GetArrayLength(*data);
    LOGD("%s", ("data len="+ std::to_string(length)).c_str());
    if (length == 0) {
        return empty;
    }
    std::vector<char> buff(length, 0);
    env->GetByteArrayRegion(*data,0,length, reinterpret_cast<jbyte*>(buff.data()));
    /* We don't need to release anything. */
    return buff;
}

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing OS-dependent functionalities.
namespace os {

/**
 * @brief Low-level interface for hardware network capabilities.
 *
 * It should have the following minimal public interface:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
 * struct data_type;                            // default-constructible type for settings
 * data_type data;                              // network settings
 * transceiver(data_type);                      // constructor with settings
 * bool send(device_t, std::vector<char>, int); // broadcasts a message after given attemps
 * message_type receive(int);                   // listens for messages after given failed sends
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 */
struct transceiver {
    //! @brief Default-constructible type for settings.
    struct data_type { // TODO: REPLACE WITH ACTUAL SETTINGS AVAILABLE IN ANDROID
        //! @brief Transmission frequency in MHz.
        int frequency;
        //! @brief Transmission power in in dBm.
        int power;
        //! @brief Base time in nanoseconds for each receive call.
        long long receive_time;
        //! @brief Number of attempts after which a send is aborted.
        uint8_t send_attempts;

        //! @brief Member constructor with defaults.
        data_type(int freq = 2450, int pow = 5, long long recv = 50000000LL, uint8_t sndatt = 5) : frequency(freq), power(pow), receive_time(recv), send_attempts(sndatt) {}
    };

    //! @brief Network settings.
    data_type data;

    //! @brief Constructor with settings.
    transceiver(data_type d) : data(d), m_fcpp_timer(common::make_tagged_tuple<>()), m_rng(std::chrono::system_clock::now().time_since_epoch().count()) {
        // TODO: do configuration here of settings
    }

    //! @brief Broadcasts a given message.
    bool send(device_t id, const std::vector<char>& m, int attempt) const {
        // combine data into a packet
        unsigned int size = panHeaderSize + m.size() + sizeof(device_t);
        // try to send it
        try {
            LOGD("Sending...");
            JNIEnv *env;
            int attachResult = jvm->AttachCurrentThread(&env, NULL);
            assert (attachResult == JNI_OK);
            jbyteArray retArray = env->NewByteArray(size);
            void *temp = env->GetPrimitiveArrayCritical((jarray) retArray, 0);
            char *ptr = (char *) temp;
            memcpy(ptr, panHeader, panHeaderSize);
            ptr += panHeaderSize;
            memcpy(ptr, m.data(), m.size());
            ptr += m.size();
            memcpy(ptr, &id, sizeof(device_t));
            env->ReleasePrimitiveArrayCritical(retArray, temp, 0);
            env->CallStaticVoidMethod(clazz, postMsg, retArray, size);
            if (enable_debugging) {
                char strbuf[2 * size + 1];
                btox(strbuf, (char *) temp, 2 * size);
                LOGD("Sent %d byte packet: %s\n", (int) size, strbuf);
            } else {
                LOGD("Sent %d byte packet\n", (int) size);
            }
            // Let's hope the Java side deallocates the string eventually.
            return true;
        } catch (std::exception& e) {
            LOGI("Send failed: %s\n", e.what());
            return attempt == data.send_attempts;
        }
    }

    void btox(char *xp, const char *bb, int n) const
    { // https://stackoverflow.com/a/53966346/60462
        const char xx[]= "0123456789ABCDEF";
        while (--n >= 0) xp[n] = xx[(bb[n>>1] >> ((1 - (n&1)) << 2)) & 0xF];
    }

    //! @brief Receives the next incoming message (empty if no incoming message).
    message_type receive(int attempt) const {
        long long interval = (data.receive_time << attempt);
        if (attempt > 0) {
            std::uniform_int_distribution<long long> d(data.receive_time, interval);
            interval = d(m_rng); // the listening time
        }
        message_type m;
        try {

            std::vector<char> vs_packet = pollData();
            if (vs_packet.size() == 0) {
                return m;
            }
            size_t size = vs_packet.size();
            char packet[size];
            memcpy(packet, vs_packet.data(),size);
            if (size >= static_cast<int>(panHeaderSize + sizeof(device_t)) and memcmp(packet, panHeader, panHeaderSize) == 0) {
                m.time = m_fcpp_timer.real_time();
                m.device = *reinterpret_cast<device_t*>(packet + size - sizeof(device_t));
                m.content.resize(size - panHeaderSize - sizeof(device_t));
                // Skips the panHeader of course:
                memcpy(m.content.data(), packet + panHeaderSize, size - panHeaderSize - sizeof(device_t));
                if (enable_debugging) {
                    char strbuf[2*size+1];
                    btox(strbuf, vs_packet.data(), 2*size);
                    LOGD("Raw packet recvd: %s\n", strbuf);
                    btox(strbuf, m.content.data(), 2*(size - panHeaderSize - sizeof(device_t)));
                    LOGD("Received %d byte packet from device %d at time %f: %s\n", (int)size, m.device, m.time, strbuf);
                } else {
                    LOGD("Received %d byte packet from device %d at time %f\n", (int)size, m.device, m.time);
                }
            } else {
                LOGI("Receive error, size: %ld\n", size);
            }
        } catch(std::exception& e) {
            LOGI("Receive exception: %s\n", e.what());
        }
        return m;
    }

  private:
    //! @brief An empty net object for accessing real time.
    component::combine<>::component<>::net m_fcpp_timer;
    //! @brief A random engine.
    mutable std::default_random_engine m_rng;
    //! @brief The size of the message header.
    static constexpr unsigned int panHeaderSize = 0; // TODO: Remove completely
    //! @brief The message header.
    static const char panHeader[panHeaderSize];
};

//! @brief The message header content.
const char transceiver::panHeader[panHeaderSize] = {};


}


}

#endif // FCPP_ANDROID_DRIVER_H_
