#pragma once

#include <cstdint>
#include <limits>
#include <ostream>
#include <queue>
#include <type_traits>
#include <utility>

#include "common_types.hpp"
#include "util/meta.hpp"
#include "util/optional.hpp"
#include "util/range.hpp"
#include "util/strprintf.hpp"

namespace nest {
namespace mc {

/* Event classes `Event` used with `event_queue` must be move and copy constructible,
 * and either have a public field `time` that returns the time value, or provide an
 * overload of `event_time(const Event&)` which returns this value.
 *
 * Time values must be well ordered with respect to `operator>`.
 */

struct postsynaptic_spike_event {
    cell_member_type target;
    time_type time;
    float weight;
};

struct sample_event {
    using size_type = std::uint32_t;

    size_type sampler_index;
    time_type time;
};

// Configuration point: define `event_time(ev)` for event objects `ev`
// that do not have the corresponding `time` member field.

template <typename Event>
auto event_time(const Event& ev) -> decltype(ev.time) {
    return ev.time;
}

namespace impl {
    using ::nest::mc::event_time;

    // wrap in `impl::` namespace to obtain correct ADL for return type.
    template <typename Event>
    using event_time_type = decltype(event_time(std::declval<Event>()));
}

template <typename Event>
class event_queue {
public :
    using value_type = Event;
    using event_time_type = impl::event_time_type<Event>;

    event_queue() {}

    void push(const value_type& e) {
         queue_.push(e);
    }

    void push(value_type&& e) {
         queue_.push(std::move(e));
    }

    bool empty() const {
        return size()==0;
    }

    std::size_t size() const {
        return queue_.size();
    }

    // Return time t of head of queue if `t_until` > `t`.
    util::optional<event_time_type> time_if_before(const event_time_type& t_until) {
        if (queue_.empty()) {
            return util::nothing;
        }

        using ::nest::mc::event_time;
        auto t = event_time(queue_.top());
        return t_until > t? util::just(t): util::nothing;
    }

    // Generic conditional pop: pop and return head of queue if
    // queue non-empty and the head satisfies predicate.
    template <typename Pred>
    util::optional<value_type> pop_if(Pred&& pred) {
        using ::nest::mc::event_time;
        if (!queue_.empty() && pred(queue_.top())) {
            auto ev = queue_.top();
            queue_.pop();
            return ev;
        }
        else {
            return util::nothing;
        }
    }

    // Pop and return top event `ev` of queue if `t_until` > `event_time(ev)`.
    util::optional<value_type> pop_if_before(const event_time_type& t_until) {
        using ::nest::mc::event_time;
        return pop_if(
            [&t_until](const value_type& ev) { return t_until > event_time(ev); }
        );
    }

    // Pop and return top event `ev` of queue unless `event_time(ev)` > `t_until`
    util::optional<value_type> pop_if_not_after(const event_time_type& t_until) {
        using ::nest::mc::event_time;
        return pop_if(
            [&t_until](const value_type& ev) { return !(event_time(ev) > t_until); }
        );
    }

    // Clear queue and free storage.
    void clear() {
        queue_ = decltype(queue_){};
    }

private:
    struct event_greater {
        bool operator()(const Event& a, const Event& b) {
            using ::nest::mc::event_time;
            return event_time(a) > event_time(b);
        }
    };

    std::priority_queue<
        Event,
        std::vector<Event>,
        event_greater
    > queue_;
};

} // namespace nest
} // namespace mc

inline std::ostream& operator<<(
    std::ostream& o, const nest::mc::postsynaptic_spike_event& e)
{
    return o << "event[" << e.target << "," << e.time << "," << e.weight << "]";
}
