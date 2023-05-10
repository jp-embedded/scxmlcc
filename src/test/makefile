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

all: test

CPPFLAGS := -Wall -Wpedantic -Wold-style-cast -Wshadow -O2 -MD -MP
CPPFLAGS += -std=c++17 -isystem gtest-1.8.1/include -pthread

test.objs := test.o test_t.o test_s.o test_ts.o

test.txml := $(wildcard *.txml)
test.scxml := $(wildcard *.scxml) 

# Using timer_switch from examples
test.scxml += ../examples/timer_switch.scxml

test.gen := $(patsubst %.txml,%.h,$(test.txml))
test.gen += $(patsubst %.scxml,%.h,$(test.scxml))
test.gen += $(patsubst %.txml,%_t.h,$(test.txml))
test.gen += $(patsubst %.scxml,%_t.h,$(test.scxml))
test.gen += $(patsubst %.txml,%_s.h,$(test.txml))
test.gen += $(patsubst %.scxml,%_s.h,$(test.scxml))
test.gen += $(patsubst %.txml,%_ts.h,$(test.txml))
test.gen += $(patsubst %.scxml,%_ts.h,$(test.scxml))

clean: gtest.clean
	rm -f test
	rm -f $(patsubst %.txml,%.scxml,$(wildcard *.txml))
	rm -f $(test.gen)
	rm -f $(test.objs) $(test.objs:.o=.d)
	rm -f timer_switch.scxml

$(test.objs): libgtest.a | $(test.gen)

test: $(test.objs)
	$(CXX) -o $@ $^ libgtest.a -lpthread

-include $(test.objs:.o=.d)

include scxml.mk
include gtest.mk

%.scxml: %.txml
	xsltproc cpp.xsl $^ > $@

