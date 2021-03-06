#include <vector>
#include "../gtest.h"

#include <backends/event.hpp>
#include <backends/multicore/multi_event_stream.hpp>

using namespace nest::mc;

namespace common_events {
    // set up four targets across three streams and two mech ids.

    constexpr cell_local_size_type mech_1 = 10u;
    constexpr cell_local_size_type mech_2 = 13u;
    constexpr cell_size_type cell_1 = 20u;
    constexpr cell_size_type cell_2 = 77u;
    constexpr cell_size_type cell_3 = 33u;
    constexpr cell_size_type n_cell = 100u;

    target_handle handle[4] = {
        target_handle(mech_1, 0u, cell_1),
        target_handle(mech_2, 1u, cell_2),
        target_handle(mech_1, 4u, cell_2),
        target_handle(mech_2, 2u, cell_3)
    };

    // cell_1 (handle 0) has one event at t=3
    // cell_2 (handle 1 and 2) has two events at t=2 and t=5
    // cell_3 (handle 3) has one event at t=3

    std::vector<deliverable_event> events = {
        deliverable_event(2.f, handle[1], 2.f),
        deliverable_event(3.f, handle[0], 1.f),
        deliverable_event(3.f, handle[3], 4.f),
        deliverable_event(5.f, handle[2], 3.f)
    };
}

TEST(multi_event_stream, init) {
    using multi_event_stream = multicore::multi_event_stream;
    using namespace common_events;

    multi_event_stream m(n_cell);
    EXPECT_EQ(n_cell, m.n_streams());

    m.init(events);
    EXPECT_FALSE(m.empty());

    m.clear();
    EXPECT_TRUE(m.empty());
}

TEST(multi_event_stream, mark) {
    using multi_event_stream = multicore::multi_event_stream;
    using namespace common_events;

    multi_event_stream m(n_cell);
    ASSERT_EQ(n_cell, m.n_streams());

    m.init(events);

    for (cell_size_type i = 0; i<n_cell; ++i) {
        EXPECT_TRUE(m.marked_events(i).empty());
    }

    std::vector<time_type> t_until(n_cell);
    t_until[cell_1] = 2.f;
    t_until[cell_2] = 2.5f;
    t_until[cell_3] = 4.f;
    m.mark_until_after(t_until);

    // Only two events should be marked: 
    //     events[0] (with handle 1) at t=2.f on cell_2
    //     events[2] (with handle 3) at t=3.f on cell_3

    for (cell_size_type i = 0; i<n_cell; ++i) {
        auto evs = m.marked_events(i);
        auto n_marked = evs.size();
        switch (i) {
        case cell_2:
            EXPECT_EQ(1u, n_marked);
            EXPECT_EQ(handle[1].mech_id, evs.front().handle.mech_id);
            EXPECT_EQ(handle[1].index, evs.front().handle.index);
            break;
        case cell_3:
            EXPECT_EQ(1u, n_marked);
            EXPECT_EQ(handle[3].mech_id, evs.front().handle.mech_id);
            EXPECT_EQ(handle[3].index, evs.front().handle.index);
            break;
        default:
            EXPECT_EQ(0u, n_marked);
            break;
        }
    }

    // Drop these events and mark all events up to t=5.f.
    //     cell_1 should have one marked event (events[1], handle 0)
    //     cell_2 should have one marked event (events[3], handle 2)

    m.drop_marked_events();
    t_until.assign(n_cell, 5.f);
    m.mark_until_after(t_until);

    for (cell_size_type i = 0; i<n_cell; ++i) {
        auto evs = m.marked_events(i);
        auto n_marked = evs.size();
        switch (i) {
        case cell_1:
            EXPECT_EQ(1u, n_marked);
            EXPECT_EQ(handle[0].mech_id, evs.front().handle.mech_id);
            EXPECT_EQ(handle[0].index, evs.front().handle.index);
            break;
        case cell_2:
            EXPECT_EQ(1u, n_marked);
            EXPECT_EQ(handle[2].mech_id, evs.front().handle.mech_id);
            EXPECT_EQ(handle[2].index, evs.front().handle.index);
            break;
        default:
            EXPECT_EQ(0u, n_marked);
            break;
        }
    }

    // No more events after these.
    EXPECT_FALSE(m.empty());
    m.drop_marked_events();
    EXPECT_TRUE(m.empty());
}

TEST(multi_event_stream, time_if_before) {
    using multi_event_stream = multicore::multi_event_stream;
    using namespace common_events;

    multi_event_stream m(n_cell);
    ASSERT_EQ(n_cell, m.n_streams());

    m.init(events);

    // Test times less than all event times (first event at t=2).
    std::vector<double> before(n_cell);
    std::vector<double> after;

    for (unsigned i = 0; i<n_cell; ++i) {
	before[i] = 0.1+i/(double)n_cell;
    }

    std::vector<double> t(before);
    m.event_time_if_before(t);

    EXPECT_EQ(before, t);

    // With times between 2 and 3, expect the event at time t=2
    // on cell_2 to restrict corresponding element of t.

    for (unsigned i = 0; i<n_cell; ++i) {
	before[i] = 2.1+0.5*i/(double)n_cell;
    }
    t = before;
    m.event_time_if_before(t);

    for (unsigned i = 0; i<n_cell; ++i) {
	if (i==cell_2) {
	    EXPECT_EQ(2., t[i]);
	}
	else {
	    EXPECT_EQ(before[i], t[i]);
	}
    }
}
