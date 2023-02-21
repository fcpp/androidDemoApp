// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file tracker.hpp
 * @brief Aggregate function tracking the basic functioning of the device.
 */

#ifndef FCPP_COORDINATION_TRACKER_H_
#define FCPP_COORDINATION_TRACKER_H_

#include "lib/beautify.hpp"
#include "lib/coordination.hpp"
#include "lib/common/plot.hpp"
#include "lib/option/aggregator.hpp"
#include "lib/coordination/past_ctl.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
    //! @brief Upper bound to the node diameter.
    struct diameter;
    //! @brief The expected time of an evacuation.
    struct evacuation_time;
    //! @brief Whether I have ever seen somebody else.
    struct not_alone {};
    //! @brief Time in seconds between transmission rounds.
    struct round_period {};
    //! @brief The retain time for messages.
    struct retain_time {};
    //! @brief The list of lags from all neighbours available in the current round.
    struct nbr_lags {};
    //! @brief Total round count since start.
    struct round_count {};
    //! @brief A shared global clock.
    struct global_clock {};
    //! @brief Maximum message size ever experienced.
    struct max_msg {};
    //! @brief Maximum message currently experienced.
    struct cur_msg {};
}

//! @brief The tagged tuple of every possible experiment parameter.
using parameter_type = common::tagged_tuple_t<
    tags::round_period,     times_t,
    tags::retain_time,      times_t,
    tags::diameter,         hops_t,
    tags::evacuation_time,  short
>;
//! @brief The fcpp tuple of every possible experiment parameter.
using parameter_tuple = tuple<times_t, times_t, hops_t, short>;

//! @brief Tracks the passage of time, maximum message size, message lags, and applies execution parameter changes.
FUN void tracker(ARGS) { CODE
    using namespace tags;
    node.frequency(1.0/node.storage(round_period{}));
    node.message_threshold(node.storage(retain_time{}));
    if (node.current_time() > 1) {
        parameter_type pt = node.storage_tuple();
        parameter_tuple p = details::tuple_promote(pt);
        size_t neigh_num = count_hood(CALL);
        field<bool> same_params = nbr(CALL,p) == p;
        node.storage(not_alone{}) = logic::P(CALL, neigh_num > 1);
        node.storage(not_alone{}) += logic::P(CALL, not all_hood(CALL, same_params));
        node.storage(not_alone{}) += logic::P(CALL, 2 * sum_hood(CALL, same_params, 1) < neigh_num);
    }
    node.storage(nbr_lags{}) = node.nbr_lag();
    node.storage(round_count{}) = counter(CALL, uint16_t{1});
    node.storage(global_clock{}) = shared_clock(CALL);
    node.storage(cur_msg{}) = min(node.msg_size(), size_t{255});
    node.storage(max_msg{}) = gossip_max(CALL, (uint16_t)node.storage(cur_msg{}));
}
//! @brief Export list for tracker.
FUN_EXPORT tracker_t = export_list<
    past_ctl_t, parameter_tuple,
    counter_t<uint16_t>, shared_clock_t, gossip_max_t<uint16_t>
>;
//! @brief Storage list for tracker.
FUN_EXPORT tracker_s = storage_list<
    tags::not_alone,        uint8_t,
    tags::round_period,     times_t,
    tags::retain_time,      times_t,
    tags::nbr_lags,         field<times_t>,
    tags::round_count,      uint16_t,
    tags::global_clock,     times_t,
    tags::cur_msg,          uint8_t,
    tags::max_msg,          uint8_t
>;
//! @brief Aggregator list for tracker.
FUN_EXPORT tracker_a = storage_list<
    tags::cur_msg,  aggregator::mean<double>,
    tags::max_msg,  aggregator::mean<double>
>;
//! @brief Plot list for tracker.
GEN_EXPORT(A) tracker_p = plot::split<plot::time, plot::values<A, common::type_sequence<>, tags::cur_msg, tags::max_msg>>;

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_TRACKER_H_
