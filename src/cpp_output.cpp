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
#include <boost/algorithm/string/split.hpp>
#include <set>
#include <iostream>

using namespace std;


string event_name(string event)
{
	string name = "event";
	if (event != "*") name += '_' + event;
	replace(name.begin(), name.end(), '.', '_'); // replace '.' with '_'
	return name;
}

//todo set private/public/protected 

string cpp_output::classname()		{ return "sc_" + sc.sc().name; }
string cpp_output::state_t()		{ return "state"; }
string cpp_output::state_composite_t()	{ return "composite"; }
string cpp_output::state_parallel_t()	{ return "parallel"; }
string cpp_output::state_actions_t()	{ return "state_actions"; }
	
void cpp_output::gen_transition_base()
{
	string ret = state_t() + "*";
	if (sc.using_parallel) ret = state_t() + "::state_list";
	string empty = "0";
	if (sc.using_parallel) empty = "state::state_list()";

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

	// combined external/internal transition
	out << tab << "// external/internal transition" << endl;
	out << tab << "template<event E, class S, class D = no_state, transition_type T = external> class transition : public transition_actions<E, S, D>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "template<transition_type I> struct id { };" << endl;
	// when exit/enter is called here without parameter, it forces the action to always exit/enter at least current state
	// for internal transitions, D must be child of S, otherwise, handle as external transition
	// S is the source state of the transition, not current state
	out << tab << tab << "void state_enter(D* d, data_model &m, id<internal>, S*) { d->template enter<S>(m, (S*)0); };" << endl;
	out << tab << tab << "void state_enter(D* d, data_model &m, ...) { d->template enter<typename S::parent_t>(m); };" << endl;
	out << tab << tab << "void state_exit(S* s, data_model &m, id<internal>, S*) {};" << endl;
	out << tab << tab << "void state_exit(S* s, data_model &m, ...) { s->template exit<typename D::parent_t>(m); };" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << ret << " operator ()(S *s, " << classname() << " &sc)" << endl;
	out << tab << tab << "{" << endl;
	out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(sc.model)) return " << empty << ';' << endl;
	if(opt.debug) out << tab << tab << tab << "std::clog << \"" << classname() << ": transition \" << typeid(S).name() << \" -> \" << typeid(D).name() << std::endl;" << endl;
	out << tab << tab << tab << "D *d = sc.new_state<D>();" << endl;
	if (sc.using_parallel) out << tab << tab << tab << "s->exit_parallel(sc, s, d);" << endl;
	if (sc.using_compound) out << tab << tab << tab << "s->exit(sc.model, typeid(S));" << endl;
	out << tab << tab << tab << "state_exit(s, sc.model, id<T>(), (typename D::parent_t*)0);" << endl;
	out << tab << tab << tab << "transition_actions<E, S, D>::enter(sc.model);" << endl;
	out << tab << tab << tab << "state_enter(d, sc.model, id<T>(), (typename D::parent_t*)0);" << endl;
	if (sc.using_parallel) {
		out << tab << tab << tab << state_t() << "::state_list r = d->template enter_parallel<typename S::parent_t>(sc, d, (typename S::parent_t*)0);" << endl;
		out << tab << tab << tab << "r.push_back(d);" << endl;
		out << tab << tab << tab << "return r;" << endl;
	} else {
		out << tab << tab << tab << "return d;" << endl;
	}   
	out << tab << tab << "}" << endl;
	out << tab << "};" << endl;
	out << endl;

	// transition without target
	if (sc.using_transition_no_target) {
		out << tab << "// transition with no target" << endl;
		out << tab << "template<event E, class S> class transition<E, S, no_state> : public transition_actions<E, S, no_state>" << endl;
		out << tab << "{" << endl;
		out << tab << tab << "public:" << endl;
		out << tab << tab << "S* operator ()(S *s, " << classname() << " &sc)" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "if(!transition_actions<E, S, no_state>::condition(sc.model)) return " << empty << ";" << endl;
		if(opt.debug) out << tab << tab << tab << "std::clog << \"" << classname() << ": transition \" << typeid(S).name() << std::endl;" << endl;
		out << tab << tab << tab << "transition_actions<E, S, no_state>::enter(sc.model);" << endl;
		out << tab << tab << tab << "return s;" << endl;
		out << tab << tab << "}" << endl;
		out << tab << "};" << endl;
		out << endl;
	}

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
		out << tab << tab << state_t() << "::state_list operator ()(S *s, " << classname() << "&sc)" << endl;

		out << tab << tab << '{' << endl;

		out << tab << tab << tab << "if(!transition_actions<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ">::condition(sc.model)) return " << empty << ";" << endl;

		if(opt.debug) {
			out << tab << tab << tab << "std::clog << \"" << classname() << ": transition \" << typeid(S).name() << \" -> \"";
			for(int i = 0; i < sz; ++i) {
				if(i) out << " << \", \"";
				out << " << typeid(D" << i << ").name()";
			}
			out << " << std::endl;" << endl;
		}

		for (int i = 0; i < sz; ++i) {
			out << tab << tab << tab << 'D' << i << "*d" << i << " = sc.new_state<D" << i << ">();" << endl;
		}
		out << tab << tab << tab << "s->exit_parallel(sc, s, d0);" << endl;
		if(sc.using_compound) out << tab << tab << tab << "s->exit(sc.model, typeid(S));" << endl;
		out << tab << tab << tab << "s->template exit<D0>(sc.model, (D0*)0);" << endl;

		out << tab << tab << tab << "transition_actions<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ">::enter(sc.model);" << endl;

		out << tab << tab << tab << "d0->template enter<S>(sc.model, (S*)0);" << endl;
		out << tab << tab << tab << state_t() << "::state_list r = d0->template enter_parallel<S>(sc, d0, s";
		for(int i = 1; i < sz; ++i) out << ", d" << i;
		out << ");" << endl;

		out << tab << tab << tab << "r.push_back(d0);" << endl;
		out << tab << tab << tab << "return r;" << endl;

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
	string ret = state_t() + "*";
	if (sc.using_parallel) ret = "state_list";
	string retp = ret;
	if (sc.using_parallel) retp = "state::" + ret;
	string empty = "0";
	if (sc.using_parallel) empty = "state::state_list()";

	out << tab << "template<class C, class P> class " << state_composite_t() << " : public P, public state_actions<C>" << endl;
	out << tab << "{" << endl;

	out << tab << tab << "virtual " << retp << " initial(" << classname() << "&) { return " << empty << "; }" << endl;

	// lca calculation
	out << tab << tab << "public:" << endl;
	out << tab << tab << "typedef P parent_t;" << endl;
	out << tab << tab << "// LCA calculation" << endl;
	out << tab << tab << "template<class T> void enter(data_model&, " << state_composite_t() << "*) {}" << endl;

	out << tab << tab << "template<class T> void enter(data_model &m, ...) { P::template enter<T>(m, (T*)0);";
	if(opt.debug) out << " std::clog << \"" << classname() << ": enter \" << typeid(C).name() << std::endl;";
	out << " state_actions<C>::enter(m); }" << endl;

	out << tab << tab << "template<class T> void exit(data_model&, " << state_composite_t() << "*) {}" << endl;

	out << tab << tab << "template<class T> void exit(data_model &m, ...) {";
	if(opt.debug) out << " std::clog << \"" << classname() << ": exit \" << typeid(C).name() << std::endl;";
	out << " state_actions<C>::exit(m); P::template exit<T>(m, (T*)0); }" << endl;

	if(sc.using_compound) {
		out << tab << tab << "virtual void exit(data_model &m, const std::type_info &sti) { if(typeid(C) == sti) return;";
		if(opt.debug) out << " std::clog << \"" << classname() << ": exit \" << typeid(C).name() << std::endl;";
		out << " state_actions<C>::exit(m); P::exit(m, sti); }" << endl;
	}

	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_state_parallel_base()
{
	const scxml_parser::state_list &states = sc.sc().states;

	// todo for now, assume all targets have same parallel parent. Split if multiple?
	// limit and validate for this. Not easy to handle.

	if (!sc.using_parallel) return;

	string empty = state_t() + "::state_list()";
	string ret = "state_list";

	std::set<int> parallel_sizes;
	for (scxml_parser::state_list::const_iterator istate = states.begin(); istate != states.end(); ++istate) {
		if (istate->get()->type && *istate->get()->type == "parallel") {
			parallel_sizes.insert(children(*istate->get()).size());
		}
	}

	if(parallel_sizes.size() == 0) {
		//todo make composite, if children < 2
		cerr << "error: parallel state with < 2 states is currently not supported" << endl;
		exit(1);
	}

	const int min_c = *parallel_sizes.begin();
	const int max_c = *parallel_sizes.rbegin();
	if(min_c < 2) {
		//todo make composite, if children < 2
		cerr << "error: parallel state with < 2 states is currently not supported" << endl;
		exit(1);
	}

	//todo combine with no_class
	if(min_c < max_c) out << tab << "class no_class {};" << endl;

	for (set<int>::reverse_iterator i = parallel_sizes.rbegin(); i != parallel_sizes.rend(); ++i) {
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

		// handle transition with all children given
		out << tab << tab << "template<class S";
		for(int c = 1; c < children; ++c) out << ", class D" << c;
		out << "> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C*, C*";
		for(int c = 1; c < children; ++c) out << ", D" << c << "*";
		out << ") { return " << empty << "; }" << endl;
		out << tab << tab << "template<class S";
		for(int c = 1; c < children; ++c) out << ", class D" << c;
		out << "> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C *d, " << state_t() << '*';
		for(int c = 1; c < children; ++c) out << ", D" << c << " *d" << c;
		out << ")" << endl;
		out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// handle transition with all children given" << endl;
		// from this state and up, all tagets are same path, so only need to follow one of them.
		out << tab << tab << tab << state_t() << "::" << ret << " r = P::template enter_parallel<S>(sc, d, (S*)0);" << endl;
		for(int c = 1; c < children; ++c) out << tab << tab << tab << 'd' << c << "->template enter<C>(sc.model, (C*)0), r.push_back(d" << c << ");" << endl;
		out << tab << tab << tab << "return r;" << endl;
		out << tab << tab << '}' << endl;
		out << endl;

		for(int c = 0; c < children; ++c) {
			out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C" << c << "*, C*) { return " << state_t() << "::" << ret << "(); }" << endl;
			out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C" << c << " *d, " << state_t() << " *s)" << endl;
			out << tab << tab << '{' << endl;
			out << tab << tab << tab << "// parallel state entered with C" << c << " or child of as target" << endl;
			out << tab << tab << tab << "return enter_parallel<S>(sc, d, s";
			for(int n = 0; n < children; ++n) {
				if (n == c) continue;
				out << ", " << "sc.new_state<C" << n << ">()";
			}
			out << ");" << endl;
			out << tab << tab << "};" << endl;
			out << tab << tab << endl;

			//todo to continue
		}

        // other cases, eg parallel state as target
		out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) { return state::state_list(); }" << endl;

		out << endl;

		// parallel exit
		out << tab << tab << "bool parallel_parent(const std::type_info& pti) { return typeid(C) == pti; }" << endl;
        for (int n = 0; n < children; ++n) {
            out << tab << tab << "void exit_parallel(" << classname() << " &sc, C" << n << "*, C*) {}" << endl;
        }
		out << tab << tab << "void exit_parallel(" << classname() << " &sc, C *s, state *d)" << endl;
		out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// parallel state exited from C or child" << endl;
		out << tab << tab << tab << "for(" << classname() << "::" << state_t() << "::" << ret << "::iterator i = sc.cur_state.begin(); (i != sc.cur_state.end()) && *i; ++i) {" << endl;
		out << tab << tab << tab << tab << "if(typeid(*this) == typeid(**i)) continue;" << endl;
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

void cpp_output::gen_model_decl()
{
	out << tab << "struct data_model;" << endl;
	out << tab << "struct user_model;" << endl;
	out << endl;
}

void cpp_output::gen_model_base_data()
{
	typedef vector<pair<string, string> > pair_vect;
	pair_vect constructs;
	using namespace boost::algorithm;
	const scxml_parser::data_list &datamodel = sc.sc().datamodel;
	for (scxml_parser::data_list::const_iterator i_data = datamodel.begin(); i_data != datamodel.end(); ++i_data) {
		string id = i_data->get()->id;
		const boost::optional<string> expr_opt = i_data->get()->expr;
		vector<string> id_tokens;
		split(id_tokens, id, is_any_of(" "), token_compress_on);
		string type = "int"; // default type
		if (id_tokens.size() == 2) {
			type = id_tokens[0];
			id = id_tokens[1];
		}
		out << tab << tab << type << ' ' << id << ';' << endl;
		if (expr_opt) {
			constructs.push_back(make_pair(id, *expr_opt));
		}
	}
	if (constructs.size()) {
		out << tab << tab << "data_model() : ";
		for (pair_vect::const_iterator i_construct = constructs.begin(); i_construct != constructs.end(); ++i_construct) {
			if (i_construct != constructs.begin()) out << ", ";
			out << i_construct->first << '(' << i_construct->second << ')';
		}	
		out << " {}" << endl;
	}
}

void cpp_output::gen_model_base()
{
	out << tab << "struct data_model" << endl;
	out << tab << "{" << endl;
	if (!opt.bare_metal) out << tab << tab << "std::queue<event> event_queue;" << endl;
	out << tab << tab << "user_model *user;" << endl;
	gen_model_base_data();
	out << tab << "} model;" << endl;
	out << endl;
}

void cpp_output::gen_state_base()
{
	using namespace boost::algorithm;

	const scxml_parser::state_list &states = sc.sc().states;

	// state actions class
	out << tab << "class " << state_t() << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;

	if (sc.using_parallel) {
		out << tab << tab << "struct state_list : public std::vector<" << state_t() << "*>" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "operator bool() { return !empty(); }" << endl;
		out << tab << tab << "};" << endl;
	}

	// events
	// generate events for each token, eg:
	// ev.a.b.c =>
	// ev        call '*' - if there
	// ev_a      call ev
	// ev_a_b    call ev_a
	// ev_a_b_c  call ev_a_b

	// collect events
	vector<string> events;
	for (scxml_parser::state_list::const_iterator i_state = states.begin(); i_state != states.end(); ++i_state) {
		if (i_state->get()->type && *i_state->get()->type == "final") {
			string event = "done.state." + i_state->get()->parent->id;
			events.push_back(event);
		}
		for (scxml_parser::transition_list::const_iterator i_trans = i_state->get()->transitions.begin(); i_trans != i_state->get()->transitions.end(); ++i_trans) {
			for(scxml_parser::slist::const_iterator i_event = i_trans->get()->event.begin(); i_event != i_trans->get()->event.end(); ++i_event) {
				events.push_back(*i_event);
			}
		}
	}

	// pass through set, to filter out dublicates
	bool use_base_event = false;
	set<string> event_set;
	for (vector<string>::const_iterator i_event = events.begin(); i_event != events.end(); ++ i_event) {
		if (*i_event == "*") {
			use_base_event = true;
		}
		// loop through event tokens 
		scxml_parser::slist tokens;
		split(tokens, *i_event, is_any_of("."), token_compress_on);
		string event;
		for (scxml_parser::slist::const_iterator i_token = tokens.begin(); i_token != tokens.end(); ++i_token) {
			if (event.size()) event += '.';
			event += *i_token;
			event_set.insert(event);
		}
	}

	string ret = state_t() + "*";
	if (sc.using_parallel) ret = "state_list";
	string retp = ret;
	if (sc.using_parallel) retp = "state::" + ret;
	string empty = "0";
	if (sc.using_parallel) empty = "state_list()";

	(void)use_base_event;
	for (set<string>::const_iterator i_event = event_set.begin(); i_event != event_set.end(); ++i_event) {

		// event parent
		string parent;
		scxml_parser::slist tokens;

		size_t delim = i_event->rfind(".");
		if (delim != string::npos) parent = i_event->substr(0, delim);

		// event name
		string event;
		if (*i_event != "*") event = "_" + *i_event;

		// replace '.' with '_'
		replace(event.begin(), event.end(), '.', '_');
		replace(parent.begin(), parent.end(), '.', '_');

		out << tab << tab << "virtual " << ret << " event" << event << "(" << classname() << '&';
		if (parent.size() || (event.size() && use_base_event)) out << " sc";
		out << ") { return ";
		if (parent.size()) out << "event_" << parent << "(sc)";
		else if (event.size() && use_base_event) out << "event(sc)";
		else out << empty;
		out << "; }" << endl;
	}
	out << tab << tab << "virtual " << ret << " unconditional(" << classname() << "&) { return " << empty << "; }" << endl;
	out << tab << tab << "virtual " << ret << " initial(" << classname() << "&) { return " << empty << "; }" << endl;

	out << endl;

	out << tab << tab << "template<class T> void enter(data_model&, ...) {}" << endl;
	out << tab << tab << "template<class T> void exit(data_model&, ...) {}" << endl;
	if(sc.using_compound) out << tab << tab << "virtual void exit(data_model&, const std::type_info&) {}" << endl;
	if(sc.using_parallel) {
		out << tab << tab << "template<class S> " << retp << " enter_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) { return state_list(); }" << endl;
		out << tab << tab << "virtual void exit_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) {}" << endl;
		out << tab << tab << "virtual bool parallel_parent(const std::type_info&) { return false; }" << endl;
	}

	// removed - this may require delete() wihich is'nt available in some embedded setups
	//out << tab << tab << "virtual ~" << state_t() << "() {}" << endl;

	out << tab << "};" << endl;
	out << endl;

	if(sc.using_parallel) {
		out << tab << state_t() << "::state_list cur_state;" << endl;
	}
	else {
		out << tab << state_t() << " *cur_state;" << endl;
	}
	out << tab << "typedef " << retp << " (" << state_t() << "::*event)(" << classname() << "&);" << endl;
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
	const bool use_ancestor = sc.using_compound; // call parent if condition is false
	const bool parallel_state = state.type && *state.type == "parallel";
	const bool final_state = state.type && *state.type == "final";
    	string ret = state_t() + "*";
    	if (sc.using_parallel) ret = "state_list";
	string retp = ret;
	if (sc.using_parallel) retp = "state::" + ret;
	string empty = "0";
	if (sc.using_parallel) empty = "state_list()";

	string parent, prefix;
	if(state.type && *state.type == "inter") prefix = "inter";
	if(state.parent) parent = "state_" + state.parent->id;
	else parent = "scxml";
	string state_classname = prefix + "state_" + state.id;

        if(parallel_state) {
		scxml_parser::state_list states = children(state);
		for(scxml_parser::state_list::const_iterator i = states.begin(); i != states.end(); ++i) {
			out << tab << "struct state_" << (*i)->id << ';' << endl;
		}
	}
	out << tab << "struct " << state_classname << " : public ";
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

	if (final_state) {
		//todo handle final in parallel states
		out << tab << tab << ret << " initial" << "(" << classname() << " &sc) { sc.model.event_queue.push(&state::event_done_" << parent << "); return " << empty << "; }" << endl;
	}
	else if(state.initial.target.size()) {
		const int sz = state.initial.target.size();
		out << tab << tab << ret << " initial" << "(" << classname() << " &sc) { return transition";
		if (sz > 1) out << sz;
		out << "<&state::initial, " << state_classname;
		for (int i = 0; i < sz; ++i) out << ", state_" << state.initial.target[i];
	       	out << ", internal>()(this, sc); }" << endl;
	}

	//events

	// build a map with event as key with vector of transitions with this event
	std::map<std::string, scxml_parser::transition_list> event_map;
	for (scxml_parser::transition_list::const_iterator itrans = state.transitions.begin(); itrans != state.transitions.end(); ++itrans) {
		if (itrans->get()->event.size() == 0) {
			event_map["unconditional"].push_back(*itrans);
		}
		else for (scxml_parser::slist::const_iterator ievent = itrans->get()->event.begin(); ievent != itrans->get()->event.end(); ++ievent) {
			event_map[event_name(*ievent)].push_back(*itrans);
		}
	}
	
	for (std::map<std::string, scxml_parser::transition_list>::const_iterator mi = event_map.begin(); mi != event_map.end(); ++mi) {

		string indent;
		for (scxml_parser::transition_list::const_iterator t = mi->second.begin(); t != mi->second.end(); ++t) {
			string target_classname = state_classname;
			const string event = mi->first;
			const bool first = t == mi->second.begin();
			const bool multiple = mi->second.size() > 1 || use_ancestor;
			const bool last = t == mi->second.end() - 1;
			const bool internal = !parallel_state && t->get()->type && *t->get()->type == "internal";
			const bool has_target = t->get()->target.size();

			if(has_target) {
				target_classname = "state_" + t->get()->target.front(); //todo handle multiple targets
			}

			if (first) {
				string s = ret + ' ' + event + "(" + classname() + " &sc) { ";
				if (multiple) s += ret + " s; ";
				s += "return ";
				out << tab << tab << s;
				indent = string(s.size() - 3, ' ');
			}
			else out << tab << tab << indent << "|| ";
			if (multiple) out << "(s = ";
			if (has_target) {
				// normal transition
				string type_str;
				if (internal) type_str = ", internal";
				out << "transition<&state::" << event << ", " << state_classname << ", " << target_classname << type_str << ">()(this, sc)";
			}
			else {
				// transition without target
				out << "transition<&state::" << event << ", " << state_classname << ">()(this, sc)";
			}

			string ancestor;
			if (use_ancestor) {
				ancestor = " || (s = parent_t::" + event + "(sc))";
			}

			if (multiple && last) out << ")" << ancestor << ", s";
			else if (multiple) out << ")" << endl;
			if (last) out << "; }" << endl;
		}
	}

	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_sc()
{
	const scxml_parser::state_list &states = sc.sc().states;
    	string ret = state_t() + "*";
    	if (sc.using_parallel) ret = "state_list";

	out << "class " << classname() << endl;
	out << "{" << endl;
	out << tab << "public:" << endl;
	out << endl;

	gen_model_decl();
	gen_state_base();
	gen_model_base();
	gen_state_actions_base();
	gen_state_composite_base();
	gen_state_parallel_base();
	gen_transition_base();

	// dispatch
	if(sc.using_parallel) {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "bool cont = false;" << endl;
		out << tab << tab << "for(state::state_list::iterator i_cur = cur_state.begin(); i_cur != cur_state.end();) if(*i_cur) {" << endl;
		out << tab << tab << tab << "if (state::state_list r = ((*i_cur)->*e)(*this)) {" << endl;
		out << tab << tab << tab << tab << "cont = true;" << endl;
		out << tab << tab << tab << tab << "std::vector<state*>::const_iterator i_new = r.begin();" << endl;
		out << tab << tab << tab << tab << "*i_cur = *i_new++;" << endl;
		out << tab << tab << tab << tab << "for(; i_new != r.end(); ++i_new) i_cur = ++cur_state.insert(i_cur, *i_new);" << endl;
		out << tab << tab << tab << '}' << endl;
		out << tab << tab << tab << "++i_cur;" << endl;
		out << tab << tab << '}' << endl;
		out << tab << tab << "else i_cur = cur_state.erase(i_cur);" << endl;
		out << tab << tab << "return cont;" << endl;
		out << tab << '}' << endl;
	}
	else {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "state *next_state;" << endl;
		out << tab << tab << "if ((next_state = (cur_state->*e)(*this))) cur_state = next_state;" << endl;
		out << tab << tab << "return !!next_state;" << endl;
		out << tab << '}' << endl;
	}
	out << endl;
	out << tab << "public: void dispatch(event e)" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "bool cont = dispatch_event(e);" << endl;
	out << tab << tab << "while (cont) {" << endl;
	out << tab << tab << tab << "if ((cont = dispatch_event(&state::initial)));" << endl;
	out << tab << tab << tab << "else if ((cont = dispatch_event(&state::unconditional)));" << endl;
	if(!opt.bare_metal) out << tab << tab << tab << "else if (model.event_queue.size()) cont = dispatch_event(model.event_queue.front()), model.event_queue.pop(), cont |= !model.event_queue.empty();" << endl;
	out << tab << tab << tab << "else break;" << endl;
	out << tab << tab << "}" << endl;
	out << tab << "}" << endl;
	out << endl;

	// constructor
	out << tab << classname() << "(user_model *user = 0)";
	if(!sc.using_parallel) out << " : cur_state(new_state<scxml>())";
	out << endl;

	out << tab << "{" << endl;
	if(sc.using_parallel) out << tab << tab << "cur_state.push_back(new_state<scxml>());" << endl;
	out << tab << tab << "model.user = user;" << endl;
	out << tab << "}" << endl;

	// init
	out << endl;
	out << tab << "void init() { dispatch(&state::initial); }" << endl;
	out << endl;

	//scxml base
	out << tab << "class scxml : public composite<scxml, state>" << endl;
	out << tab << "{" << endl;

	const int sz = sc.sc().initial.target.size();
	out << tab << tab << ret << " initial(" << classname() << "&sc) { return transition";
	if(sz > 1) out << sz;
	out << "<&state::initial, scxml";
	for(int i = 0; i < sz; ++i) out << ", state_" << sc.sc().initial.target[i];
	out << ", internal>()(this, sc); }" << endl;

	out << tab << "};" << endl;
	out << endl;
	
	// states
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		gen_state(*s->get());
	}

	// new_state
	out << tab << "template<class T> T* new_state()" << endl;
	out << tab << '{' << endl;
	out << tab << tab << "static T t;" << endl;
	out << tab << tab << "return &t;" << endl;
	out << tab << "};" << endl;

	out << "};" << endl;
	out << endl;
}

void cpp_output::gen_action_part_log(scxml_parser::action &a)
{
	const string label = a.attr["label"];
	const string expr = a.attr["expr"];

	//out << tab << tab << "// " << a.type << " label=" << label << " expr=" << expr << endl;
	out << tab << tab << "std::clog";
	if(label.size()) out << " << \"[" << label << "] \""; 
	out << " << \"" << expr << "\" << std::endl;" << endl;
}

void cpp_output::gen_action_part_assign(scxml_parser::action &a)
{
	const string location = a.attr["location"];
	const string expr = a.attr["expr"];

	//out << tab << tab << "// " << a.type << " location=" << location << " expr=" << expr << endl;
	out << tab << tab << location << " = " << expr << ';' << endl;
}

void cpp_output::gen_action_part_script(scxml_parser::action &a)
{
	const string expr = a.attr["expr"];

	//out << tab << tab << "// " << a.type << " expr=" << expr << endl;
	out << tab << tab << expr << endl;
}

void cpp_output::gen_action_part_raise(scxml_parser::action &a)
{
	const string ev = a.attr["event"];

	//out << tab << tab << "// " << a.type << " event=" << ev << endl;
	out << tab << tab << "event_queue.push(&" << classname() << "::state::event_" << ev << ");" << endl;
}

void cpp_output::gen_action_part(scxml_parser::action &a)
{
	if(a.type == "raise") gen_action_part_raise(a);
	else if(a.type == "log") gen_action_part_log(a);
	else if(a.type == "assign") gen_action_part_assign(a);
	else if(a.type == "script") gen_action_part_script(a);
	else {
		out << tab << tab << "// warning: unknown action type '" << a.type << "'" << endl;
		cerr << "warning: unknown action type '" << a.type << "'" << endl;
	}
}

void cpp_output::gen_with_begin(bool cond)
{
	string ret = "void";
	if (cond) ret = "bool";
	out << tab << "struct with : " << classname() << "::data_model { " << ret << " operator ()() { // 'with' construct, to bring model variables in scope" << endl;
}

void cpp_output::gen_with_end(bool cond)
{
	string ret;
	if (cond) {
		ret = " return";
	}
	out << tab << "}};" << ret << " static_cast<with&>(m)();" << endl;
}
	
void cpp_output::gen_actions()
{
	const scxml_parser::state_list &states = sc.sc().states;
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {

		// entry actions
		if(s->get()->entry_actions.size()) {
			out << "template<> void " << classname() << "::state_actions<" << classname() << "::state_" << s->get()->id << ">::enter(" << classname() << "::data_model &m)" << endl;
			out << '{' << endl;
			gen_with_begin(false);
			for (scxml_parser::plist<scxml_parser::action>::const_iterator i = s->get()->entry_actions.begin(); i != s->get()->entry_actions.end(); ++i) {
				gen_action_part(*i->get());
			}
			gen_with_end(false);
			out << '}' << endl;
			out << endl;
		}

		// exit actions
		if(s->get()->exit_actions.size()) {
			out << "template<> void " << classname() << "::state_actions<" << classname() << "::state_" << s->get()->id << ">::exit(" << classname() << "::data_model &m)" << endl;
			out << '{' << endl;
			gen_with_begin(false);
			for (scxml_parser::plist<scxml_parser::action>::const_iterator i = s->get()->exit_actions.begin(); i != s->get()->exit_actions.end(); ++i) {
				gen_action_part(*i->get());
			}
			gen_with_end(false);
			out << '}' << endl;
			out << endl;
		}

		// initial actions
		if(s->get()->initial.actions.size()) {
			out << "template<> void " << classname() << "::transition_actions<&" << classname() << "::state::initial, " << classname() << "::state_" << s->get()->id;
			for(scxml_parser::slist::const_iterator ai = s->get()->initial.target.begin(); ai != s->get()->initial.target.end(); ++ai) {
				out << ", " << classname() << "::state_" << *ai;
			}
		       	out << ">::enter(" << classname() << "::data_model &m)" << endl;
			out << '{' << endl;
			gen_with_begin(false);
			for (scxml_parser::plist<scxml_parser::action>::const_iterator i = s->get()->initial.actions.begin(); i != s->get()->initial.actions.end(); ++i) {
				gen_action_part(*i->get());
			}
			gen_with_end(false);
			out << '}' << endl;
			out << endl;
		}

		// transition actions
		// todo support multiple events. create an action for each event which calls a common action? - this would work only for the last transition if multiple transitions because of the call priority.
		for (scxml_parser::transition_list::const_iterator itrans = s->get()->transitions.begin(); itrans != s->get()->transitions.end(); ++itrans) {
			if(itrans->get()->actions.size()) {
				string event = "unconditional";
				if(itrans->get()->event.size()) event = event_name(itrans->get()->event.front());

				out << "template<> void " << classname() << "::transition_actions<&" << classname() << "::state::" << event << ", " << classname() << "::state_" << s->get()->id;
				for(scxml_parser::slist::const_iterator iaction = itrans->get()->target.begin(); iaction != itrans->get()->target.end(); ++iaction) {
					out << ", " << classname() << "::state_" << *iaction;
				}
			       	out << ">::enter(" << classname() << "::data_model &m)" << endl;
				out << '{' << endl;
				gen_with_begin(false);
				for (scxml_parser::plist<scxml_parser::action>::const_iterator i = itrans->get()->actions.begin(); i != itrans->get()->actions.end(); ++i) {
					gen_action_part(*i->get());
				}
				gen_with_end(false);
				out << '}' << endl;
				out << endl;
			}
			if (itrans->get()->condition) {
				string event = "unconditional";
				if(itrans->get()->event.size()) event = event_name(itrans->get()->event.front());

				out << "template<> bool " << classname() << "::transition_actions<&" << classname() << "::state::" << event << ", " << classname() << "::state_" << s->get()->id;
				for(scxml_parser::slist::const_iterator iaction = itrans->get()->target.begin(); iaction != itrans->get()->target.end(); ++iaction) {
					out << ", " << classname() << "::state_" << *iaction;
				}
			       	out << ">::condition(" << classname() << "::data_model &m)" << endl;
				out << '{' << endl;
				gen_with_begin(true);
				out << tab << tab << "return " << *itrans->get()->condition << ';' << endl;
				gen_with_end(true);
				out << '}' << endl;
				out << endl;
			}
		}
		//out << endl;
	}
}

void cpp_output::trim()
{
	const scxml_parser::state_list &states = sc.sc().states;

	// replace '-' with '_' in event names
	// replace '-' with '_' in actions
        // remove '.*' postfix in events
	for (scxml_parser::state_list::const_iterator istate = states.begin(); istate != states.end(); ++istate) {
		for (scxml_parser::transition_list::const_iterator itrans = istate->get()->transitions.begin(); itrans != istate->get()->transitions.end(); ++itrans) {
			for (scxml_parser::slist::iterator ievent = itrans->get()->event.begin(); ievent != itrans->get()->event.end(); ++ievent) {
				replace(ievent->begin(), ievent->end(), '-', '_');
                if (ievent->size() >= 2 && string(ievent->rbegin(), ievent->rbegin() + 2) == "*.") ievent->erase(ievent->size()-2, 2);
			}

			// transition actions
			for (scxml_parser::plist<scxml_parser::action>::const_iterator iaction = itrans->get()->actions.begin(); iaction != itrans->get()->actions.end(); ++iaction) {
				for (map<string, string>::iterator iattr = iaction->get()->attr.begin(); iattr != iaction->get()->attr.end(); ++iattr) {
					replace(iattr->second.begin(), iattr->second.end(), '-', '_');
				}
			}
		}

		// entry actions
		for (scxml_parser::plist<scxml_parser::action>::const_iterator ai = istate->get()->entry_actions.begin(); ai != istate->get()->entry_actions.end(); ++ai) {
			for (map<string, string>::iterator attri = ai->get()->attr.begin(); attri != ai->get()->attr.end(); ++attri) {
				replace(attri->second.begin(), attri->second.end(), '-', '_');
			}
		}

		// exit actions
		for (scxml_parser::plist<scxml_parser::action>::const_iterator ai = istate->get()->exit_actions.begin(); ai != istate->get()->exit_actions.end(); ++ai) {
			for (map<string, string>::iterator attri = ai->get()->attr.begin(); attri != ai->get()->attr.end(); ++attri) {
				replace(attri->second.begin(), attri->second.end(), '-', '_');
			}
		}
	}
}

void cpp_output::gen()
{
	if (sc.using_parallel) cerr << "warning: parallel support is not fully implemented/tested" << endl;

	if (opt.bare_metal && sc.using_parallel) {
		cerr << "error: parallel states is not supported with bare metal C++" << endl;
		exit(1);
	}
	if (opt.bare_metal && sc.using_final) {
		cerr << "error: final states is not supported with bare metal C++" << endl;
		exit(1);
	}
	if (opt.bare_metal && sc.using_compound) {
		cerr << "error: Hierarchical states is not currenty supported with bare metal C++" << endl;
		exit(1);
	}
	if (opt.bare_metal && opt.debug) {
		cerr << "error: The debug option is not currenty supported with bare metal C++" << endl;
		exit(1);
	}

	// include guard
	out << "// This file is automatically generated by scxmlcc (version " << version() << ")" << endl;
	out << "// For more information, see http://scxmlcc.org" << endl;
	out << endl;
	out << "#ifndef __SC_" << boost::to_upper_copy(sc.sc().name) << endl;
	out << "#define __SC_" << boost::to_upper_copy(sc.sc().name) << endl;
	out << endl;

	if(sc.using_compound || opt.debug) out << "#include <typeinfo>" << endl;
	if(!opt.bare_metal) out << "#include <queue>" << endl;
	if(opt.debug || sc.using_log) out << "#include <iostream>" << endl;
	out << endl;

	trim();
	gen_sc();
	gen_actions();

	// end of include guard
 	out << "#endif" << endl;

}

