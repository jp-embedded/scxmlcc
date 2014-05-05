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

APPS := timer_switch hello_world

timer_switch.objs := timer_switch.o
hello_world.objs := hello_world.o

all: $(APPS)
clean: $(APPS:=.clean)
	rm -f $(patsubst %.scxml,%.h,$(wildcard *.scxml))

define TARGET
$1: $($1.objs)
	$(CXX) -o $1 $($1.objs)

$1.clean:
	rm -f $1 $($1.objs) $($1.objs:.o=.d)

-include $($1.objs:.o=.d)
endef
$(foreach app,$(APPS),$(eval $(call TARGET,$(app))))

include scxml.mk

# Initial depedency files must be generated to be able to auto generate .h from .scxml
%.d: %.cpp
	$(CXX) -MM -MG $^ > $@

