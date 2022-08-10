#include "test144_ts.h"
#include "test355_ts.h"
#include "test364_ts.h"
#include "test372_ts.h"
#include "test412_ts.h"
#include "test504_ts.h"
#include "test505_ts.h"
#include "test506_ts.h"
#include "test533_ts.h"
#include "test570_ts.h"
#include "test375_ts.h"
#include "test376_ts.h"
#include "test377_ts.h"
#include "test378_ts.h"
#include "test576_ts.h"
#include "event_list_ts.h"
#include "event_tokens_ts.h"
#include "conditional_ts.h"
#include "../examples/timer_switch_ts.h"
#include "lcca_parallel_ts.h"
#include "issue_69_ts.h"
#include "issue_72_ts.h"
#include "issue_88_ts.h"
#include "issue_94_ts.h"
#include "issue_97_ts.h"
#include "eventless_ts.h"
#include "stringevents_ts.h"
#include "parallel_exit_ts.h"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

namespace ts {

#include "all-tests.h"

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

}
