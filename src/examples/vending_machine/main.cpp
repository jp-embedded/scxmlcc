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

int main()
{
	sc_vending_machine sc;
	input input;
	coin_sensor coin_sensor;
	keypad keypad;

	functor<sc_vending_machine::event> dispatch(&sc, &sc_vending_machine::dispatch);

	input.sig_key.push_back(functor<const char>(&coin_sensor, &coin_sensor::simulate_input));
	input.sig_key.push_back(functor<const char>(&keypad, &keypad::simulate_input));
	coin_sensor.sig_dime.push_back(bind(dispatch, &sc_vending_machine::state::event_D));
	coin_sensor.sig_nickel.push_back(bind(dispatch, &sc_vending_machine::state::event_N));
	keypad.sig_coke.push_back(bind(dispatch, &sc_vending_machine::state::event_coke));
	keypad.sig_diet.push_back(bind(dispatch, &sc_vending_machine::state::event_diet));
//todo	keypad.sig_zero.push_back(bind(dispatch, &sc_vending_machine::state::event_zero));

	input.run();

	return 0;
}

