// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file general.hpp
 * @brief Aggregate function tracking the basic functioning of the device.
 */

#ifndef FCPP_COORDINATION_GENERAL_H_
#define FCPP_COORDINATION_GENERAL_H_

#include <cassert>

#include "lib/coordination.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {


//! @brief Namespace containing the libraries of coordination routines.
namespace coordination {

//! @brief Tags used in the node storage.
namespace tags {
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
}

//! @brief Tracks the passage of time, maximum message size, message lags, and applies execution parameter changes.
FUN void tracker(ARGS) { CODE
    using namespace tags;
    node.next_time(node.current_time() + node.storage(round_period{}));
    node.message_threshold(node.storage(retain_time{}));
    node.storage(nbr_lags{}) = node.nbr_lag();
    node.storage(round_count{}) = counter(CALL, uint16_t{1});
    node.storage(global_clock{}) = shared_clock(CALL);
    node.storage(max_msg{}) = gossip_max(CALL, (uint16_t)min(node.msg_size(), size_t{255}));
}
//! @brief Export list for tracker.
FUN_EXPORT tracker_t = export_list<counter_t<uint16_t>, shared_clock_t, gossip_max_t<uint16_t>>;
//! @brief Storage list for tracker.
FUN_EXPORT tracker_s = storage_list<
    tags::round_period,       times_t,
    tags::retain_time,        times_t,
    tags::nbr_lags,           field<times_t>,
    tags::round_count,        uint16_t,
    tags::global_clock,       times_t,
    tags::max_msg,            uint8_t
>;

} // namespace coordination

} // namespace fcpp

#endif  // FCPP_COORDINATION_GENERAL_H_
