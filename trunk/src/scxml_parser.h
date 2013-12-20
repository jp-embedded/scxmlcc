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

#ifndef __SCXML_PARSER
#define __SCXML_PARSER

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <string>
#include <list>

class scxml_parser
{
	public:
		template <class T> class list : public std::vector<boost::shared_ptr<T> > {};

		struct action {
			std::string expr;
		};

		struct transition {
			boost::optional<std::string> target;
			boost::optional<std::string> event;
			list<action> actions;
		};
		typedef list<transition> transition_list;

		struct state {
			std::string id;
			boost::shared_ptr<state> parent;
			boost::optional<std::string> initial, type;
			transition_list transitions;
			list<action> entry_actions;
			list<action> exit_actions;
		};
		typedef list<state> state_list;

		struct scxml {
			std::string name;
			std::string initial;
			state_list states;
		};

		scxml_parser(const char *name, const boost::property_tree::ptree &pt);

		const scxml& sc() const { return m_scxml; }

	protected:
		scxml m_scxml;

		void parse_scxml(const boost::property_tree::ptree &pt);
		void parse(const boost::property_tree::ptree &pt);
		void parse_state(const boost::property_tree::ptree &pt, const boost::shared_ptr<state> &parent);
		boost::shared_ptr<transition> parse_transition(const boost::property_tree::ptree &pt);
		boost::shared_ptr<action> parse_script(const boost::property_tree::ptree &pt);
		boost::shared_ptr<action> parse_log(const boost::property_tree::ptree &pt);
		list<action> parse_entry(const boost::property_tree::ptree &pt);

};


#endif

