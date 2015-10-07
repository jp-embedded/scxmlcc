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

#include "machine.h"
#include "dispenser.h"
#include "coin_refund.h"
#include "input.h"
#include "coin_sensor.h"
#include "keypad.h"
#include "display.h"

#include <iostream>

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

	// connect input simulator to coin sensor and keypad
	input.sig_key.connect(&coin_sensor, &coin_sensor::simulate_input);
	input.sig_key.connect(&keypad, &keypad::simulate_input);

	// connect coin sensor to state machine
	coin_sensor.sig_dime.connect(bind(dispatch, &sc::state::event_D));
	coin_sensor.sig_nickel.connect(bind(dispatch, &sc::state::event_N));

	// connect keypad to state machine
	keypad.sig_coke.connect(bind(dispatch, &sc::state::event_coke));
	keypad.sig_diet.connect(bind(dispatch, &sc::state::event_diet));
	keypad.sig_zero.connect(bind(dispatch, &sc::state::event_zero));
	keypad.sig_cancel.connect(bind(dispatch, &sc::state::event_cancel));

	// connect dispenser to state machine
	m.sig_dispense_coke.connect(&dispenser, &dispenser::coke);
	m.sig_dispense_diet.connect(&dispenser, &dispenser::diet);
	m.sig_dispense_zero.connect(&dispenser, &dispenser::zero);

	// connect display co state machine
	m.sig_insert_coins.connect(&display, &display::insert);

	// connect coin refund to state machine
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

