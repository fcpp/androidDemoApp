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
#include "lib/common/template_remover.hpp"

#include "driver.hpp"

#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, "fcpp", __VA_ARGS__))

/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Tag for the vulnerability detection experiment.
struct vulnerability_detection {};

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
    //! @brief GPS latitude.
    struct position_latitude {};
    //! @brief GPS longitude.
    struct position_longitude {};
    //! @brief GPS accuracy.
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


//! @brief The export type for each experiment.
template <typename> struct experiment_t;
//! @brief The storage tuple type for each experiment.
template <typename> struct storage_t;

//! @brief Computes whether there is a node with only one connected neighbour at a given time.
FUN void experiment(ARGS, vulnerability_detection) { CODE
    using namespace tags;
    int diameter = node.storage(tags::diameter{});
    node.storage(degree{}) = node.size() - 1;
    node.storage(im_weak{}) = node.size() <= 2;
    tie(node.storage(min_uid{}), node.storage(hop_dist{})) = diameter_election_distance(CALL, diameter);
    bool collect_weak = sp_collection(CALL, node.storage(hop_dist{}), node.size() <= 2, false, [](bool x, bool y) {
        return x or y;
    });
    node.storage(some_weak{}) = broadcast(CALL, node.storage(hop_dist{}), collect_weak);
}
//! @brief The export type for the vulnerability detection experiment.
template <>
struct experiment_t<vulnerability_detection> : export_list<
    diameter_election_distance_t<>, sp_collection_t<hops_t, bool>, broadcast_t<hops_t, bool>
> {};
//! @brief The storage tuple type for the vulnerability detection experiment.
template <>
struct storage_t<vulnerability_detection> : common::type_sequence<component::tags::tuple_store<
    tags::degree,         int8_t,
    tags::min_uid,        device_t,
    tags::hop_dist,       hops_t,
    tags::im_weak,        bool,
    tags::some_weak,      bool,
    tags::diameter,       hops_t
>> {};


//! @brief Templated main aggregate function.
template <typename tag>
struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        using namespace tags;
        node.next_time(node.current_time() + node.storage(round_period{}));
        node.storage(nbr_lags{}) = node.nbr_lag();
        node.message_threshold(node.storage(retain_time{}));
        experiment(CALL, tag{});
        resource_tracking(CALL);
    }
};
//! @brief The export type for the main aggregate function.
template <typename tag>
FUN_EXPORT main_t = export_list<experiment_t<tag>, resource_tracking_t>;

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
    uid,                device_t,
    round_period,       times_t,
    nbr_lags,           field<times_t>,
    retain_time,        times_t,
    round_count,        uint16_t,
    global_clock,       times_t,
    max_msg,            uint8_t,
    position_latitude,  double,
    position_longitude, double,
    position_accuracy,  float
>;

//! @brief Main FCPP option setup.
template <typename tag>
DECLARE_OPTIONS(main,
    program<coordination::main<tag>>,
    exports<coordination::main_t<tag>>,
    store_type,
    coordination::storage_t<tag>,
    retain_type,
    message_push<false>
);

} // namespace option

//! @brief The list of supported experiments.
using experiments = common::type_sequence<vulnerability_detection>;

//! @brief Type of the network object for a given experiment.
template <typename tag>
using net_t = typename component::deployment<option::main<tag>>::net;

//! @brief The type of locks for accessing node objects.
using lock_type = typename net_t<experiments::front>::lock_type;

//! @brief Type of the tuple of network pointers for all experiments (general form).
template <typename E>
struct exp_t;

//! @brief Type of the tuple of network pointers for all experiments.
template <typename... Es>
struct exp_t<common::type_sequence<Es...>> : common::tagged_tuple<common::type_sequence<Es...>, common::type_sequence<net_t<Es>*...>> {};

//! @brief Tuple of network pointers for all experiments.
exp_t<experiments> network;

//! @brief Which experiment is currently running.
std::string running_experiment = "vulnerability_detection";

//! @brief The thread running the experiment.
std::thread t;

//! @cond INTERNAL
namespace details {
    //! @brief Converts since the target is a string.
    template <typename T>
    inline std::string maybe_convert(T&& x, common::type_sequence<std::string>) {
        return to_string(std::forward<T>(x));
    }
    //! @brief Does not convert since the target is not a string.
    template <typename T, typename R, typename = std::enable_if_t<not std::is_same<R, std::string>::value>>
    inline T&& maybe_convert(T&& x, common::type_sequence<R>) {
        return std::forward<T>(x);
    }

    //! @brief Type predicates checking whether the type is convertible to R or R is string.
    template <typename R>
    struct convertible_or_string {
        template <typename T>
        struct predicate {
            static constexpr bool value = std::is_convertible<T, R>::value or std::is_same<R, std::string>::value;
        };
    };
}
//! @endcond

//! @brief Accesses a field of the storage by the string name of a tag.
template <typename R>
inline R storage_getter(std::string const& name) {
    return common::applier(running_experiment, network, [&name](auto* n){
        return common::applier<details::convertible_or_string<R>::template predicate>(name, n->node_at(os::uid()).storage_tuple(), [](auto const& x){
            return details::maybe_convert(x, common::type_sequence<R>{});
        });
    });
}

//! @brief Updates a field of the storage by the string name of a tag.
template <typename R>
inline void storage_setter(std::string const& name, R&& val) {
    common::applier(running_experiment, network, [&name,&val](auto* n){
        lock_type l;
        return common::setter(name, n->node_at(os::uid(), l).storage_tuple(), std::forward<R>(val));
    });
}

//! @brief Gets the whole storage as a string.
char* get_storage() {
    std::string s = common::applier(running_experiment, network, [](auto* n){
        return to_string(n->node_at(os::uid()).storage_tuple());
    });
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

//! @brief Accesses a field of the storage as a string by the string name of a tag.
char* get_string(char* name) {
    std::string s = storage_getter<std::string>(name);
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

//! @brief Accesses a floating-point field of the storage by the string name of a tag.
double get_double(char* name) {
    return storage_getter<double>(name);
}

//! @brief Accesses an integer field of the storage by the string name of a tag.
int get_int(char* name) {
    return storage_getter<int>(name);
}

//! @brief Accesses an integer field of the storage by the string name of a tag.
int get_bool(char* name) {
    return storage_getter<bool>(name);
}

//! @brief Updates a string field of the storage by the string name of a tag.
void set_string(char* name, char* val) {
    storage_setter(name, val);
}

//! @brief Updates a floating-point field of the storage by the string name of a tag.
void set_double(char* name, double val) {
    storage_setter(name, val);
}

//! @brief Updates an integer field of the storage by the string name of a tag.
void set_int(char* name, int val) {
    storage_setter(name, val);
}

//! @brief Updates an integer field of the storage by the string name of a tag.
void set_bool(char* name, bool val) {
    storage_setter(name, val);
}

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
    get<vulnerability_detection>(network) = new net_t<vulnerability_detection>(init_v);
    // Run the program until exit.
    t = std::thread([]() {
        get<vulnerability_detection>(network)->run();
    });
}

//! @brief Stops FCPP.
void stop() {
    lock_type l;
    get<vulnerability_detection>(network)->node_at(os::uid(), l).terminate();
    t.join();
    delete get<vulnerability_detection>(network);
}

char* get_nbr_lags() {
    std::string s = storage_getter<std::string>("nbr_lags");
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

uint16_t get_round_count() {
    return storage_getter<uint16_t>("round_count");
}

uint8_t get_max_msg_size() {
    return storage_getter<uint8_t>("max_msg");
}

bool get_im_weak() {
    return storage_getter<bool>("im_weak");
}

bool get_some_weak() {
    return storage_getter<bool>("some_weak");
}

uint8_t get_min_uid() {
    return storage_getter<uint8_t>("min_uid");
}

float get_global_clock() {
    return storage_getter<float>("global_clock");
}

uint8_t get_hop_dist() {
    return storage_getter<uint8_t>("hop_dist");
}

uint8_t get_degree() {
    return storage_getter<uint8_t>("degree");
}

uint8_t get_diameter() {
    return storage_getter<uint8_t>("diameter");
}

void set_diameter(uint8_t v) {
    storage_setter("diameter", v);
}

float get_retain_time() {
    return storage_getter<float>("retain_time");
}

void set_retain_time(float v) {
    storage_setter("retain_time", v);
}

float get_round_period() {
    return storage_getter<float>("round_period");
}

void set_round_period(float v) {
    storage_setter("round_period", v);
}

void set_position_latlong(double latitude, double longitude) {
    storage_setter("position_latitude", latitude);
    storage_setter("position_longitude", longitude);
}
void set_position_accuracy(float accuracy) {
    storage_setter("position_accuracy", accuracy);
}

} // namespace fcpp
