#*************************************************************************
#** Copyright (C) 2013 Jan Pedersen <jp@jp-embedded.com>
#** 
#** This program is free software: you can redistribute it and/or modify
#** it under the terms of the GNU General Public License as published by
#** the Free Software Foundation, either version 3 of the License, or
#** (at your option) any later version.
#** 
#** This program is distributed in the hope that it will be useful,
#** but WITHOUT ANY WARRANTY; without even the implied warranty of
#** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#** GNU General Public License for more details.
#** 
#** You should have received a copy of the GNU General Public License
#** along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*************************************************************************

CPPFLAGS := -Wall -Wpedantic -O2 -MD -MP
CXXFLAGS := -std=c++17
APPS := timer_switch hello_world microwave-01 microwave-02 vending_machine/vending_machine

timer_switch.objs := timer_switch.o
timer_switch.gen := timer_switch.h

hello_world.objs := hello_world.o
hello_world.gen := hello_world.h

microwave-01.objs := microwave-01.o
microwave-01.gen := microwave-01-cplusplus.h

microwave-02.objs := microwave-02.o
microwave-02.gen := microwave-02-cplusplus.h

vending_machine/vending_machine.objs := $(patsubst %.cpp,%.o,$(wildcard vending_machine/*.cpp))
vending_machine/vending_machine.gen := vending_machine/vending_machine.h

# These should compile without RTTI
$(timer_switch.objs) : CPPFLAGS += -fno-rtti
$(hello_world.objs) : CPPFLAGS += -fno-rtti
$(microwave-01.objs) : CPPFLAGS += -fno-rtti
$(vending_machine/vending_machine.objs) : CPPFLAGS += -fno-rtti

all: $(APPS)
clean: $(APPS:=.clean)
	rm -f $(patsubst %.scxml,%.h,$(wildcard *.scxml))
	rm -f $(patsubst %.scxml,%.h,$(wildcard */*.scxml))

define TARGET
$1: $($1.objs)
	$(CXX) -o $$@ $$^

$1.clean:
	rm -f $1 $($1.objs) $($1.objs:.o=.d)

$($1.objs): | $($1.gen)

-include $($1.objs:.o=.d)
endef
$(foreach app,$(APPS),$(eval $(call TARGET,$(app))))

include scxml.mk

