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

#ifndef __CPP_OUTPUT
#define __CPP_OUTPUT

//todo store scxml in internal format. to be able to parse other input formats
#include "scxml_parser.h"

#include <ostream>

class cpp_output {

	std::ostream &out;
	const scxml_parser &sc;
	std::string tab;

	public:
	cpp_output(std::ostream &ofs, const scxml_parser &sc) : out(ofs), sc(sc), tab("\t") {};

	void gen();

	protected:
	std::string classname();
	std::string state_t();
	std::string state_actions_t();
	std::string state_composite_t();
	void gen_model_base();
	void gen_state_base();
	void gen_state_composite_base();
	void gen_state_actions_base();
	void gen_transition_base();
	void gen_state(const scxml_parser::state &state);
	void gen_sc();
	void trim();


};

#endif

