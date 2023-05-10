#include "test144_s.h"
#include "test355_s.h"
#include "test364_s.h"
#include "test372_s.h"
#include "test412_s.h"
#include "test504_s.h"
#include "test505_s.h"
#include "test506_s.h"
#include "test533_s.h"
#include "test570_s.h"
#include "test375_s.h"
#include "test376_s.h"
#include "test377_s.h"
#include "test378_s.h"
#include "test576_s.h"
#include "event_list_s.h"
#include "event_tokens_s.h"
#include "conditional_s.h"
#include "timer_switch_s.h"
#include "lcca_parallel_s.h"
#include "issue_69_s.h"
#include "issue_72_s.h"
#include "issue_88_s.h"
#include "issue_94_s.h"
#include "issue_97_s.h"
#include "eventless_s.h"
#include "stringevents_s.h"
#include "parallel_exit_s.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

namespace s {

#include "all-tests.h"

TEST(stringevents, 0)
{
	sc_stringevents sc;
	sc.init();

	// must trigger event "ev1"
	sc.dispatch("ev1.foo");
	EXPECT_TRUE(sc.model.In<sc_stringevents::state_State_2>());

	// must trigger event "ev2.1", not "ev2"
	sc.dispatch("ev2.1");
	EXPECT_TRUE(sc.model.In<sc_stringevents::state_State_3>());
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

}
