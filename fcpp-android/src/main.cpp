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
#include "lib/coordination/general.hpp"
#include "lib/coordination/vulnerability_detection.hpp"

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
    //! @brief GPS latitude.
    struct position_latitude {};
    //! @brief GPS longitude.
    struct position_longitude {};
    //! @brief GPS accuracy.
    struct position_accuracy {};
}

//! @brief The storage tuple type for the vulnerability detection experiment.
template <>
struct experiment_s<vulnerability_detection> : storage_list<
    tags::degree,         int8_t,
    tags::min_uid,        device_t,
    tags::hop_dist,       hops_t,
    tags::im_weak,        bool,
    tags::some_weak,      bool,
    tags::diameter,       hops_t
> {};

//! @brief Templated main aggregate function.
template <typename tag>
struct main {
    template <typename node_t>
    void operator()(node_t& node, times_t) {
        using namespace tags;
        experiment(CALL, tag{});
        tracker(CALL);
    }
};
//! @brief The export type for the main aggregate function.
template <typename tag>
FUN_EXPORT main_t = export_list<experiment_t<tag>, tracker_t>;
template <typename tag>
FUN_EXPORT main_s = storage_list<experiment_s<tag>, tracker_s,
    component::tags::uid,     device_t,
    tags::position_latitude,  double,
    tags::position_longitude, double,
    tags::position_accuracy,  float
>;

} // namespace coordination


//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Main FCPP option setup.
template <typename tag>
DECLARE_OPTIONS(main,
    program<coordination::main<tag>>,
    exports<coordination::main_t<tag>>,
    tuple_store<coordination::main_s<tag>>,
    retain<metric::retain<RETAIN_TIME>>,
    message_push<false>
);

} // namespace option

//! @brief The list of supported experiments.
using experiments = common::type_sequence<
    vulnerability_detection
>;

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
std::string running_experiment = "";

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
char* get_string(char const* name) {
    std::string s = storage_getter<std::string>(name);
    char* c = new char[s.size()+1];
    strcpy(c, s.c_str());
    return c;
}

//! @brief Accesses a floating-point field of the storage by the string name of a tag.
double get_double(char const* name) {
    return storage_getter<double>(name);
}

//! @brief Accesses an integer field of the storage by the string name of a tag.
int get_int(char const* name) {
    return storage_getter<int>(name);
}

//! @brief Accesses an integer field of the storage by the string name of a tag.
int get_bool(char const* name) {
    return storage_getter<bool>(name);
}

//! @brief Updates a string field of the storage by the string name of a tag.
void set_string(char const* name, char const* val) {
    storage_setter(name, val);
}

//! @brief Updates a floating-point field of the storage by the string name of a tag.
void set_double(char const* name, double val) {
    storage_setter(name, val);
}

//! @brief Updates an integer field of the storage by the string name of a tag.
void set_int(char const* name, int val) {
    storage_setter(name, val);
}

//! @brief Updates an integer field of the storage by the string name of a tag.
void set_bool(char const* name, bool val) {
    storage_setter(name, val);
}

//! @brief Generic initialisation values.
auto init_v = common::make_tagged_tuple<option::nbr_lags, option::diameter, option::threshold, option::retain_time, option::round_period>(0, DIAMETER, (times_t)RETAIN_TIME, (times_t)RETAIN_TIME, ROUND_PERIOD);

//! @brief Starts FCPP, returning a pointer to the storage.
void start(JNIEnv *env, jclass apclazz, int uid, char const* experiment) {
    assert(running_experiment == "");
    env->GetJavaVM(&jvm);
    /* Cache some frequently used, expensive values: */
    clazz = apclazz;
    messageMe = env->GetStaticMethodID(clazz, "getNextMsg", "()[B");
    assert (messageMe != NULL);
    postMsg = env->GetStaticMethodID(clazz, "postMsg", "([BI)V");
    assert (postMsg!=NULL);

    // Sets the id.
    os::m_uid = uid;
    // Construct the network object and run the program until exit.
    running_experiment = experiment;
    applier(running_experiment, network, [](auto& n){
        n = new std::remove_pointer_t<std::decay_t<decltype(n)>>(init_v);
        t = std::thread([&n]() {
            n->run();
        });
    });
}

//! @brief Stops FCPP.
void stop() {
    applier(running_experiment, network, [](auto& n){
        lock_type l;
        n->node_at(os::uid(), l).terminate();
        t.join();
        delete n;
    });
    running_experiment = "";
}

// DEPRECATED

void start(JNIEnv *env, jclass apclazz, int uid) {
    start(env, apclazz, uid, "vulnerability_detection");
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
