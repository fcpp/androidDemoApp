// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file friend_finding.hpp
 * @brief Experiment allowing to search for friends.
 */

#ifndef FCPP_COORDINATION_FRIEND_FINDING_H_
#define FCPP_COORDINATION_FRIEND_FINDING_H_

#include "lib/common/option.hpp"
#include "lib/component/calculus.hpp"
#include "lib/coordination/experiment_helper.hpp"
#include "lib/coordination/tracker.hpp"
#include "lib/coordination/slcs.hpp"
#include "lib/coordination/vulnerability_detection.hpp"


//! @brief Struct representing a request.
struct request {
    //! @brief Source UID.
    fcpp::device_t source;
    //! @brief Destination UID.
    fcpp::device_t destination;

    //! @brief Empty constructor.
    request() = default;

    //! @brief Member constructor.
    request(fcpp::device_t source, fcpp::device_t destination) : source(source), destination(destination) {}

    //! @brief Equality operator.
    bool operator==(request const& m) const {
        return source == m.source and destination == m.destination;
    }

    //! @brief Hash computation.
    size_t hash() const {
        constexpr size_t offs = sizeof(size_t)*CHAR_BIT/2;
        return (size_t(source) << (offs)) | size_t(destination);
    }

    //! @brief Serialises the content from/to a given input/output stream.
    template <typename S>
    S& serialize(S& s) {
        return s & source & destination;
    }

    //! @brief Serialises the content from/to a given input/output stream (const overload).
    template <typename S>
    S& serialize(S& s) const {
        return s << source << destination;
    }
};

namespace std {
    //! @brief Hasher object for the request struct.
    template <>
    struct hash<request> {
        //! @brief Produces an hash for a request, combining source and destination into a size_t.
        size_t operator()(request const& m) const {
            return m.hash();
        }
    };
}


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Tag for the friend finding experiment.
struct friend_finding {};


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Which friend has been requested (zero for no request).
    struct friend_requested {};
    //! @brief A score approximating the distance to the searched friend (if any, zero otherwise).
    struct distance_score {};
    //! @brief Hop-count distance to the searched friend (if any, zero otherwise).
    struct hop_distance {};
    //! @brief If a friend has already been found.
    struct friend_found {};
}


//! @brief Simulation logic of the friend finding experiment.
FUN void experiment_simulation(ARGS, friend_finding, common::bool_pack<true>) { CODE
    using namespace tags;
    vec<2> low = {0, 0};
    vec<2> hi = {hi_x, hi_y};
    if (counter(CALL) == 1)
        node.position() = node.uid == 0 ? 0.2*hi : node.uid == 1 ? 0.8*hi : random_rectangle_target(CALL, 0.2*hi, 0.8*hi);
    rectangle_walk(CALL, 0.2*hi, 0.8*hi, 0.2, node.storage(round_period{}));
    constexpr times_t request_time = 30;
    field<bool> is_my_friend = false;
    if (node.uid == 0 and node.current_time() > request_time and not node.storage(friend_found{})) {
        node.storage(friend_requested{}) = 1;
        follow_target(CALL, node.net.node_at(1).position(), 0.5, node.storage(round_period{}));
        is_my_friend = node.nbr_uid() == node.storage(friend_requested{});
    }
    if (any_hood(CALL, is_my_friend)) {
        node.storage(friend_found{}) = true;
        node.storage(friend_requested{}) = 0;
    }
    node.storage(node_size{}) = node.uid > 1 ? 1 : 2;
    node.storage(node_shape{}) = node.uid == 0 ? shape::cube : node.uid == 1 ? shape::star : shape::sphere;
    node.storage(node_color{}) = node.storage(friend_found{}) ? color(SEA_GREEN) : color(GRAY);
    if (node.current_time() > request_time) {
        real_t k = node.storage(distance_score{}) / node.storage(diameter{});
        if (node.uid == 1) node.storage(node_color{}) = color(GOLD);
        else if (k > 0) node.storage(node_color{}) = k*color(BLUE) + (1-k)*color(RED);
    }
}
//! @brief Export list for the simulation logic of the friend finding experiment.
template <>
struct experiment_simulation_t<friend_finding, true> : export_list<
    rectangle_walk_t<2>, counter_t<>
> {};
//! @brief Storage list for the simulation logic of the friend finding experiment.
template <>
struct experiment_simulation_s<friend_finding, true> : storage_list<
    experiment_simulation_base_s,
    tags::friend_found, bool
> {};


//! @brief Allows users to search for friends.
GEN(S) void experiment(ARGS, friend_finding, S) { CODE
    using namespace tags;
    node.storage(degree{}) = node.size() - 1;
    node.storage(hop_distance{}) = 0;
    node.storage(distance_score{}) = 0;
    bool requesting = node.current_time() > 2.5 and node.storage(friend_requested{}) > 0;
    common::option<request> key_set;
    if (requesting) key_set.emplace(node.uid, node.storage(friend_requested{}));
    auto res = spawn(CALL, [&](request r){
        bool source = node.uid == r.source;
        bool destination = node.uid == r.destination;
        hops_t h = abf_hops(CALL, destination);
        field<real_t> w = node.storage(retain_time{}) - node.nbr_lag();
        real_t d = sum_hood(CALL, nbr(CALL, h) * w) / sum_hood(CALL, w);
        node.storage(hop_distance{}) = h;
        node.storage(distance_score{}) = d;
        status s = abf_hops(CALL, source) < node.storage(diameter{}) ? status::internal : status::external;
        if (source) s = requesting ? status::output : status::terminated;
        return make_tuple(d, s);
    }, key_set);
    if (res.size() > 0) {
        assert(res.size() == 1 and res.begin()->first == *key_set.begin());
        node.storage(distance_score{}) = res.begin()->second;
    }
    experiment_simulation(CALL, friend_finding{}, S{});
}
//! @brief Export list for the friend finding experiment.
template <bool simulation>
struct experiment_t<friend_finding, simulation> : export_list<
    experiment_simulation_t<friend_finding, simulation>,
    spawn_t<request, status>, abf_hops_t, nbr_t<real_t>
> {};
//! @brief Storage list for the friend finding experiment.
template <bool simulation>
struct experiment_s<friend_finding, simulation> : storage_list<
    experiment_simulation_s<friend_finding, simulation>,
    tags::degree,           hops_t,
    tags::friend_requested, device_t,
    tags::distance_score,   real_t,
    tags::hop_distance,     hops_t,
    tags::diameter,         hops_t
> {};
//! @brief Aggregator list for the friend finding experiment.
template <>
struct experiment_a<friend_finding> : storage_list<
    tags::distance_score,   aggregator::mean<double>
> {};
//! @brief Plot list for the friend finding experiment.
template <typename A>
struct experiment_p<friend_finding,A> : public plot::split<plot::time, plot::values<A, common::type_sequence<>, tags::distance_score>> {};

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_FRIEND_FINDING_H_
