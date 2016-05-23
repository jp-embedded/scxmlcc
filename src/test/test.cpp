#include "test355.h"
#include "test372.h"
#include "test412.h"
#include "test576.h"
#include "event_list.h"
#include "event_tokens.h"
#include "conditional.h"
#include <gtest/gtest.h>

// test that default initial state is first in document order.  If we enter s0 first we succeed, if s1, failure.
TEST(w3c, 355)
{
	sc_test355 sc;
        sc.init();
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

// test that entering a final state generates done.state.parentid after executing the onentry elements.  
// Var1 should be set to 2 (but not 3) by the time the event is raised
TEST(w3c, 372)
{
	sc_test372 sc;
        sc.init();
}

// test that executable content in the <initial> transition executes after the onentry handler on the state
// and before the onentry handler of the child states.  Event1, event2, and event3 should occur in that order.
TEST(w3c, 412)
{
	sc_test412 sc;
        sc.init();
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}


// test that the 'initial' value of scxml is respected.  We set the value to deeply nested non-default parallel siblings and
// test that both are entered. 
TEST(w3c, 576)
{
	sc_test576 sc;
        sc.init();
	EXPECT_EQ(1, sc.cur_state.size());
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state.front());
}

TEST(event_list, event_a)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(&sc.m_state_init, sc.cur_state);
	sc.dispatch(&sc_event_list::state::event_a);
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

TEST(event_list, event_b)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(&sc.m_state_init, sc.cur_state);
	sc.dispatch(&sc_event_list::state::event_b);
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

TEST(event_list, event_c)
{
	sc_event_list sc;
	sc.init();
	EXPECT_EQ(&sc.m_state_init, sc.cur_state);
	sc.dispatch(&sc_event_list::state::event_c);
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

// test transitions through state a -> b -> c
TEST(event_tokens, 0)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(&sc.m_state_init, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(&sc.m_state_a, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_b);
        EXPECT_EQ(&sc.m_state_b, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_c);
        EXPECT_EQ(&sc.m_state_c, sc.cur_state);
}

TEST(event_tokens, 1)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(&sc.m_state_init, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(&sc.m_state_a, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but);
        EXPECT_EQ(&sc.m_state_but, sc.cur_state);
}

TEST(event_tokens, 2)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(&sc.m_state_init, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(&sc.m_state_a, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_c);
        EXPECT_EQ(&sc.m_state_but, sc.cur_state);
}

TEST(event_tokens, 3)
{
        sc_event_tokens sc;
        sc.init();
        EXPECT_EQ(&sc.m_state_init, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_but_a);
        EXPECT_EQ(&sc.m_state_a, sc.cur_state);
        sc.dispatch(&sc_event_tokens::state::event_dummy0);
        EXPECT_EQ(&sc.m_state_default, sc.cur_state);
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
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
}

TEST(conditional, 1)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.model.user->e0 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_end0, sc.cur_state);
}

TEST(conditional, 2)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.model.user->e1 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_end1, sc.cur_state);
}

TEST(conditional, 3)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_end2, sc.cur_state);
}

TEST(conditional, 4)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.model.user->e0 = true;
	sc.model.user->e1 = true;
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_end1, sc.cur_state);
}

TEST(conditional, 5)
{
	sc_conditional::user_model m;
	sc_conditional sc(&m);
	sc.init();
	EXPECT_EQ(&sc.m_state_ok0, sc.cur_state);
	sc.model.user->e0 = true;
	sc.model.user->e2 = true;
	sc.dispatch(&sc_conditional::state::event_a);
	EXPECT_EQ(&sc.m_state_end2, sc.cur_state);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

