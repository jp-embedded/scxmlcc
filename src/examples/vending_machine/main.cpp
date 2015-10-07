/*************************************************************************
 ** Copyright (C) 2014 Jan Pedersen <jp@jp-embedded.com>
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

#include "vending_machine.h"
#include "dispenser.h"
#include "coin_refund.h"
#include "input.h"
#include "coin_sensor.h"
#include "keypad.h"
#include "display.h"

#include <iostream>

typedef sc_vending_machine sc;

struct sc::user_model
{
	signal<> sig_dispense_coke;
	signal<> sig_dispense_diet;
	signal<> sig_dispense_zero;
	signal<int> sig_insert_coins;
	signal<> sig_refund_nickel;
	signal<> sig_refund_dime;
	int credit;
	user_model() : credit(0) {}
};

template<> void sc::transition_actions<&sc::state::event_N, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
{ 
	m.user->credit += 5; 
}

template<> void sc::transition_actions<&sc::state::event_D, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
{ 
	m.user->credit += 10; 
}

template<> void sc::state_actions<sc::state_collect_coins>::enter(sc::data_model &m) 		
{ 
	m.user->sig_insert_coins(m.user->credit); 
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_d, sc::state_return_n>::condition(sc::data_model &m)
{
	return m.user->credit < 10;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_d>::condition(sc::data_model &m)
{
	return m.user->credit >= 10;
}

template<> void sc::transition_actions<&sc::state::unconditional, sc::state_return_d>::enter(sc::data_model &m)
{
	m.user->sig_refund_dime();
	m.user->credit -= 5;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_n, sc::state_return_done>::condition(sc::data_model &m)
{
	return m.user->credit < 5;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_n>::condition(sc::data_model &m)
{
	return m.user->credit >= 5;
}

template<> void sc::transition_actions<&sc::state::unconditional, sc::state_return_n>::enter(sc::data_model &m)
{
	m.user->sig_refund_nickel();
	m.user->credit -= 5;
}

template<> void sc::state_actions<sc::state_return_done>::enter(sc::data_model &m)
{
	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> bool sc::transition_actions<&sc::state::event_zero, sc::state_collect_coins, sc::state_dispense_zero>::condition(sc::data_model &m)
{
	return m.user->credit >= 15;
}

template<> bool sc::transition_actions<&sc::state::event_coke, sc::state_collect_coins, sc::state_dispense_coke>::condition(sc::data_model &m)
{
	return m.user->credit >= 15;
}

template<> bool sc::transition_actions<&sc::state::event_diet, sc::state_collect_coins, sc::state_dispense_diet>::condition(sc::data_model &m)
{
	return m.user->credit >= 15;
}

template<> void sc::state_actions<sc::state_dispense_coke>::enter(sc::data_model &m)
{
	m.user->sig_dispense_coke();
	m.user->credit -= 15;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> void sc::state_actions<sc::state_dispense_diet>::enter(sc::data_model &m)
{
	m.user->sig_dispense_diet();
	m.user->credit -= 15;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> void sc::state_actions<sc::state_dispense_zero>::enter(sc::data_model &m)
{
	m.user->sig_dispense_zero();
	m.user->credit -= 15;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

int main()
{
	sc::user_model m;
	sc sc(&m);
	input input;
	coin_sensor coin_sensor;
	keypad keypad;
	dispenser dispenser;
	display display;
	coin_refund coin_refund;

	functor<sc::event> dispatch(&sc, &sc::dispatch);

	input.sig_key.connect(&coin_sensor, &coin_sensor::simulate_input);
	input.sig_key.connect(&keypad, &keypad::simulate_input);
	coin_sensor.sig_dime.connect(bind(dispatch, &sc::state::event_D));
	coin_sensor.sig_nickel.connect(bind(dispatch, &sc::state::event_N));
	keypad.sig_coke.connect(bind(dispatch, &sc::state::event_coke));
	keypad.sig_diet.connect(bind(dispatch, &sc::state::event_diet));
	keypad.sig_zero.connect(bind(dispatch, &sc::state::event_zero));
	keypad.sig_cancel.connect(bind(dispatch, &sc::state::event_cancel));
	m.sig_dispense_coke.connect(&dispenser, &dispenser::coke);
	m.sig_dispense_diet.connect(&dispenser, &dispenser::diet);
	m.sig_dispense_zero.connect(&dispenser, &dispenser::zero);
	m.sig_insert_coins.connect(&display, &display::insert);
	m.sig_refund_dime.connect(&coin_refund, &coin_refund::dime);
	m.sig_refund_nickel.connect(&coin_refund, &coin_refund::nickel);

	std::cout << "input:  1 = coke button" << std::endl;
	std::cout << "        2 = zero button" << std::endl;
	std::cout << "        3 = light button" << std::endl;
	std::cout << "        c = cancel button" << std::endl;
	std::cout << "        n = insert nickel" << std::endl;
	std::cout << "        d = inserd dime" << std::endl;
	std::cout << "        q = quit" << std::endl;

	sc.init();
	input.run();

	return 0;
}

