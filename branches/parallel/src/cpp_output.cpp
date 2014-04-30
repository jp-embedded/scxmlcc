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
string cpp_output::state_parallel_t()	{ return "parallel"; }
string cpp_output::state_actions_t()	{ return "state_actions"; }
	
void cpp_output::gen_transition_base()
{
	out << tab << "class no_state {};" << endl; 
	out << tab << "enum transition_type { external, internal };" << endl; 
	out << endl;

	const int max_targets = sc.parallel_target_sizes.size() ? *sc.parallel_target_sizes.rbegin() : 1;
	out << tab << "template<event E, class S";
	for(int i = 0; i < max_targets; ++i) out << ", class D" << i << " = no_state";
	out << "> class transition_actions" << endl;
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
	out << tab << tab << "state* operator ()(S *s, D &d, " << classname() << " &sc)" << endl;
	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(sc.model)) return 0;" << endl;
	if(sc.using_parallel) out << tab << tab << tab << "s->exit_parallel(sc, s, &d);" << endl;
       	out << tab << tab << tab << "s->exit(sc.model, typeid(S));" << endl;
       	out << tab << tab << tab << "s->template exit<D>(sc.model);" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, D>::enter(sc.model);" << endl;
       	out << tab << tab << tab << "d.template enter<S>(sc.model);" << endl;
       	if(sc.using_parallel) out << tab << tab << tab << "return d.template enter_parallel<S>(sc, &d, s);" << endl;
	else out << tab << tab << tab << "return &d;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;
	
	// todo veryfy how these work at down transition
	out << tab << "// internal transition" << endl;
	out << tab << "template<event E, class S, class D> class transition<E, S, D, internal> : public transition_actions<E, S, D>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << "state* operator ()(S *s, D &d, " << classname() << " &sc)" << endl; 
	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(sc.model)) return 0;" << endl;
	if(sc.using_parallel) out << tab << tab << tab << "s->exit_parallel(sc, s, &d);" << endl;
       	out << tab << tab << tab << "s->exit(sc.model, typeid(S));" << endl;
       	out << tab << tab << tab << "s->template exit<D>(sc.model, (D*)0);" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, D>::enter(sc.model);" << endl;
       	out << tab << tab << tab << "d.template enter<S>(sc.model, (S*)0);" << endl;
       	if(sc.using_parallel) out << tab << tab << tab << "return d.template enter_parallel<S>(sc, &d, s);" << endl;
	else out << tab << tab << tab << "return &d;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;
	
	// transition without target
	out << tab << "// transition with no target" << endl;
	out << tab << "template<event E, class S> class transition<E, S, no_state> : public transition_actions<E, S, no_state>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << "S* operator ()(S *s, " << classname() << " &sc)" << endl;
       	out << tab << tab << "{" << endl;
       	out << tab << tab << tab << "if(!transition_actions<E, S, no_state>::condition(sc.model)) return 0;" << endl;
       	out << tab << tab << tab << "transition_actions<E, S, no_state>::enter(sc.model);" << endl;
       	out << tab << tab << tab << "return s;" << endl;
       	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;

	for (set<int>::const_iterator i = sc.parallel_target_sizes.begin(); i != sc.parallel_target_sizes.end(); ++i) {
		const int sz = *i;

		out << tab << "template<event E, class S";
		for(int i = 0; i < sz; ++i) out << ", class D" << i;
		out << ", transition_type T = external> class transition" << sz << ';' << endl;
		
		out << tab << "template<event E, class S";
		for(int i = 0; i < sz; ++i) out << ", class D" << i;
		out << "> class transition" << sz << "<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ", internal> : public transition_actions<E, S, D0, D1>" << endl;

		out << tab << '{' << endl;
		out << tab << tab << "public:" << endl;

		//todo: for now, all targets must have same parallel parent
		out << tab << tab << "state* operator ()(S *s";
		for(int i = 0; i < sz; ++i) out << ", D" << i << " &d" << i;
		out << ", sc_test576 &sc)" << endl;

		out << tab << tab << '{' << endl;

		out << tab << tab << tab << "if(!transition_actions<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ">::condition(sc.model)) return 0;" << endl;

		out << tab << tab << tab << "s->exit_parallel(sc, s, &d0);" << endl;
		out << tab << tab << tab << "s->exit(sc.model, typeid(S));" << endl;
		out << tab << tab << tab << "s->template exit<D0>(sc.model, (D0*)0);" << endl;

		out << tab << tab << tab << "transition_actions<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ">::enter(sc.model);" << endl;

		out << tab << tab << tab << "d0.template enter<S>(sc.model, (S*)0);" << endl;
		out << tab << tab << tab << "return d0.template enter_parallel<S>(sc, &d0, s";
		for(int i = 1; i < sz; ++i) out << ", d" << i;
		out << ");" << endl;

		out << tab << tab << '}' << endl;
		out << tab << "};" << endl;
		
		out << endl;
	}
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

void cpp_output::gen_state_parallel_base()
{
	if (!sc.using_parallel) return;

	if(sc.parallel_sizes.size() == 0) {
		//todo make composite, if children < 2
		cerr << "error: parallel state with < 2 states is currently not supported" << endl;
		exit(1);
	}

	const int min_c = *sc.parallel_sizes.begin();
	const int max_c = *sc.parallel_sizes.rbegin();
	if(min_c < 2) {
		//todo make composite, if children < 2
		cerr << "error: parallel state with < 2 states is currently not supported" << endl;
		exit(1);
	}

	//todo combine with no_class
	if(min_c < max_c) out << tab << "class no_class {};" << endl;

	for (set<int>::reverse_iterator i = sc.parallel_sizes.rbegin(); i != sc.parallel_sizes.rend(); ++i) {
		const int children = *i;
		out << tab << "template<class C, class P";
		for(int c = 0; c < children; ++c) {
			out << ", class C" << c;
			if ((children == max_c) && (c >= min_c)) out << " = no_class";
		}
		out << "> class " << state_parallel_t();
		if(children < max_c) {
			out << " <C, P";
			for (int c = 0; c < children; ++c) out << ", C" << c;
			out << '>';
		}
		out << " : public composite<C, P>" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "public:" << endl;
		for(int c = 0; c < children; ++c) {
			out << tab << tab << "template<class S> " << state_t() << "* enter_parallel(" << classname() << " &sc, C" << c << "*, C*) { return this; }" << endl;
			out << tab << tab << "template<class S> " << state_t() << "* enter_parallel(" << classname() << " &sc, C" << c << " *d, " << state_t() << "*)" << endl;
			out << tab << tab << '{' << endl;
			out << tab << tab << tab << "// parallel state entered with C" << c << " or child of as target" << endl;
			out << tab << tab << tab << "P::template enter_parallel<S>(sc, d, (S*)0);" << endl;
			for(int n = 0; n < children; ++n) {
				if (n == c) continue;
				out << tab << tab << tab << "sc.cur_state.push_back(d->init_child(sc, (C" << n << "*)0));" << endl;
			}
			out << tab << tab << tab << "return this;" << endl;
			out << tab << tab << "};" << endl;
			out << tab << tab << endl;

			//todo to continue
		}

		out << tab << tab << "template<class S> " << state_t() << "* enter_parallel(" << classname() << " &sc, C*, C*) { return this; }" << endl;
		out << tab << tab << "template<class S> " << state_t() << "* enter_parallel(" << classname() << " &sc, C *d, " << state_t() << "*)" << endl;
		out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// parallel state entered with parallel as target" << endl;
		out << tab << tab << tab << "P::template enter_parallel<S>(sc, d, (S*)0);" << endl;
		for(int n = 0; n < children - 1; ++n) {
			out << tab << tab << tab << "sc.cur_state.push_back(d->init_child(sc, (C" << n << "*)0));" << endl;
		}
		out << tab << tab << tab << "return d->init_child(sc, (C" << children - 1 << "*)0);" << endl;
		out << tab << tab << "};" << endl;
		out << endl;

		// handle transition with all children given
		// todo, only add if needed
		out << tab << tab << "template<class S";
		for(int c = 1; c < children; ++c) out << ", class D" << c;
		out << "> " << state_t() << "* enter_parallel(" << classname() << " &sc, C*, C*";
		for(int c = 1; c < children; ++c) out << ", D" << c << "&";
		out << ") { return this; }" << endl;
		out << tab << tab << "template<class S";
		for(int c = 1; c < children; ++c) out << ", class D" << c;
		out << "> " << state_t() << "* enter_parallel(" << classname() << " &sc, C *d, " << state_t() << '*';
		for(int c = 1; c < children; ++c) out << ", D" << c << " &d" << c;
		out << ")" << endl;
	        out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// handle transition with all children given" << endl;
		out << tab << tab << tab << "P::template enter_parallel<S>(sc, d, (S*)0);" << endl;
		for(int c = 1; c < children; ++c) out << tab << tab << tab << 'd' << c << ".template enter<C>(sc.model, (C*)0), sc.cur_state.push_back(&d" << c << ");" << endl;

		// todo call init_child where Cn is not a child of Dn or *d
		// if (false) sc.cur_state.push_back(d->init_child(sc, (C0*)0));
		// if (false) sc.cur_state.push_back(d->init_child(sc, (C1*)0));
		
	        out << tab << tab << tab << "return this;" << endl;
	        out << tab << tab << '}' << endl;
		out << endl;

		out << tab << tab << "bool parallel_parent(const std::type_info& pti) { return typeid(C) == pti; }" << endl;
		out << tab << tab << "void exit_parallel(" << classname() << " &sc, C*, C*) {}" << endl;
		out << tab << tab << "void exit_parallel(" << classname() << " &sc, C *s, state *d)" << endl;
		out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// parallel state exited from C or child" << endl;
		out << tab << tab << tab << "for(" << classname() << "::cur_state_l::iterator i = sc.cur_state.begin(); (i != sc.cur_state.end()) && *i; ++i) {" << endl;
		out << tab << tab << tab << tab << "if(this == *i) continue;" << endl;
		out << tab << tab << tab << tab << "if(!(*i)->parallel_parent(typeid(C))) continue;" << endl;
		out << tab << tab << tab << tab << "(*i)->exit(sc.model, typeid(C));" << endl;
		out << tab << tab << tab << tab << "*i = 0;" << endl;
		out << tab << tab << tab << '}' << endl;
		out << tab << tab << tab << "P::exit_parallel(sc, s, d);" << endl;
		out << tab << tab << '}' << endl;


		out << tab << "};" << endl;
		out << endl;
	}
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
	if(sc.using_parallel) {
 		out << tab << tab << "template<class S> " << state_t() << "* enter_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) { return this; }" << endl;
 		out << tab << tab << "virtual void exit_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) {}" << endl;
 		out << tab << tab << "virtual bool parallel_parent(const std::type_info&) { return false; }" << endl;
	}

	out << tab << tab << "virtual ~" << state_t() << "() {}" << endl;

	out << tab << "};" << endl;
	out << endl;

	if(sc.using_parallel) {
		out << tab << "typedef std::vector<" << state_t() << "*> cur_state_l;" << endl;
		out << tab << "cur_state_l cur_state;" << endl;
	}
	else {
		out << tab << state_t() << " *cur_state;" << endl;
	}
	out << tab << "typedef " << state_t() << "* (" << state_t() << "::*event)(" << classname() << "&);" << endl;
	out << endl;
}

scxml_parser::state_list cpp_output::children(const scxml_parser::state &state)
{
	scxml_parser::state_list states;
	for(scxml_parser::state_list::const_iterator i = sc.sc().states.begin(); i != sc.sc().states.end(); ++i) {
		if((*i)->parent && (*i)->parent->id == state.id) states.push_back(*i);
	}
	return states;
}

void cpp_output::gen_state(const scxml_parser::state &state)
{
	const bool parallel_state = state.type && *state.type == "parallel";
	string parent, prefix;
	if(state.type && *state.type == "inter") prefix = "inter";
	if(state.parent) parent = "state_" + state.parent->id;
	else parent = "scxml";
	string state_classname = prefix + "state_" + state.id;

        if(parallel_state) {
		scxml_parser::state_list states = children(state);
		for(scxml_parser::state_list::const_iterator i = states.begin(); i != states.end(); ++i) {
			out << tab << "class state_" << (*i)->id << ';' << endl;
		}
	}
	out << tab << "class " << state_classname << " : public ";
	if(parallel_state) out << state_parallel_t();
       	else out << state_composite_t();
        out << '<' << state_classname << ", " << parent;
        if(parallel_state) {
		scxml_parser::state_list states = children(state);
		for(scxml_parser::state_list::const_iterator i = states.begin(); i != states.end(); ++i) {
			out << ", state_" << (*i)->id;
		}
	}
	out << '>' << endl;

	out << tab << "{" << endl;

	if(state.initial.size()) {
		string target = "sc.m_state_" + state.initial.front();
		string target_classname = "state_" + state.initial.front();
		out << tab << tab << state_t() << "* " << "initial" << "(" << classname() << " &sc) { return transition<&state::initial, " << state_classname << ", " << target_classname << ", internal>()(this, " << target << ", sc); }" << endl;
	}

	//events
	for (scxml_parser::transition_list::const_iterator t = state.transitions.begin(); t != state.transitions.end(); ++t) {
		string target;
		string target_classname = state_classname;
		string event;

		if(t->get()->target.size()) {
			target = "sc.m_state_" + t->get()->target.front(); //todo handle multiple targets
			target_classname = "state_" + t->get()->target.front(); //todo handle multiple targets
		}
		if(t->get()->event) {
			event = "event_" + *t->get()->event;
		}
		else {
			event = "unconditional";
		}
		if(target.size()) {
			// normal transition
			out << tab << tab << state_t() << "* " << event << "(" << classname() << " &sc) { return transition<&state::" << event << ", " << state_classname << ", " << target_classname << ">()(this, " << target << ", sc); }" << endl;
		}
		else {
			// transition without target
			out << tab << tab << state_t() << "* " << event << "(" << classname() << " &sc) { return transition<&state::" << event << ", " << state_classname << ">()(this, sc); }" << endl;
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
	gen_state_parallel_base();
	gen_transition_base();

	// dispatch
	if(sc.using_parallel) {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "bool cont = false;" << endl;
		out << tab << tab << "state *next_state;" << endl;
		out << tab << tab << "for(unsigned i = 0; i < cur_state.size();) if(cur_state[i]) {" << endl;
		out << tab << tab << tab << "if ((next_state = (cur_state[i]->*e)(*this))) cur_state[i] = next_state, cont = true;" << endl;
		out << tab << tab << tab << "++i;" << endl;
		out << tab << tab << '}' << endl;
		out << tab << tab << "else cur_state.erase(cur_state.begin() + i);" << endl;
		out << tab << tab << "return cont;" << endl;
		out << tab << '}' << endl;
	}
	else {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "state *next_state;" << endl;
		out << tab << tab << "if ((next_state = (cur_state->*e)(*this))) cur_state = next_state;" << endl;
		out << tab << tab << "return next_state;";
		out << tab << '}' << endl;
	}
	out << endl;
	out << tab << "public: void dispatch(event e)" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "bool cont = dispatch_event(e);" << endl;
	out << tab << tab << "while (cont) {" << endl;
	out << tab << tab << tab << "if ((cont = dispatch_event(&state::initial)));" << endl;
	out << tab << tab << tab << "else if ((cont = dispatch_event(&state::unconditional)));" << endl;
	out << tab << tab << tab << "else if (event_queue.size()) cont = dispatch_event(event_queue.front()), event_queue.pop();" << endl;
	out << tab << tab << tab << "else break;" << endl;
	out << tab << tab << "}" << endl;
	out << tab << "}" << endl;
	out << endl;

	// constructor
	out << tab << classname() << "(user_model_p user = user_model_p())";
	if(!sc.using_parallel) out << " : cur_state(&m_scxml)";
	out << endl;

	out << tab << "{" << endl;
	if(sc.using_parallel) out << tab << tab << "cur_state.push_back(&m_scxml);" << endl;
	out << tab << tab << "model.user = user;" << endl;
	out << tab << tab << "dispatch(&state::initial);" << endl;
	out << tab << "}" << endl;
	out << endl;

	//m_scxml
	out << tab << "class scxml : public composite<scxml, state>" << endl;
	out << tab << "{" << endl;

	const int sz = sc.sc().initial.size();
	out << tab << tab << state_t() << "* initial(" << classname() << "&sc) { return transition";
	if(sz > 1) out << sz;
	out << "<&state::initial, scxml";
	for(int i = 0; i < sz; ++i) out << ", state_" << sc.sc().initial[i];
	out << ", internal>()(this";
	for(int i = 0; i < sz; ++i) out << ", sc.m_state_" << sc.sc().initial[i];
	out << ", sc); }" << endl;

	out << tab << "} m_scxml;" << endl;
	out << endl;
	
	// states
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		gen_state(*s->get());
	}

	out << tab << "std::queue<event> event_queue;" << endl;

	out << "};" << endl;
	out << endl;
}

void cpp_output::trim()
{
	const scxml_parser::state_list &states = sc.sc().states;

	// replace '-' with '_' in event names
	// replace '.' with '_' in event names todo: see '3.12.1 event descriptors how to handle event tokens
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		for (scxml_parser::transition_list::const_iterator i = s->get()->transitions.begin(); i != s->get()->transitions.end(); ++i) {
			if(i->get()->event) {
				if(i->get()->event->find('.') != string::npos) cerr << "warning: event tokens not supported" << endl;
				replace(i->get()->event->begin(), i->get()->event->end(), '.', '_');
				replace(i->get()->event->begin(), i->get()->event->end(), '-', '_');
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

