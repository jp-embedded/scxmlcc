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

#include "coin_sensor.h"
#include <iostream>

void coin_sensor::simulate_input(const char c)
{
	switch (c) {
		case 'n': std::cout << __PRETTY_FUNCTION__ << ": nickel" << std::endl; sig_nickel(); break;
		case 'd': std::cout << __PRETTY_FUNCTION__ << ": dime" << std::endl; sig_dime(); break;
	}
}

