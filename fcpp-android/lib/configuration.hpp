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
#define RETAIN_TIME  10
//! @brief initial round period
#define ROUND_PERIOD 1

#include "lib/component.hpp"
#include "lib/deployment.hpp"
#include "lib/coordination/main.hpp"


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

} // namespace fcpp

#endif  // FCPP_CONFIGURATION_H_
