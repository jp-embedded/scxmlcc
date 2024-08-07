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

SCXMLCC:=../scxmlcc

%.h: %.scxml
	$(SCXMLCC) --stringevents -o $@ $<

%_t.h: %.scxml
	$(SCXMLCC) --threadsafe   -n t -o $@ $<

%_s.h: %.scxml
	$(SCXMLCC) --stringevents -n s -o $@ $<

%_b.h: %.scxml
	$(SCXMLCC) --baremetal -n b -o $@ $<

%_ts.h: %.scxml
	$(SCXMLCC) --threadsafe --stringevents -n ts -o $@ $<

