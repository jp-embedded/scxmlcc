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

CPPFLAGS := -Wall -O2 -MD -MP

APPS := timer_switch hello_world vending_machine/vending_machine

timer_switch.objs := timer_switch.o
hello_world.objs := hello_world.o
vending_machine/vending_machine.objs := $(patsubst %.cpp,%.o,$(wildcard vending_machine/*.cpp))

all: $(APPS)
clean: $(APPS:=.clean)
	rm -f $(patsubst %.scxml,%.h,$(wildcard *.scxml))
	rm -f $(patsubst %.scxml,%.h,$(wildcard */*.scxml))

define TARGET
$1: $($1.objs)
	$(CXX) -o $$@ $$^

$1.clean:
	rm -f $1 $($1.objs) $($1.objs:.o=.d)

-include $($1.objs:.o=.d)
endef
$(foreach app,$(APPS),$(eval $(call TARGET,$(app))))

include scxml.mk

#todo auto generate these
vending_machine/main.cpp: vending_machine/vending_machine.h
timer_switch.cpp: timer_switch.h
hello_world.cpp: hello_world.h

