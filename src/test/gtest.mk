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

GTEST:=gtest-1.7.0

libgtest.a: gtest-all.o
	ar -rv $@ $^

gtest-all.o: $(GTEST)/.unzip
	$(CXX) -isystem $(GTEST)/include/ -I $(GTEST) -pthread -c $(GTEST)/src/gtest-all.cc

$(GTEST)/.unzip: $(GTEST).zip
	unzip -q -o $^
	touch $@

gtest.clean:
	rm -rf $(GTEST) gtest-all.o libgtest.a

