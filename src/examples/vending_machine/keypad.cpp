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

#include "keypad.h"
#include <iostream>

void keypad::simulate_input(const char c)
{
	switch (c) {
		case '1': std::cout << __PRETTY_FUNCTION__ << ": coke" << std::endl; sig_coke(); break;
		case '2': std::cout << __PRETTY_FUNCTION__ << ": light" << std::endl; sig_light(); break;
		case '3': std::cout << __PRETTY_FUNCTION__ << ": zero" << std::endl; sig_zero(); break;
	}
}

