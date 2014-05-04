#include "test355.h"
#include "test576.h"
#include "test412.h"
#include <gtest/gtest.h>

TEST(w3c, 355)
{
	sc_test355 sc;
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

TEST(w3c, 576)
{
	sc_test576 sc;
	sc.dispatch(&sc_test576::state::event_In_s11p112);
	EXPECT_EQ(1, sc.cur_state.size());
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state.front());
}

TEST(w3c, 412)
{
	sc_test412 sc;
	sc.dispatch(&sc_test412::state::event_event1);	// todo: these should be sent from the sc itself
	sc.dispatch(&sc_test412::state::event_event2);
	sc.dispatch(&sc_test412::state::event_event3);
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

