#pragma once

#include <algorithm>
#include <memory>
#include <string>

#include <ion.hpp>
#include <parameter_list.hpp>
#include <util/indirect.hpp>
#include <util/meta.hpp>
#include <util/make_unique.hpp>

namespace nest {
namespace mc {
namespace mechanisms {

enum class mechanismKind {point, density};

/// The mechanism type is templated on a memory policy type.
/// The only difference between the abstract definition of a mechanism on host
/// or gpu is the information is stored, and how it is accessed.
template <typename Backend>
class mechanism {
public:
    using backend = Backend;

    using value_type = typename backend::value_type;
    using size_type = typename backend::size_type;

    // define storage types
    using array = typename backend::array;
    using iarray = typename backend::iarray;

    using view = typename backend::view;
    using iview = typename backend::iview;

    using const_view = typename backend::const_view;
    using const_iview = typename backend::const_iview;

    using ion_type = ion<backend>;

    using multi_event_stream = typename backend::multi_event_stream;

    mechanism(size_type mech_id, const_iview vec_ci, const_view vec_t, const_view vec_t_to, const_view vec_dt, view vec_v, view vec_i, iarray&& node_index):
        mech_id_(mech_id),
        vec_ci_(vec_ci),
        vec_t_(vec_t),
        vec_t_to_(vec_t_to),
        vec_dt_(vec_dt),
        vec_v_(vec_v),
        vec_i_(vec_i),
        node_index_(std::move(node_index))
    {}

    std::size_t size() const {
        return node_index_.size();
    }

    const_iview node_index() const {
        return node_index_;
    }

    virtual void set_params() = 0;
    virtual std::string name() const = 0;
    virtual std::size_t memory() const = 0;
    virtual void nrn_init()     = 0;
    virtual void nrn_state()    = 0;
    virtual void nrn_current()  = 0;
    virtual void deliver_events(multi_event_stream& events) {};
    virtual bool uses_ion(ionKind) const = 0;
    virtual void set_ion(ionKind k, ion_type& i, const std::vector<size_type>& index) = 0;
    virtual mechanismKind kind() const = 0;

    // net_receive() is used internally by deliver_events(), but
    // is exposed primarily for unit testing.
    virtual void net_receive(int, value_type) {};

    virtual ~mechanism() = default;

    // Mechanism identifier: index into list of mechanisms on cell group.
    size_type mech_id_;

    // Maps compartment index to cell index.
    const_iview vec_ci_;
    // Maps cell index to integration start time.
    const_view vec_t_;
    // Maps cell index to integration stop time.
    const_view vec_t_to_;
    // Maps compartment index to (stop time) - (start time).
    const_view vec_dt_;
    // Maps compartment index to voltage.
    view vec_v_;
    // Maps compartment index to current.
    view vec_i_;
    // Maps mechanism instance index to compartment index.
    iarray node_index_;
};

template <class Backend>
using mechanism_ptr = std::unique_ptr<mechanism<Backend>>;

template <typename M>
auto make_mechanism(
    typename M::size_type mech_id,
    typename M::const_iview vec_ci,
    typename M::const_view vec_t,
    typename M::const_view vec_t_to,
    typename M::const_view vec_dt,
    typename M::view vec_v,
    typename M::view vec_i,
    typename M::array&& weights,
    typename M::iarray&& node_indices
)
DEDUCED_RETURN_TYPE(util::make_unique<M>(mech_id, vec_ci, vec_t, vec_t_to, vec_dt, vec_v, vec_i, std::move(weights), std::move(node_indices)))

} // namespace mechanisms
} // namespace mc
} // namespace nest
