#include "test355.h"
#include "test412.h"
#include "test576.h"
#include "event_list.h"
#include "event_tokens.h"
#include <gtest/gtest.h>

TEST(w3c, 355)
{
	sc_test355 sc;
        sc.init();
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

TEST(w3c, 412)
{
	sc_test412 sc;
        sc.init();
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

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

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

