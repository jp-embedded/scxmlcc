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

#include "timer-switch.h"
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
	return m.user->timer > 5;
}

template<> void sc::transition_actions<&sc::state::event_timer, sc::state_on>::enter(sc::data_model &m){
	++m.user->timer;
}

int main(int argc, char *argv[])
{
	sc::user_model_p m(new sc::user_model);
	sc sc0(m);

	sc0.dispatch(&sc::state::event_button);
	sc0.dispatch(&sc::state::event_timer);
	sc0.dispatch(&sc::state::event_timer);
	sc0.dispatch(&sc::state::event_timer);
	sc0.dispatch(&sc::state::event_timer);
	sc0.dispatch(&sc::state::event_timer);
	sc0.dispatch(&sc::state::event_timer);
}

