#include "test144.h"
#include "test355.h"
#include "test372.h"
#include "test412.h"
//#include "test504.h"
#include "test505.h"
#include "test506.h"
//#include "test533.h"
#include "test576.h"
#include "event_list.h"
#include "event_tokens.h"
#include "conditional.h"
#include "../examples/timer_switch.h"
#include <gtest/gtest.h>

// test that default initial state is first in document order.  If we enter s0 first we succeed, if s1, failure.
TEST(w3c, 355)
{
	sc_test355 sc;
        sc.init();
	EXPECT_EQ(typeid(sc_test355::state_pass), typeid(*sc.cur_state));
}

// test that entering a final state generates done.state.parentid after executing the onentry elements.  
// Var1 should be set to 2 (but not 3) by the time the event is raised
TEST(w3c, 372)
{
	sc_test372 sc;
        sc.init();
	EXPECT_EQ(typeid(sc_test372::state_pass), typeid(*sc.cur_state));
}

// test that executable content in the <initial> transition executes after the onentry handler on the state
// and before the onentry handler of the child states.  Event1, event2, and event3 should occur in that order.
TEST(w3c, 412)
{
	sc_test412 sc;
        sc.init();
	EXPECT_EQ(typeid(sc_test412::state_pass), typeid(*sc.cur_state));
}

// if [a transition's] 'type' is "external", its exit set consists of all active states that are proper descendents of the Least Common Compound Ancestor (LCCA) of the source and target states.
TEST(w3c, 504)
{
	// todo
}

// Otherwise, if the transition has 'type' "internal", its source state is a compound state and all its target states are proper descendents of its source state, the target set consists of all active states that are proper descendents of its source state.
TEST(w3c, 505)
{
	sc_test505 sc;
	sc.init();
	EXPECT_EQ(typeid(sc_test505::state_pass), typeid(*sc.cur_state));
}

// If a transition has 'type' of "internal", but its source state is not a compound state or its target states are not all proper descendents of its source state, its exit set is defined as if it had 'type' of "external".
TEST(w3c, 506)
{
	sc_test506 sc;
	sc.init();
	EXPECT_EQ(typeid(sc_test506::state_pass), typeid(*sc.cur_state));
	// todo s2 does not exit twice
}

// If a transition has 'type' of "internal", but its source state is not a compound state, its exit set is defined as if it had 'type' of "external".
TEST(w3c, 533)
{
	// todo
}

// test that the 'initial' value of scxml is respected.  We set the value to deeply nested non-default parallel siblings and
// test that both are entered. 
TEST(w3c, 576)
{
	sc_test576 sc;
        sc.init();
	EXPECT_EQ(1, sc.cur_state.size());
	EXPECT_EQ(typeid(sc_test576::state_pass), typeid(*sc.cur_state.front()));
}

TEST(event_list, event_a)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(typeid(sc_event_list::state_init), typeid(*sc.cur_state));
	sc.dispatch(&sc_event_list::state::event_a);
	EXPECT_EQ(typeid(sc_event_list::state_pass), typeid(*sc.cur_state));
}

TEST(event_list, event_b)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(typeid(sc_event_list::state_init), typeid(*sc.cur_state));
	sc.dispatch(&sc_event_list::state::event_b);
	EXPECT_EQ(typeid(sc_event_list::state_pass), typeid(*sc.cur_state));
}

TEST(event_list, event_c)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(typeid(sc_event_list::state_init), typeid(*sc.cur_state));
	sc.dispatch(&sc_event_list::state::event_c);
	EXPECT_EQ(typeid(sc_event_list::state_pass), typeid(*sc.cur_state));
}

// test transitions through state a -> b -> c
TEST(event_tokens, 0)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(typeid(sc_event_tokens::state_init), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(typeid(sc_event_tokens::state_a), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_b);
        EXPECT_EQ(typeid(sc_event_tokens::state_b), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_c);
        EXPECT_EQ(typeid(sc_event_tokens::state_c), typeid(*sc.cur_state));
}

TEST(event_tokens, 1)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(typeid(sc_event_tokens::state_init), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(typeid(sc_event_tokens::state_a), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but);
        EXPECT_EQ(typeid(sc_event_tokens::state_but), typeid(*sc.cur_state));
}

TEST(event_tokens, 2)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(typeid(sc_event_tokens::state_init), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(typeid(sc_event_tokens::state_a), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_c);
        EXPECT_EQ(typeid(sc_event_tokens::state_but), typeid(*sc.cur_state));
}

TEST(event_tokens, 3)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(typeid(sc_event_tokens::state_init), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(typeid(sc_event_tokens::state_a), typeid(*sc.cur_state));
        sc.dispatch(&sc_event_tokens::state::event_dummy0);
        EXPECT_EQ(typeid(sc_event_tokens::state_default), typeid(*sc.cur_state));
}

struct sc_conditional::user_model
{
	bool e0, e1, e2;
	user_model() : e0(false), e1(false), e2(false) {}
};

template<> bool sc_conditional::transition_actions<&sc_conditional::state::event_a, sc_conditional::state_cluster, sc_conditional::state_end0>::condition(sc_conditional::data_model &m)
{
	return m.user->e0;
}

template<> bool sc_conditional::transition_actions<&sc_conditional::state::event_a, sc_conditional::state_ok0, sc_conditional::state_end1>::condition(sc_conditional::data_model &m)
{
	return m.user->e1;
}

template<> bool sc_conditional::transition_actions<&sc_conditional::state::event_a, sc_conditional::state_ok0, sc_conditional::state_end2>::condition(sc_conditional::data_model &m)
{
	return m.user->e2;
}

TEST(conditional, 0)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
}

TEST(conditional, 1)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
	sc.model.user->e0 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_end0), typeid(*sc.cur_state));
}

TEST(conditional, 2)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional ::state_ok0), typeid(*sc.cur_state));
	sc.model.user->e1 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_end1), typeid(*sc.cur_state));
}

TEST(conditional, 3)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_end2), typeid(*sc.cur_state));
}

TEST(conditional, 4)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
	sc.model.user->e0 = true;
	sc.model.user->e1 = true;
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_end1), typeid(*sc.cur_state));
}

TEST(conditional, 5)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_conditional::state_ok0), typeid(*sc.cur_state));
	sc.model.user->e0 = true;
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(typeid(sc_conditional::state_end2), typeid(*sc.cur_state));
}

struct sc_timer_switch::user_model 
{
	int timer;
};

template<> void sc_timer_switch::state_actions<sc_timer_switch::state_off>::enter(sc_timer_switch::data_model &m)
{
	m.user->timer = 0;
}

template<> bool sc_timer_switch::transition_actions<&sc_timer_switch::state::unconditional, sc_timer_switch::state_on, sc_timer_switch::state_off>::condition(sc_timer_switch::data_model &m)
{
	return m.user->timer >= 5;
}

template<> void sc_timer_switch::transition_actions<&sc_timer_switch::state::event_timer, sc_timer_switch::state_on>::enter(sc_timer_switch::data_model &m)
{
	++m.user->timer;
}

TEST(timer_switch, 0)
{
	sc_timer_switch::user_model m;
	sc_timer_switch sc(&m);
	sc.init();
	EXPECT_EQ(typeid(sc_timer_switch::state_off), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_button);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_button);
	EXPECT_EQ(typeid(sc_timer_switch::state_off), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_button);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_timer);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_timer);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_timer);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_timer);
	EXPECT_EQ(typeid(sc_timer_switch::state_on), typeid(*sc.cur_state));
	sc.dispatch(&sc_timer_switch::state::event_timer);
	EXPECT_EQ(typeid(sc_timer_switch::state_off), typeid(*sc.cur_state));
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

