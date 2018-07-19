#include "test144.h"
#include "test355.h"
#include "test364.h"
#include "test372.h"
#include "test412.h"
#include "test504.h"
#include "test505.h"
#include "test506.h"
#include "test533.h"
#include "test570.h"
#include "test375.h"
#include "test376.h"
#include "test576.h"
#include "event_list.h"
#include "event_tokens.h"
#include "conditional.h"
#include "../examples/timer_switch.h"
#include "lcca_parallel.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "all-tests.h"

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

