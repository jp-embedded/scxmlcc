#include "test355.h"
#include "test576.h"
#include <gtest/gtest.h>

TEST(w3c, 355)
{
	sc_test355 sc;
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

TEST(w3c, 576)
{
	sc_test576 sc;
	EXPECT_EQ(&sc.m_state_pass, sc.cur_state);
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

