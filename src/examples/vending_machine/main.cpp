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

using namespace std::placeholders;

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

	// connect input simulator to coin sensor and keypad
	input.sig_key.connect(std::bind(&coin_sensor::simulate_input, &coin_sensor, _1));
	input.sig_key.connect(std::bind(&keypad::simulate_input, &keypad, _1));

	// connect coin sensor to state machine
	coin_sensor.sig_dime.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_D));
	coin_sensor.sig_nickel.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_N));

	// connect keypad to state machine
	keypad.sig_coke.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_coke));
	keypad.sig_diet.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_diet));
	keypad.sig_zero.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_zero));
	keypad.sig_cancel.connect(std::bind(&sc::dispatch, &sc, &sc::state::event_cancel));

	// connect dispenser to state machine
	m.sig_dispense_coke.connect(std::bind(&dispenser::coke, &dispenser));
	m.sig_dispense_diet.connect(std::bind(&dispenser::diet, &dispenser));
	m.sig_dispense_zero.connect(std::bind(&dispenser::zero, &dispenser));

	// connect display to state machine
	m.sig_insert_coins.connect(std::bind(&display::insert, &display, _1));

	// connect coin refund to state machine
	m.sig_refund_dime.connect(std::bind(&coin_refund::dime, &coin_refund));
	m.sig_refund_nickel.connect(std::bind(&coin_refund::nickel, &coin_refund));

	std::cout << "input:  1 = coke button" << std::endl;
	std::cout << "        2 = zero button" << std::endl;
	std::cout << "        3 = light button" << std::endl;
	std::cout << "        c = cancel button" << std::endl;
	std::cout << "        n = insert nickel" << std::endl;
	std::cout << "        d = insert dime" << std::endl;
	std::cout << "        q = quit" << std::endl;

	sc.init();
	input.run();

	return 0;
}

