// Copyright © 2023 Giorgio Audrito. All Rights Reserved.

/**
 * @file configuration.hpp
 * @brief Main aggregate function, calling a given experiment.
 */

#ifndef FCPP_CONFIGURATION_H_
#define FCPP_CONFIGURATION_H_

//! @brief initial diameter upper bound
#define DIAMETER     10
//! @brief initial retain time
#define RETAIN_TIME  5
//! @brief initial round period
#define ROUND_PERIOD 0.5

#include "lib/component.hpp"
#include "lib/deployment.hpp"
#include "lib/coordination/main.hpp"
#include "lib/data.hpp"
#include "lib/simulation.hpp"


/**
 * @brief Namespace containing all the objects in the FCPP library.
 */
namespace fcpp {

//! @brief Namespace for component options.
namespace option {

//! @brief Import tags to be used for component options.
using namespace component::tags;
//! @brief Import tags used by aggregate functions.
using namespace coordination::tags;

//! @brief Main FCPP option setup.
template <typename tag, bool simulation = false>
DECLARE_OPTIONS(main,
    program<coordination::main<tag, simulation>>,
    exports<coordination::main_t<tag, simulation>>,
    tuple_store<coordination::main_s<tag, simulation>>,
    round_schedule<sequence::periodic_n<1, 0, 1>>,    // the sequence generator for round events on the network
    retain<metric::retain<RETAIN_TIME>>,
    message_push<false>
);

//! @brief The communication model (50% loss at 10m, 99% loss at 15m).
using connector_t = connect::radial<10,connect::fixed<15>>;

//! @brief The sequence of spawn events (100 devices in interval from 0 to 1s)
using spawn_s = sequence::multiple<distribution::constant_n<size_t, 100>, distribution::interval_n<times_t, 0, 1>, false>;

//! @brief The distribution of initial node positions (random in a 120x80 rectangle).
using rectangle_d = distribution::rect_n<1, 0, 0, hi_x, hi_y>;

template <typename tag>
DECLARE_OPTIONS(simulation,
    main<tag, true>,     // import general options
    parallel<false>,     // multithreading enabled on node rounds
    synchronised<false>, // optimise for asynchronous networks
    message_size<true>,  // emulate message sizes
    log_schedule<sequence::periodic_n<1, 0, 1>>,    // the sequence generator for log events on the network
    coordination::main_a<tag>,                      // the tags and corresponding aggregators to be logged
    plot_type<coordination::main_p<tag>>,           // the plot description
    connector<connector_t>, // probabilistic communication model
    area<0, 0, hi_x, hi_y>, // bounding coordinates of the simulated space
    shape_tag<node_shape>,  // the shape of a node is read from this tag in the store
    size_tag<node_size>,    // the size  of a node is read from this tag in the store
    color_tag<node_color>,  // the color of a node is read from this tag in the store
    spawn_schedule<spawn_s>,// definition of the spawn events
    init<
        x,              rectangle_d, // random displacement of devices in the simulation area
        diameter,       distribution::constant_i<int, diameter>,        // pass net construction parameters to nodes
        threshold,      distribution::constant_i<times_t, retain_time>, // pass net construction parameters to nodes
        retain_time,    distribution::constant_i<times_t, retain_time>, // pass net construction parameters to nodes
        round_period,   distribution::constant_i<times_t, round_period> // pass net construction parameters to nodes
    >
);

} // namespace option

//! @brief The list of supported experiments.
using experiments = common::type_sequence<
    traitor_detection,
    friend_finding,
    vulnerability_detection
>;

} // namespace fcpp

#endif  // FCPP_CONFIGURATION_H_
