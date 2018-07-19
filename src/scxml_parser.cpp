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

#include "scxml_parser.h"
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <iostream>

using namespace boost::property_tree;
using namespace std;

// allow appending vector to vector
template <typename T> void operator+=(std::vector<T> &v1, const std::vector<T> &v2) {
    v1.insert(v1.end(), v2.begin(), v2.end());
}

void scxml_parser::parse_scxml(const ptree &pt)
{
	try {
		using namespace boost::algorithm;
		const ptree &xmlattr = pt.get_child("<xmlattr>");
		boost::optional<string> initial(xmlattr.get_optional<string>("initial"));
		if(initial) split(m_scxml.initial.target, *initial, is_any_of(" "), token_compress_on);
		m_scxml.name = xmlattr.get<string>("name", m_scxml.name);

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (it->first == "state") parse_state(it->second, boost::shared_ptr<state>());
			else if (it->first == "history") parse_state(it->second, boost::shared_ptr<state>());
			else if (it->first == "final") parse_final(it->second, boost::shared_ptr<state>());
			else if (it->first == "parallel") parse_parallel(it->second, boost::shared_ptr<state>());
			else if (it->first == "initial") m_scxml.initial = parse_initial(it->second);
			else if (it->first == "datamodel") m_scxml.datamodel = parse_datamodel(it->second);
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <scxml>" << endl;
		}
		if(m_scxml.initial.target.size() > 1) parallel_target_sizes.insert(m_scxml.initial.target.size());

		// if initial state is not set, use first state in document order
		// also use first state for parallel states. Other children are entered implicit
		if(m_scxml.initial.target.empty()) {
			if(m_scxml.states.size()) {
				m_scxml.initial.target.push_back((*m_scxml.states.begin())->id);
			}
			else {
				cerr << "error: " << __FUNCTION__ << ": could not set initial state" << endl;
				exit(1);
			}
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
}

void scxml_parser::parse_parallel(const ptree &pt, const boost::shared_ptr<state> &parent)
{
	try {
		using_parallel = true;
		const ptree &xmlattr = pt.get_child("<xmlattr>");
		boost::shared_ptr<state> st = boost::make_shared<state>();
		st->id = xmlattr.get<string>("id");
		if(parent) {
			using_compound = true;
			st->parent = parent;
		}
		st->type.reset("parallel");
		m_scxml.states.push_back(st);

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (it->first == "state") parse_state(it->second, st);
			else if (it->first == "history") parse_state(it->second, st);
			else if (it->first == "parallel") parse_parallel(it->second, st);
			else if (it->first == "transition") st->transitions.push_back(parse_transition(it->second));
			else if (it->first == "onentry") st->entry_actions += parse_entry(it->second);
			else if (it->first == "onexit") st->exit_actions += parse_entry(it->second);
			else if (it->first == "initial") st->initial = parse_initial(it->second);
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <parallel>" << endl;
		}

		// if parent initial state is not set, use first state in document order
		// if parent is parallel, also use first state. Other children are entered implicit
		if(parent && parent->initial.target.empty()) {
			parent->initial.target.push_back(st->id);
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
}

boost::shared_ptr<scxml_parser::data> scxml_parser::parse_data(const ptree &pt)
{
	boost::shared_ptr<scxml_parser::data> data = boost::make_shared<scxml_parser::data>();
	
	try {
		using namespace boost::algorithm;
		const ptree &xmlattr = pt.get_child("<xmlattr>");
		data->id = xmlattr.get<string>("id");
		data->expr = xmlattr.get_optional<string>("expr");
	
		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <data>" << endl;
		}

	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
	return data;
}

scxml_parser::data_list scxml_parser::parse_datamodel(const ptree &pt)
{
	scxml_parser::data_list datamodel;
	try {

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "data") datamodel.push_back(parse_data(it->second));
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <datamodel>" << endl;
		}

	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
	return datamodel;
}

scxml_parser::transition scxml_parser::parse_initial(const ptree &pt)
{
	scxml_parser::transition initial;
	initial.event.push_back("initial");

	try {

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "transition") initial = *parse_transition(it->second);
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <initial>" << endl;
		}

	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}

	return initial;
}

void scxml_parser::parse_state(const ptree &pt, const boost::shared_ptr<state> &parent)
{
	try {
		using namespace boost::algorithm;
		const ptree &xmlattr = pt.get_child("<xmlattr>");
		boost::shared_ptr<state> st = boost::make_shared<state>();
		st->id = xmlattr.get<string>("id");
		if(parent) {
			using_compound = true;
			st->parent = parent;
		}
		boost::optional<string> initial(xmlattr.get_optional<string>("initial"));
		if(initial) split(st->initial.target, *initial, is_any_of(" "), token_compress_on);
		m_scxml.states.push_back(st);

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (it->first == "state") parse_state(it->second, st);
			else if (it->first == "history") parse_state(it->second, st);
			else if (it->first == "final") parse_final(it->second, st);
			else if (it->first == "parallel") parse_parallel(it->second, st);
			else if (it->first == "transition") st->transitions.push_back(parse_transition(it->second));
			else if (it->first == "onentry") st->entry_actions += parse_entry(it->second);
			else if (it->first == "onexit") st->exit_actions += parse_entry(it->second);
			else if (it->first == "initial") st->initial = parse_initial(it->second);
			else if (it->first == "datamodel") { scxml_parser::data_list m = parse_datamodel(it->second); m_scxml.datamodel.insert(m_scxml.datamodel.end(), m.begin(), m.end()); }
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <state>" << endl;
		}

		// if parent initial state is not set, use first state in document order
		// if parent is parallel, also use first state. Other children are entered implicit
		if(parent && parent->initial.target.empty()) {
			parent->initial.target.push_back(st->id);
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
}

void scxml_parser::parse_final(const ptree &pt, const boost::shared_ptr<state> &parent)
{
	try {
		using_final = true;
		using namespace boost::algorithm;
		const ptree &xmlattr = pt.get_child("<xmlattr>");
		boost::shared_ptr<state> st = boost::make_shared<state>();
		st->id = xmlattr.get<string>("id");
		if(parent) {
			using_compound = true;
			st->parent = parent;
		}
		st->type.reset("final");
		m_scxml.states.push_back(st);
		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (it->first == "onentry") st->entry_actions += parse_entry(it->second);
			else if (it->first == "onexit") st->exit_actions += parse_entry(it->second);
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <state>" << endl;
		}

		// if parent initial state is not set, use first state in document order
		// if parent is parallel, also use first state. Other children are entered implicit
		if(parent && parent->initial.target.empty()) {
			parent->initial.target.push_back(st->id);
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
}

scxml_parser::plist<scxml_parser::action> scxml_parser::parse_entry(const ptree &pt)
{
	plist<action> l_ac;
	try {
		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "script") l_ac.push_back(parse_script(it->second));
			else if (it->first == "log") l_ac.push_back(parse_log(it->second));
			else if (it->first == "raise") l_ac.push_back(parse_raise(it->second));
			else if (it->first == "assign") l_ac.push_back(parse_assign(it->second));
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <onentry> or <onexit>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
	return l_ac;
}

boost::shared_ptr<scxml_parser::action> scxml_parser::parse_raise(const ptree &pt)
{
	boost::shared_ptr<action> ac = boost::make_shared<action>();
	try {
		const ptree &xmlattr = pt.get_child("<xmlattr>");

		const string event = xmlattr.get<string>("event");

		ac->type = "raise";
		ac->attr["event"] = event;

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <raise>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}

	using_event_queue = true;
	return ac;
}

boost::shared_ptr<scxml_parser::action> scxml_parser::parse_log(const ptree &pt)
{
	boost::shared_ptr<action> ac = boost::make_shared<action>();
	try {
		const ptree &xmlattr = pt.get_child("<xmlattr>");

		boost::optional<string> label(xmlattr.get_optional<string>("label"));
		const string expr = xmlattr.get<string>("expr");

		ac->type = "log";
		if(label) ac->attr["label"] = *label;
		ac->attr["expr"] = expr;

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <log>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}

	using_log = true;
	return ac;
}

boost::shared_ptr<scxml_parser::action> scxml_parser::parse_assign(const ptree &pt)
{
	boost::shared_ptr<action> ac = boost::make_shared<action>();
	try {
		const ptree &xmlattr = pt.get_child("<xmlattr>");

		const string location = xmlattr.get<string>("location");
		boost::optional<string> expr(xmlattr.get_optional<string>("expr"));

		ac->type = "assign";
		ac->attr["location"] = location;
		if(expr) ac->attr["expr"] = *expr;

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <assign>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}

	return ac;
}

boost::shared_ptr<scxml_parser::action> scxml_parser::parse_script(const ptree &pt)
{
	boost::shared_ptr<action> ac = boost::make_shared<action>();
	try {
		ac->type = "script";
		ac->attr["expr"] = pt.get_value<string>();

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <script>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
	return ac;
}

boost::shared_ptr<scxml_parser::transition> scxml_parser::parse_transition(const ptree &pt)
{
	boost::shared_ptr<transition> tr = boost::make_shared<transition>();
	try {
		boost::optional<const ptree &> xmlattr(pt.get_child_optional("<xmlattr>"));
		if (xmlattr) {
			using namespace boost::algorithm;
			boost::optional<string> target(xmlattr->get_optional<string>("target"));
			if(target) split(tr->target, *target, is_any_of(" "), token_compress_on);
			if(tr->target.size() > 1) parallel_target_sizes.insert(tr->target.size());
			boost::optional<string> event = xmlattr->get_optional<string>("event");
			if(event) split(tr->event, *event, is_any_of(" "), token_compress_on);
			tr->type = xmlattr->get_optional<string>("type");
			tr->condition = xmlattr->get_optional<string>("cond");
		}

		for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
			if (it->first == "<xmlcomment>") ; // ignore comments
			else if (it->first == "<xmlattr>") ; // ignore, parsed above
			else if (it->first == "script") tr->actions.push_back(parse_script(it->second));
			else if (it->first == "log") tr->actions.push_back(parse_log(it->second));
			else if (it->first == "raise") tr->actions.push_back(parse_raise(it->second));
			else if (it->first == "assign") tr->actions.push_back(parse_assign(it->second));
			else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
			else cerr << "warning: unknown item '" << it->first << "' in <transition>" << endl;
		}
	}
	catch (ptree_error e) {
		cerr << "error: " << __FUNCTION__ << ": " << e.what() << endl;
		exit(1);
	}
        if (!tr->target.size()) using_transition_no_target = true;
	return tr;
}

void scxml_parser::parse(const ptree &pt)
{
	for (ptree::const_iterator it = pt.begin(); it != pt.end(); ++it) {
		if (it->first == "<xmlcomment>") ; // ignore comments
		else if (it->first == "scxml") parse_scxml(it->second);
		else if (std::regex_match(it->first, ignore_elements_re)) ; // ignore
		else cerr << "warning: unknown item '" << it->first << "'" << endl;
	}
}

scxml_parser::scxml_parser(const char *name, const std::string& ignore_unknown, const ptree &pt) : using_parallel(false), using_final(false), using_event_queue(false), using_log(false), using_compound(false), using_transition_no_target(false), ignore_elements_re(ignore_unknown)
{
	m_scxml.name = name;
	parse(pt);
}

