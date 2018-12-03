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
#include "options.h"

#include <ostream>

class cpp_output {

	std::ostream &out;
	const scxml_parser &sc;
	const options &opt;
	std::string tab;
	const std::string any_ns; // namespace for any in generated code
	const std::string optional_ns;

	public:
	cpp_output(std::ostream &ofs, const scxml_parser &sc, const options &op)
	: out(ofs)
	, sc(sc)
	, opt(op)
	, tab("\t")
	, any_ns(op.cpp14 ? "boost::" : "std::")
	, optional_ns(any_ns)
	{};

	void gen();

	protected:
	std::string classname();
	std::string state_t();
	std::string state_actions_t();
	std::string state_composite_t();
	std::string state_final_t();
	std::string state_parallel_t();
	void gen_model_decl();
	void gen_model_base_data();
	void gen_model_base_finals();
	void gen_model_base();
	void gen_state_base();
	void gen_state_composite_base();
	void gen_state_final_base();
	void gen_state_parallel_base();
	void gen_state_actions_base();
	void gen_transition_base();
	void gen_state(const scxml_parser::state &state);
	void gen_with_begin(bool cond);
	void gen_with_end(bool cond);
	void gen_action_part_log(scxml_parser::action &a);
	void gen_action_part_raise(scxml_parser::action &a);
	void gen_action_part_assign(scxml_parser::action &a);
	void gen_action_part_script(scxml_parser::action &a);
	void gen_action_part(scxml_parser::action &a);
	void gen_condition_part(std::string cond);
	void gen_condition_part_In(std::string cond);
	void gen_actions();
	void gen_sc();
	void trim();

	private:
	scxml_parser::state_list children(const scxml_parser::state &state);
	scxml_parser::state_list states(const std::string &type);
	std::set<std::string> get_event_names() const;
};

#endif

