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

CPPFLAGS := -Wall -Wpedantic -Wold-style-cast -Wshadow -O2 -MD -MP
CPPFLAGS += -std=c++17 -isystem gtest-1.8.1/include -pthread

test.objs := test.o test_t.o test_s.o test_ts.o

all: test

clean: gtest.clean
	rm -f test
	rm -f $(patsubst %.txml,%.h,$(wildcard *.txml))
	rm -f $(patsubst %.scxml,%.h,$(wildcard *.scxml))
	rm -f $(patsubst %.txml,%_t.h,$(wildcard *.txml))
	rm -f $(patsubst %.scxml,%_t.h,$(wildcard *.scxml))
	rm -f $(patsubst %.txml,%.scxml,$(wildcard *.txml))
	rm -f $(test.objs) $(test.objs:.o=.d)

$(test.objs): libgtest.a

test: $(test.objs)
	$(CXX) -o $@ $^ libgtest.a -lpthread

-include $(test.objs:.o=.d)

include scxml.mk
include gtest.mk

%.scxml: %.txml
	xsltproc cpp.xsl $^ > $@

# Initial depedency files must be generated to be able to auto generate .h from .scxml
%.d: %.cpp
	$(CXX) -std=c++17 -MM -MG $^ > $@

