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
};

template <> void sc::state_actions<sc::state_select>::enter(sc::data_model &m)	{ m.user->sig_select(); }

int main()
{
	sc::user_model_p m(new sc::user_model);
	sc sc(m);
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
	sc.model.user->sig_dispense_coke.connect(&dispenser, &dispenser::coke);
	sc.model.user->sig_dispense_diet.connect(&dispenser, &dispenser::diet);
	sc.model.user->sig_dispense_zero.connect(&dispenser, &dispenser::zero);
	sc.model.user->sig_select.connect(&display, &display::select);

	sc.init();
	input.run();

	return 0;
}

