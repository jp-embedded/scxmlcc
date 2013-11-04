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

URL := "$HeadURL: https://jp-embedded.com/svn/platform/trunk/src/fsm/makefile $"
URL_TRUNK := $(findstring trunk,$(URL))
URL_TAG := $(firstword $(subst /, ,$(word 3,$(subst tags, ,$(URL)))))
VER := $(URL_TRUNK)$(URL_TAG)

OBJS := main.o cpp_output.o scxml_parser.o version.o
CPPFLAGS := -Wall -O2 -MD -MP -DVER="$(VER)"

all: scxmlcc

scxmlcc: $(OBJS)
	$(CXX) -o $@ $^ -lboost_program_options -lboost_system -lboost_filesystem

clean:
	rm -f $(OBJS) $(OBJS:.o=.d) scxmlcc


-include $(OBJS:.o=.d)

