/*************************************************************************
 ** Copyright (C) 2013 Jan Pedersen <jp@jp-embedded.com>
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

#include "timer_switch.h"
#include "termio.h"
#include <iostream>

using namespace std;

typedef sc_timer_switch sc;

struct sc::user_model 
{
	int timer;
};

template<> void sc::state_actions<sc::state_on>::enter(sc::data_model &m)
{
	cout << "on" << endl;	
}

template<> void sc::state_actions<sc::state_off>::enter(sc::data_model &m)
{
	cout << "off" << endl;	
	m.user->timer = 0;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_on, sc::state_off>::condition(sc::data_model &m)
{
	return m.user->timer >= 5;
}

template<> void sc::transition_actions<&sc::state::event_timer, sc::state_on>::enter(sc::data_model &m)
{
	++m.user->timer;
}

int main(int argc, char *argv[])
{
	// set unbuffered input and disable echo
	termio t;
	t.echo(false);
	t.canonical(false);

	cout << "input:  b = button event" << endl;
      	cout << "        t = timer event" << endl;
	cout << "        q = quit" << endl;

	sc::user_model m;
	sc sc(&m);

	while(true) {
		char c;	
		cin >> c;

		sc::event e;
		switch (c) {
			case 'b': e = &sc::state::event_button; break;
			case 't': e = &sc::state::event_timer;  break;
			case 'q': return 0;
			default: continue;
		}
		sc.dispatch(e);
	}
	return 0;
}

