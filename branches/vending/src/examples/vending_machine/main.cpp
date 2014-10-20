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

typedef sc_vending_machine sc;

struct sc::user_model
{
	signal<> sig_dispense_coke;
	signal<> sig_dispense_diet;
	signal<> sig_dispense_zero;
	signal<> sig_select;
	signal<int> sig_insert_coins;
	int debit;
};

template <> void sc::state_actions<sc::state_select>::enter(sc::data_model &m)			{ m.user->sig_select(); }

// todo move to collect_coins and make N, D with target
template <> void sc::state_actions<sc::state_show_price>::enter(sc::data_model &m)		{ m.user->sig_insert_coins(m.user->debit); }

template <> void sc::transition_actions<&sc::state::event_coke, sc::state_select, sc::state_active>::enter(sc::data_model &m)	{ m.user->debit = 15; }
template <> void sc::transition_actions<&sc::state::event_zero, sc::state_select, sc::state_active>::enter(sc::data_model &m)	{ m.user->debit = 15; }
template <> void sc::transition_actions<&sc::state::event_diet, sc::state_select, sc::state_active>::enter(sc::data_model &m)	{ m.user->debit = 15; }

template <> void sc::transition_actions<&sc::state::event_N, sc::state_collect_coins>::enter(sc::data_model &m)	
{
	m.user->debit -= 5;
	m.user->sig_insert_coins(m.user->debit);
}

template <> void sc::transition_actions<&sc::state::event_D, sc::state_collect_coins>::enter(sc::data_model &m)	
{
	m.user->debit -= 10;
	m.user->sig_insert_coins(m.user->debit);
}

template <> void sc::state_actions<sc::state_dispense>::enter(sc::data_model &m)
{
	//tood this should be added automatically
	m.event_queue.push(&sc::state::event_final);
}

template <> bool sc::transition_actions<&sc::state::unconditional, sc::state_collect_coins, sc::state_dispense>::condition(sc::data_model &m) { return m.user->debit <= 0; }	

int main()
{
	sc::user_model m;
	sc sc(&m);
	input input;
	coin_sensor coin_sensor;
	keypad keypad;
	dispenser dispenser;
	display display;

	functor<sc::event> dispatch(&sc, &sc::dispatch);

	input.sig_key.connect(&coin_sensor, &coin_sensor::simulate_input);
	input.sig_key.connect(&keypad, &keypad::simulate_input);
	coin_sensor.sig_dime.connect(bind(dispatch, &sc::state::event_D));
	coin_sensor.sig_nickel.connect(bind(dispatch, &sc::state::event_N));
	keypad.sig_coke.connect(bind(dispatch, &sc::state::event_coke));
	keypad.sig_diet.connect(bind(dispatch, &sc::state::event_diet));
	keypad.sig_zero.connect(bind(dispatch, &sc::state::event_zero));
	m.sig_dispense_coke.connect(&dispenser, &dispenser::coke);
	m.sig_dispense_diet.connect(&dispenser, &dispenser::diet);
	m.sig_dispense_zero.connect(&dispenser, &dispenser::zero);
	m.sig_select.connect(&display, &display::select);
	m.sig_insert_coins.connect(&display, &display::insert);

	sc.init();
	input.run();

	return 0;
}

