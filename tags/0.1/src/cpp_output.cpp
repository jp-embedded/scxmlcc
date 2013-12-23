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

#include "cpp_output.h"
#include "version.h"
#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <set>

using namespace std;

//todo set private/public/protected 

string cpp_output::classname()		{ return "sc_" + sc.sc().name; }
string cpp_output::state_t()		{ return "state"; }
string cpp_output::state_composite_t()	{ return "composite"; }
string cpp_output::state_actions_t()	{ return "state_actions"; }
	
void cpp_output::gen_transition_base()
{
	out << tab << "class no_state {};" << endl; 
	out << tab << "enum transition_type { external, internal };" << endl; 
	out << endl;
	
	out << tab << "template<event E, class S, class D = no_state> class transition_actions" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "protected:" << endl;
	out << tab << tab << "void enter(data_model&) {} // default enter action" << endl;
	out << tab << tab << "bool condition(data_model&) { return true; } // default condition action" << endl;
	out << tab << "};" << endl;
	out << endl;

	// external transition
	out << tab << "// external transition" << endl;
	out << tab << "template<event E, class S, class D = no_state, transition_type T = external> class transition : public transition_actions<E, S, D>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	// exit/enter is called here without parameter, which forces the action to always exit/enter at least current state
	out << tab << tab << "D* operator ()(S *s, D &d, data_model &m)" << endl;
	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(m)) return 0;" << endl;
       	out << tab << tab << tab << "s->exit(m, typeid(S));" << endl;
       	out << tab << tab << tab << "s->template exit<D>(m);" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, D>::enter(m);" << endl;
       	out << tab << tab << tab << "d.template enter<S>(m);" << endl;
       	out << tab << tab << tab << "return &d;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;
	
	// todo veryfy how these work at down transition
	out << tab << "// internal transition" << endl;
	out << tab << "template<event E, class S, class D> class transition<E, S, D, internal> : public transition_actions<E, S, D>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << "D* operator ()(S *s, D &d, data_model &m)" << endl; 
	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(m)) return 0;" << endl;
       	out << tab << tab << tab << "s->exit(m, typeid(S));" << endl;
       	out << tab << tab << tab << "s->template exit<D>(m, (D*)0);" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, D>::enter(m);" << endl;
       	out << tab << tab << tab << "d.template enter<S>(m, (S*)0);" << endl;
       	out << tab << tab << tab << "return &d;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;
	
	// transition without target
	out << tab << "// transition with no target" << endl;
	out << tab << "template<event E, class S> class transition<E, S, no_state> : public transition_actions<E, S, no_state>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << "S* operator ()(S *s, data_model &m)" << endl;
       	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, no_state>::condition(m)) return 0;" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, no_state>::enter(m);" << endl;
       	out << tab << tab << tab << "return s;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_state_actions_base()
{
	out << tab << "template<class C> class " << state_actions_t() << endl;
	out << tab << "{" << endl;
	// enter/exit
    	out << tab << tab << "protected:" << endl;
    	out << tab << tab << "void enter(data_model&) {} // default enter action" << endl;
	out << tab << tab << "void exit(data_model&) {} // default exit action" << endl;
	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_state_composite_base()
{
	out << tab << "template<class C, class P> class " << state_composite_t() << " : public P, public state_actions<C>" << endl;
	out << tab << "{" << endl;
	
	out << tab << tab << "virtual " << state_t() << "* initial(" << classname() << "&) { return 0; }" << endl;

	// lca calculation
	out << tab << tab << "public:" << endl;
	out << tab << tab << "// LCA calculation" << endl;
	out << tab << tab << "template<class T> void enter(data_model&, " << state_composite_t() << "*) {}" << endl;
	out << tab << tab << "template<class T> void enter(data_model &m, ...) {  P::template enter<T>(m, (T*)0); state_actions<C>::enter(m); }" << endl;
	out << tab << tab << "template<class T> void exit(data_model&, " << state_composite_t() << "*) {}" << endl;
	out << tab << tab << "template<class T> void exit(data_model &m, ...) {  state_actions<C>::exit(m); P::template exit<T>(m, (T*)0); }" << endl;
	out << tab << tab << "virtual void exit(data_model &m, const std::type_info &sti) {  if(typeid(C) == sti) return; state_actions<C>::exit(m); P::exit(m, sti); }" << endl;
	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_model_base()
{
	out << tab << "struct user_model;" << endl;
	out << tab << "typedef std::auto_ptr<user_model> user_model_p;" << endl;
	out << tab << "struct data_model" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "user_model_p user;" << endl;
	out << tab << "} model;" << endl;
	out << endl;
}

void cpp_output::gen_state_base()
{
	const scxml_parser::state_list &states = sc.sc().states;

	// state actions class
	out << tab << "class " << state_t() << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;

	// events
	// pass through set, to sort out dublicates
	set<string> event_set;
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		for (scxml_parser::transition_list::const_iterator i = s->get()->transitions.begin(); i != s->get()->transitions.end(); ++i) {
			if(i->get()->event) event_set.insert(*i->get()->event);
		}
	}
	for (set<string>::const_iterator i = event_set.begin(); i != event_set.end(); ++i) {
		out << tab << tab << "virtual " << state_t() << "* event_" << *i << "(" << classname() << "&) { return 0; }" << endl;
	}
	out << tab << tab << "virtual " << state_t() << "* unconditional(" << classname() << "&) { return 0; }" << endl;
	out << tab << tab << "virtual " << state_t() << "* initial(" << classname() << "&) { return 0; }" << endl;

	out << endl;

	out << tab << tab << "template<class T> void enter(data_model&, ...) {}" << endl;
	out << tab << tab << "template<class T> void exit(data_model&, ...) {}" << endl;
	out << tab << tab << "virtual void exit(data_model&, const std::type_info&) {}" << endl;

	out << tab << tab << "virtual ~" << state_t() << "() {}" << endl;

	out << tab << "};" << endl;
	out << endl;

	out << tab << "typedef " << state_t() << "* (" << state_t() << "::*event)(" << classname() << "&);" << endl;
	out << endl;
}

void cpp_output::gen_state(const scxml_parser::state &state)
{
	string parent, prefix;
	if(state.type && *state.type == "inter") prefix = "inter";
	if(state.parent) parent = "state_" + state.parent->id;
	else parent = state_t();
	string state_classname = prefix + "state_" + state.id;
	out << tab << "class " << state_classname << " : public " << state_composite_t() << "<" << state_classname << ", " << parent << '>' << endl;
	out << tab << "{" << endl;

	if(state.initial) {
		string target = "sc.m_state_" + *state.initial;
		string target_classname = "state_" + *state.initial;
		out << tab << tab << state_t() << "* " << "initial" << "(" << classname() << " &sc) { return transition<&state::initial, " << state_classname << ", " << target_classname << ", internal>()(this, " << target << ", sc.model); }" << endl;
	}

	//events
	for (scxml_parser::transition_list::const_iterator t = state.transitions.begin(); t != state.transitions.end(); ++t) {
		string target;
		string target_classname = state_classname;
		string event;

		if(t->get()->target) {
			target = "sc.m_state_" + *t->get()->target;
			target_classname = "state_" + *t->get()->target;
		}
		if(t->get()->event) {
			event = "event_" + *t->get()->event;
		}
		else {
			event = "unconditional";
		}
		if(target.size()) {
			// normal transition
			out << tab << tab << state_t() << "* " << event << "(" << classname() << " &sc) { return transition<&state::" << event << ", " << state_classname << ", " << target_classname << ">()(this, " << target << ", sc.model); }" << endl;
		}
		else {
			// transition without target
			out << tab << tab << state_t() << "* " << event << "(" << classname() << " &sc) { return transition<&state::" << event << ", " << state_classname << ">()(this, sc.model); }" << endl;
		}
	}

	out << tab << "} m_" << prefix << "state_" << state.id << ";" << endl;
	out << endl;
}

void cpp_output::gen_sc()
{
	const scxml_parser::state_list &states = sc.sc().states;

	out << "class " << classname() << endl;
	out << "{" << endl;
	out << tab << "public:" << endl;
	out << endl;

	gen_model_base();
	gen_state_base();
	gen_state_actions_base();
	gen_state_composite_base();
	gen_transition_base();

	// new_state
	out << tab << "void new_state(state *next_state)" << endl;
	// todo optimize this away if no unconditional or no event queued.
	// todo unconditional is called on every event, so it might be cheaper to put it in the event queue
	out << tab << "{" << endl;
	out << tab << tab << "while (next_state) {" << endl;
	out << tab << tab << tab << "cur_state = next_state;" << endl;
	out << tab << tab << tab << "if ((next_state = cur_state->initial(*this)));" << endl;
	out << tab << tab << tab << "else if ((next_state = cur_state->unconditional(*this)));" << endl;
	out << tab << tab << tab << "else if (event_queue.size()) { next_state = (cur_state->*event_queue.front())(*this); event_queue.pop(); }" << endl;
	out << tab << tab << tab << "else break;" << endl;
	out << tab << tab << "}" << endl;
	out << tab << "}" << endl;

	// dispatch
	out << tab << "void dispatch(event e) { new_state((cur_state->*e)(*this)); }" << endl;
	out << endl;

	// constructor
	out << tab << classname() << "(user_model_p user = user_model_p()) : cur_state(&m_state_" << sc.sc().initial << ") {" << endl;
	out << tab << tab << "model.user = user;" << endl;
	out << tab << tab << "m_state_" << sc.sc().initial << ".enter<" << state_t() << ">(model);" << endl;
	out << tab << tab << "new_state(cur_state);" << endl;
	out << tab << "}" << endl;
	out << endl;
	
	// states
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		gen_state(*s->get());
	}

	out << tab << state_t() << " *cur_state;" << endl;
	out << tab << "std::queue<event> event_queue;" << endl;

	out << "};" << endl;
	out << endl;
}

void cpp_output::trim()
{
	const scxml_parser::state_list &states = sc.sc().states;

	// replace '.' with '_' in event names
	// todo: see '3.12.1 event descriptors how to handle this
	set<string> event_set;
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		for (scxml_parser::transition_list::const_iterator i = s->get()->transitions.begin(); i != s->get()->transitions.end(); ++i) {
			if(i->get()->event) {
				if(i->get()->event->find('.') != string::npos) cerr << "warning: event tokens not supported" << endl;
				replace(i->get()->event->begin(), i->get()->event->end(), '.', '_');
			}
		}
	}
}

void cpp_output::gen()
{
	// include guard
	out << "// This file is automatically generated by scxmlcc (version " << version() << ")" << endl;
	out << "// For more information, see http://scxmlcc.org" << endl;
	out << endl;
	out << "#ifndef __SC_" << boost::to_upper_copy(sc.sc().name) << endl;
	out << "#define __SC_" << boost::to_upper_copy(sc.sc().name) << endl;
	out << endl;
	out << "#include <typeinfo>" << endl;
	out << "#include <queue>" << endl;
	out << "#include <memory>" << endl;
	out << endl;

	trim();
	gen_sc();

	// end of include guard
 	out << "#endif" << endl;

}

