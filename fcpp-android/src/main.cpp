// Copyright © 2022 Giorgio Audrito and Volker Stolz. All Rights Reserved.

#define FCPP_SYSTEM FCPP_SYSTEM_EMBEDDED
#define FCPP_ENVIRONMENT FCPP_ENVIRONMENT_PHYSICAL
#define FCPP_EXPORT_NUM 2

#define DIAMETER     10
#define RETAIN_TIME  10
#define ROUND_PERIOD 1

#include <android/log.h>
#include <cstring>

#include "main.hpp"
#include "lib/fcpp.hpp"

#include "driver.hpp"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "fcpp", __VA_ARGS__))

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace containing OS-dependent functionalities.
namespace os {
    device_t m_uid;

    //! @brief Access the local unique identifier.
    inline device_t uid() {
        return m_uid;
    }
} // namespace os

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief The degree of the node.
    struct degree {};
    //! @brief The list of lags from all neighbours available in the current round.
    struct nbr_lags {};
    //! @brief Total round count since start.
    struct round_count {};
    //! @brief A shared global clock.
    struct global_clock {};
    //! @brief Minimum UID in the network.
    struct min_uid {};
    //! @brief Distance in hops to the device with minimum UID.
    struct hop_dist {};
    //! @brief Whether the current device has only one neighbour.
    struct im_weak {};
    //! @brief Whether some device in the network has only one neighbour.
    struct some_weak {};
    //! @brief Maximum message size ever experienced.
    struct max_msg {};
    //! @brief Maximum diameter in hops for a deployment.
    struct diameter {};
    //! @brief The retain time for messages.
    struct retain_time {};
    //! @brief Time in seconds between transmission rounds.
    struct round_period {};
    // TODO: Should be a pair?
    struct position_latitude {};
    struct position_longitude {};
    struct position_accuracy {};
}


//! @brief Tracks the passage of time and maximum message size.
FUN void resource_tracking(ARGS) { CODE
    using namespace tags;
    node.storage(round_count{}) = counter(CALL, uint16_t{1});
    node.storage(global_clock{}) = shared_clock(CALL);
    node.storage(max_msg{}) = gossip_max(CALL, (uint16_t)min(node.msg_size(), size_t{255}));
}
FUN_EXPORT resource_tracking_t = export_list<counter_t<uint16_t>, shared_clock_t, gossip_max_t<uint16_t>>;

//! @brief Computes whether there is a node with only one connected neighbour at a given time.
FUN void vulnerability_detection(ARGS, int diameter) { CODE
    using namespace tags;
    node.storage(degree{}) = node.size() - 1;
    node.storage(im_weak{}) = node.size() <= 2;
    tie(node.storage(min_uid{}), node.storage(hop_dist{})) = diameter_election_distance(CALL, diameter);
    bool collect_weak = sp_collection(CALL, node.storage(hop_dist{}), node.size() <= 2, false, [](bool x, bool y) {
        return x or y;
    });
    node.storage(some_weak{}) = broadcast(CALL, node.storage(hop_dist{}), collect_weak);
}
FUN_EXPORT vulnerability_detection_t = export_list<diameter_election_distance_t<>, sp_collection_t<hops_t, bool>, broadcast_t<hops_t, bool>>;


//! @brief Main aggregate function.
MAIN() {
    using namespace tags;
    node.next_time(node.current_time() + node.storage(round_period{}));
    node.storage(nbr_lags{}) = node.nbr_lag();
    node.storage(retain_time{}) = node.message_threshold();
    vulnerability_detection(CALL, node.storage(diameter{}));
    resource_tracking(CALL);
}
FUN_EXPORT main_t = export_list<vulnerability_detection_t, resource_tracking_t>;

} // namespace coordination


//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Dictates that messages are thrown away after RETAIN_TIME seconds.
using retain_type = retain<metric::retain<RETAIN_TIME>>;

//! @brief Tag-type pairs that can appear in node.storage(tag{}) = type{} expressions (are all printed in output).
using store_type = tuple_store<
    uid,            device_t,
    degree,         int8_t,
    nbr_lags,       field<times_t>,
    round_count,    uint16_t,
    global_clock,   times_t,
    min_uid,        device_t,
    hop_dist,       hops_t,
    im_weak,        bool,
    some_weak,      bool,
    max_msg,        uint8_t,
    diameter,       hops_t,
    retain_time,    times_t,
    round_period,   times_t
    , position_latitude,    double
    , position_longitude,    double
    , position_accuracy,    float
>;

//! @brief Main FCPP option setup.
DECLARE_OPTIONS(main,
    program<coordination::main>,
    exports<coordination::main_t>,
    retain_type,
    store_type,
    message_push<false>
);

} // namespace option

//! @brief Type for the network object.
using net_t = component::deployment<option::main>::net;

//! @brief The network object.
net_t* network;

std::thread t;

//! @brief Starts FCPP, returning a pointer to the storage.
void start(JNIEnv *env, jclass apclazz, int uid) {
    env->GetJavaVM(&jvm);
    /* Cache some frequently used, expensive values: */
    clazz = apclazz;
    messageMe = env->GetStaticMethodID(clazz, "getNextMsg", "()[B");
    assert (messageMe != NULL);
    postMsg = env->GetStaticMethodID(clazz, "postMsg", "([BI)V");
    assert (postMsg!=NULL);

    // Sets the id.
    os::m_uid = uid;
    // The initialisation values.
    auto init_v = common::make_tagged_tuple<option::nbr_lags, option::diameter, option::threshold, option::round_period>(0, DIAMETER, (times_t)RETAIN_TIME, ROUND_PERIOD);
    // Construct the network object.
    network = new net_t(init_v);
    // Run the program until exit.
    t = std::thread([]() {
        network->run();
    });
}

//! @brief Stops FCPP.
void stop() {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).terminate();
    t.join();
    delete network;
}

char* get_storage() {
    std::string s = to_string(network->node_at(os::uid()).storage_tuple());
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

char* get_nbr_lags() {
    std::string s = to_string(network->node_at(os::uid()).storage(option::nbr_lags{}));
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

uint16_t get_round_count() {
    return network->node_at(os::uid()).storage(option::round_count{});
}

uint8_t get_max_msg_size() {
    return network->node_at(os::uid()).storage(option::max_msg{});
}

bool get_im_weak() {
    return network->node_at(os::uid()).storage(option::im_weak{});
}

bool get_some_weak() {
    return network->node_at(os::uid()).storage(option::some_weak{});
}

uint8_t get_min_uid() {
    return network->node_at(os::uid()).storage(option::min_uid{});
}

float get_global_clock() {
    return network->node_at(os::uid()).storage(option::global_clock{});
}

uint8_t get_hop_dist() {
    return network->node_at(os::uid()).storage(option::hop_dist{});
}

uint8_t get_degree() {
    return network->node_at(os::uid()).storage(option::degree{});
}

uint8_t get_diameter() {
    return network->node_at(os::uid()).storage(option::diameter{});
}

void set_diameter(uint8_t v) {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).storage(option::diameter{}) = v;
}

float get_retain_time() {
    return network->node_at(os::uid()).message_threshold();
}

void set_retain_time(float v) {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).message_threshold(v);
}

float get_round_period() {
    return network->node_at(os::uid()).storage(option::round_period{});
}

void set_round_period(float v) {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).storage(option::round_period{}) = v;
}

void set_position_latlong(double latitude, double longitude) {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).storage(option::position_latitude{}) = latitude;
    network->node_at(os::uid(), l).storage(option::position_longitude{}) = longitude;
}
void set_position_accuracy(float accuracy) {
    typename net_t::lock_type l;
    network->node_at(os::uid(), l).storage(option::position_accuracy{}) = accuracy;
}

} // namespace fcpp
