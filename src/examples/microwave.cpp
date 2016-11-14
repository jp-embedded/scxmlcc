/*************************************************************************
 ** Copyright (C) 2016 Jan Pedersen <jp@jp-embedded.com>
 ** 
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 ** 
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 ** 
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include "microwave-01-cplusplus.h"
#include "termio.h"
#include <iostream>

using namespace std;

typedef sc_microwave_01_cplusplus sc;

// add logging
template<> void sc::state_actions<sc::state_off>::enter(data_model &m)
{
	cout << "off" << endl;
}

template<> void sc::state_actions<sc::state_on>::enter(data_model &m)
{
	cout << "on" << endl;
}

template<> void sc::state_actions<sc::state_idle>::enter(data_model &m)
{
	cout << "idle" << endl;
}

template<> void sc::state_actions<sc::state_cooking>::enter(data_model &m)
{
	cout << "cooking" << endl;
}

int main(int argc, char *argv[])
{
	// set unbuffered input and disable echo
	termio t;
	t.echo(false);
	t.canonical(false);

	cout << "input:  P = turn on" << endl;
      	cout << "        p = turn off" << endl;
      	cout << "        D = open door" << endl;
      	cout << "        d = close door" << endl;
	cout << "        t = timer" << endl;
	cout << "        q = quit" << endl;

	sc sc;
	sc.init();

	while(true) {
		char c;	
		cin >> c;

		sc::event e;
		switch (c) {
			case 'P': e = &sc::state::event_turn_on;    break;
			case 'p': e = &sc::state::event_turn_off;   break;
			case 'D': e = &sc::state::event_door_open;  break;
			case 'd': e = &sc::state::event_door_close; break;
			case 't': e = &sc::state::event_time;       break;
			case 'q': return 0;
			default: continue;
		}
		sc.dispatch(e);
	}
	
	return 0;
}

