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

#ifndef __MACHINE
#define __MACHINE

#include "vending_machine.h"
#include "signal.h"

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

const int nickel = 5;
const int dime = 10;
const int price_coke = 15;
const int price_zero = 15;
const int price_diet = 15;

template<> void sc::transition_actions<&sc::state::event_N, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
{ 
	// nickel inserted
	m.user->credit += nickel; 
}

template<> void sc::transition_actions<&sc::state::event_D, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
{ 
	// dime inserted
	m.user->credit += dime; 
}

template<> void sc::state_actions<sc::state_collect_coins>::enter(sc::data_model &m) 		
{ 
	// ready for coin insert
	m.user->sig_insert_coins(m.user->credit); 
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_d, sc::state_return_n>::condition(sc::data_model &m)
{
	// no more dimes te return?
	return m.user->credit < dime;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_d>::condition(sc::data_model &m)
{
	// more dimes to return?
	return m.user->credit >= dime;
}

template<> void sc::transition_actions<&sc::state::unconditional, sc::state_return_d>::enter(sc::data_model &m)
{
	// refund dime
	m.user->sig_refund_dime();
	m.user->credit -= dime;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_n, sc::state_return_done>::condition(sc::data_model &m)
{
	// no more nickels to return?
	return m.user->credit < nickel;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_return_n>::condition(sc::data_model &m)
{
	// more nickels to return?
	return m.user->credit >= nickel;
}

template<> void sc::transition_actions<&sc::state::unconditional, sc::state_return_n>::enter(sc::data_model &m)
{
	// refund nickel
	m.user->sig_refund_nickel();
	m.user->credit -= nickel;
}

template<> void sc::state_actions<sc::state_return_done>::enter(sc::data_model &m)
{
	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> bool sc::transition_actions<&sc::state::event_zero, sc::state_collect_coins, sc::state_dispense_zero>::condition(sc::data_model &m)
{
	// enough credit for zero?
	return m.user->credit >= price_zero;
}

template<> bool sc::transition_actions<&sc::state::event_coke, sc::state_collect_coins, sc::state_dispense_coke>::condition(sc::data_model &m)
{
	// enough credit for coke?
	return m.user->credit >= price_coke;
}

template<> bool sc::transition_actions<&sc::state::event_diet, sc::state_collect_coins, sc::state_dispense_diet>::condition(sc::data_model &m)
{
	// enough credit for diet?
	return m.user->credit >= price_diet;
}

template<> void sc::state_actions<sc::state_dispense_coke>::enter(sc::data_model &m)
{
	// dispense coke
	m.user->sig_dispense_coke();
	m.user->credit -= price_coke;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> void sc::state_actions<sc::state_dispense_diet>::enter(sc::data_model &m)
{
	// dispense diet
	m.user->sig_dispense_diet();
	m.user->credit -= price_diet;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

template<> void sc::state_actions<sc::state_dispense_zero>::enter(sc::data_model &m)
{
	// dispense zero
	m.user->sig_dispense_zero();
	m.user->credit -= price_zero;

	//todo this should be added automatically, when in final state
	m.event_queue.push(&sc::state::event_done);
}

#endif

