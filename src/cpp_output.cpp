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
string cpp_output::state_final_t()	{ return "final"; }
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

	// lcca calculation for transition
	if (sc.using_parallel) {
		out << tab << tab << "struct lcca_not_fonud_t { char c[2]; };" << endl;
		out << tab << tab << "template<class A> static char lcca_test(const A*);" << endl;
		out << tab << tab << "template<class A> static lcca_not_fonud_t lcca_test(...);" << endl;
		out << tab << tab << "template<class A, class B, bool F = sizeof(lcca_test<A>(static_cast<B*>(nullptr))) == 1> struct lcca" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "typedef typename lcca<typename A::parent_t, B>::type type; " << endl;
		out << tab << tab << "};" << endl;
		out << tab << tab << "template<class A, class B> struct lcca<A, B, true>" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "typedef A type; " << endl;
		out << tab << tab << "};" << endl;
		out << endl;
	}

	// combined external/internal transition
	out << tab << "// external/internal transition" << endl;
	out << tab << "template<event E, class S, class D = no_state, transition_type T = external> class transition : public transition_actions<E, S, D>" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "template<transition_type I> struct id { };" << endl;
	// when exit/enter is called here without parameter, it forces the action to always exit/enter at least current state
	// for internal transitions, D must be child of S, otherwise, handle as external transition
	// S is the source state of the transition, not current state

	// transition types:
	// no	type		lca	exit action				enter action
	//------------------------------------------------------------------------------------------------------------------	
	// 0	int		src	-					enter from lca to dst (only allowed int) 1-n
	// 1	ext		src	force exit lca 1			force enter lca 1, enter from lca to dst 1-n
	// 2	int/ext		parent	exit from src to lca 1-n		enter from lca 1-n	
	// 3	int/ext		src,dst	force exit lca 1			force enter lca 1
	// 4	int/ext		dst	exit to lca 1-n, force exit lca 	force enter lca 

	// since internal use S instead S::parent_t, use composite instead S, so enter is not forced on parallel states in this case

	out << tab << tab << "void state_enter(D* d, data_model &m, id<internal>, S*) { d->template enter<composite<S, typename S::parent_t> >(m); } // internal transition, where dst is descendant of src" << endl;
	out << tab << tab << "void state_enter(D* d, data_model &m, ...) { d->template enter<typename S::parent_t>(m); } // external transition, or dst is not descendant of src" << endl;
	out << tab << tab << "void state_exit(S*, data_model &, id<internal>, S*) {} // internal transition, where dst is descendant of src" << endl;
	out << tab << tab << "void state_exit(S* s, data_model &m, ...) { s->template exit<typename D::parent_t>(m); } // external transition, or dst is not descendant of src" << endl;
	if (sc.using_parallel) {
		out << tab << tab << "void state_exit_parallel(S*, D*, " << classname() << " &, id<internal>, S*) {} // internal transition, where dst is descendant of src" << endl;
		out << tab << tab << "void state_exit_parallel(S* s, D *d, " << classname() << " &sc, ...) { s->exit_parallel(sc, s, d); } // external transition, or dst is not descendant of src" << endl;
		out << tab << tab << "bool would_exit(state* n, id<internal>, S*) { return !!dynamic_cast<S*>(n) && typeid(S) != typeid(*n); }" << endl;
		out << tab << tab << "bool would_exit(state* n, ...) { return !!dynamic_cast<typename lcca<S, D>::type*>(n) && typeid(typename lcca<S, D>::type) != typeid(*n); }" << endl;
	}
	
	out << tab << tab << "public:" << endl;
	if (sc.using_parallel) out << tab << tab << ret << " operator ()(S *s, " << classname() << " &sc, state::eval_data &eval";
	else out << tab << tab << ret << " operator ()(S *s, " << classname() << " &sc";
	if (opt.debug == "clog") out << ", const char* ename)" << endl;
	else out << ")" << endl;
	out << tab << tab << "{" << endl;
	if (sc.using_parallel) {
		out << tab << tab << tab << "if (eval.filtering) {" << endl;
		out << tab << tab << tab << tab << state_t() << "::state_list r;" << endl;
		out << tab << tab << tab << tab << "if(transition_actions<E, S, D>::condition(sc.model)) {" << endl;
		out << tab << tab << tab << tab << tab << "eval.enabled.push_back(typeid(*this));" << endl;
		out << tab << tab << tab << tab << tab << "r.push_back(s);" << endl;

		out << tab << tab << tab << tab << tab << "state::eval_data::eval_item t1;" << endl;
		out << tab << tab << tab << tab << tab << "t1.s = sc.get_state<S>();" << endl;
		out << tab << tab << tab << tab << tab << "t1.cur = s;" << endl;

		out << tab << tab << tab << tab << tab << "size_t i_mask = 0;" << endl;
		out << tab << tab << tab << tab << tab << "for(state::state_list::iterator i_cur = sc.model.cur_state.begin(); i_cur != sc.model.cur_state.end(); ++i_cur) if (*i_cur) {" << endl;
		out << tab << tab << tab << tab << tab << tab << "t1.exit_mask[i_mask++] = would_exit(*i_cur, id<T>(), static_cast<typename D::parent_t*>(nullptr));" << endl;
		out << tab << tab << tab << tab << tab << "}" << endl;
		out << tab << tab << tab << tab << tab << "bool t1_preemted = false;" << endl;
		out << tab << tab << tab << tab << tab << "for (state::eval_data::eval_list::iterator i_t2 = eval.filtered.begin(); i_t2 != eval.filtered.end();) {" << endl;
		out << tab << tab << tab << tab << tab << tab << "bool erased = false;" << endl;
		out << tab << tab << tab << tab << tab << tab << "if ((t1.exit_mask & i_t2->exit_mask).any()) {" << endl;
		out << tab << tab << tab << tab << tab << tab << tab << "if (i_t2->s->is_descendant(t1.s)) i_t2 = eval.filtered.erase(i_t2), erased = true;" << endl;
		out << tab << tab << tab << tab << tab << tab << tab << "else t1_preemted = true;" << endl;
		out << tab << tab << tab << tab << tab << tab << "}" << endl;
		out << tab << tab << tab << tab << tab << tab << "if (!erased) ++i_t2;" << endl;
		out << tab << tab << tab << tab << tab << "}" << endl;
		out << tab << tab << tab << tab << tab << "if (!t1_preemted) eval.filtered.push_back(t1);" << endl;

		out << tab << tab << tab << tab << "}" << endl;
		out << tab << tab << tab << tab << "return r;" << endl;
		out << tab << tab << tab << "}" << endl;
		out << endl;
		out << tab << tab << tab << "bool enabled = false;" << endl;
		out << tab << tab << tab << "for (state::eval_data::enabled_list::const_iterator i = eval.enabled.begin(); i != eval.enabled.end() && !enabled; ++i) enabled = (i->get() == typeid(*this));" << endl; 
		out << tab << tab << tab << "if (!enabled) return " << empty << ';' << endl;
	}
	else {
		out << tab << tab << tab << "if(!transition_actions<E, S, D>::condition(sc.model)) return " << empty << ';' << endl;
	}
	if(opt.debug == "clog") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"" << classname() << ": transition [\" << ename << \"] \" << S::debug_name() << \" -> \" << D::debug_name() << std::endl;" << endl;
	else if(opt.debug == "scxmlgui") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"3 \" << S::debug_name() << \" -> \" << D::debug_name() << std::endl;" << endl;
	out << tab << tab << tab << "D *d = sc.get_state<D>();" << endl;
	if (sc.using_parallel) out << tab << tab << tab << "state_exit_parallel(s, d, sc, id<T>(), static_cast<typename D::parent_t*>(nullptr));" << endl;
	if (sc.using_compound) out << tab << tab << tab << "s->exit_to_src(sc.model, S::id());" << endl;
	out << tab << tab << tab << "state_exit(s, sc.model, id<T>(), static_cast<typename D::parent_t*>(nullptr));" << endl;
	out << tab << tab << tab << "transition_actions<E, S, D>::enter(sc.model);" << endl;
	out << tab << tab << tab << "state_enter(d, sc.model, id<T>(), static_cast<typename D::parent_t*>(nullptr));" << endl;
	if (sc.using_parallel) {
		out << tab << tab << tab << state_t() << "::state_list r = d->template enter_parallel<S>(sc, d, s);" << endl;
		out << tab << tab << tab << "r.push_back(d);" << endl;
	}

	if(opt.debug == "scxmlgui") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"2 \" << S::debug_name() << \" -> \" << D::debug_name() << std::endl;" << endl;

	if (sc.using_parallel) {
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
		if (sc.using_parallel) {
			out << tab << tab << "bool would_exit(state* n) { return !!dynamic_cast<S*>(n) && typeid(S) != typeid(*n); }" << endl;
		}
		out << tab << tab << "public:" << endl;
		if (sc.using_parallel) out << tab << tab << ret << " operator ()(S *s, " << classname() << " &sc, state::eval_data &eval";
		else out << tab << tab << ret << " operator ()(S *s, " << classname() << " &sc";
		if (opt.debug == "clog") out << ", const char* ename)" << endl;
		else out << ")" << endl;
		out << tab << tab << "{" << endl;
		if (sc.using_parallel) {
			out << tab << tab << tab << "if (eval.filtering) {" << endl;
			out << tab << tab << tab << tab << state_t() << "::state_list r;" << endl;
			out << tab << tab << tab << tab << "if (transition_actions<E, S, no_state>::condition(sc.model)) {" << endl;
			out << tab << tab << tab << tab << tab << "eval.enabled.push_back(typeid(*this));" << endl;
			out << tab << tab << tab << tab << tab << "r.push_back(s);" << endl;

			out << tab << tab << tab << tab << tab << "state::eval_data::eval_item t1;" << endl;
			out << tab << tab << tab << tab << tab << "t1.s = sc.get_state<S>();" << endl;
			out << tab << tab << tab << tab << tab << "t1.cur = s;" << endl;

			out << tab << tab << tab << tab << tab << "size_t i_mask = 0;" << endl;
			out << tab << tab << tab << tab << tab << "for(state::state_list::iterator i_cur = sc.model.cur_state.begin(); i_cur != sc.model.cur_state.end(); ++i_cur) if (*i_cur) {" << endl;
			out << tab << tab << tab << tab << tab << tab << "t1.exit_mask[i_mask++] = would_exit(*i_cur);" << endl;
			out << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << "bool t1_preemted = false;" << endl;
			out << tab << tab << tab << tab << tab << "for (state::eval_data::eval_list::iterator i_t2 = eval.filtered.begin(); i_t2 != eval.filtered.end();) {" << endl;
			out << tab << tab << tab << tab << tab << tab << "bool erased = false;" << endl;
			out << tab << tab << tab << tab << tab << tab << "if ((t1.exit_mask & i_t2->exit_mask).any()) {" << endl;
			out << tab << tab << tab << tab << tab << tab << tab << "if (i_t2->s->is_descendant(t1.s)) i_t2 = eval.filtered.erase(i_t2), erased = true;" << endl;
			out << tab << tab << tab << tab << tab << tab << tab << "else t1_preemted = true;" << endl;
			out << tab << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << tab << "if (!erased) ++i_t2;" << endl;
			out << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << "if (!t1_preemted) eval.filtered.push_back(t1);" << endl;

			out << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << "return r;" << endl;
			out << tab << tab << tab << "}" << endl;
			out << endl;
			out << tab << tab << tab << "bool enabled = false;" << endl;
			out << tab << tab << tab << "for (state::eval_data::enabled_list::const_iterator i = eval.enabled.begin(); i != eval.enabled.end() && !enabled; ++i) enabled = (i->get() == typeid(*this));" << endl; 
			out << tab << tab << tab << "if (!enabled) return " << empty << ';' << endl;
		}
		else {
			out << tab << tab << tab << "if(!transition_actions<E, S, no_state>::condition(sc.model)) return " << empty << ";" << endl;
		}
		if(opt.debug == "clog") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"" << classname() << ": transition [\" << ename << \"] \" << S::debug_name() << std::endl;" << endl;
		else if(opt.debug == "scxmlgui") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"3 \" << S::debug_name() << \" -> \" << S::debug_name() << std::endl;" << endl;
		out << tab << tab << tab << "transition_actions<E, S, no_state>::enter(sc.model);" << endl;
		if (sc.using_parallel) {
			out << tab << tab << tab << state_t() << "::state_list r;" << endl;
			out << tab << tab << tab << "r.push_back(s);" << endl;
		}

		if(opt.debug == "scxmlgui") out << tab << tab << tab << "if (sc.model.debug) std::clog << \"2 \" << S::debug_name() << \" -> \" << S::debug_name() << std::endl;" << endl;

		if (sc.using_parallel) {
			out << tab << tab << tab << "return r;" << endl;
		} else {
			out << tab << tab << tab << "return s;" << endl;
		}
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
		if (sc.using_parallel) {
			out << tab << tab << "bool would_exit(state* n) { return !!dynamic_cast<S*>(n) && typeid(S) != typeid(*n); }" << endl;
		}
		out << tab << tab << "public:" << endl;

		//todo: for now, all targets must have same parallel parent
		if (sc.using_parallel) out << tab << tab << state_t() << "::state_list operator ()(S *s, " << classname() << "&sc, state::eval_data &eval";
		else out << tab << tab << state_t() << "::state_list operator ()(S *s, " << classname() << "&sc";
		if (opt.debug == "clog") out << ", const char* ename)" << endl;
		else out << ")" << endl;

		out << tab << tab << '{' << endl;

		if (sc.using_parallel) {
			out << tab << tab << tab << "if (eval.filtering) {" << endl;
			out << tab << tab << tab << tab << state_t() << "::state_list r;" << endl;
			out << tab << tab << tab << tab << "if (transition_actions<E, S";
			for (int i = 0; i < sz; ++i) out << ", D" << i;
			out << ">::condition(sc.model)) {" << endl;
			out << tab << tab << tab << tab << tab << "eval.enabled.push_back(typeid(*this));" << endl;
			out << tab << tab << tab << tab << tab << "r.push_back(s);" << endl;

			out << tab << tab << tab << tab << tab << "state::eval_data::eval_item t1;" << endl;
			out << tab << tab << tab << tab << tab << "t1.s = sc.get_state<S>();" << endl;
			out << tab << tab << tab << tab << tab << "t1.cur = s;" << endl;

			out << tab << tab << tab << tab << tab << "size_t i_mask = 0;" << endl;
			out << tab << tab << tab << tab << tab << "for(state::state_list::iterator i_cur = sc.model.cur_state.begin(); i_cur != sc.model.cur_state.end(); ++i_cur) if (*i_cur) {" << endl;
			
			//todo: for now, all targets must have same parallel parent
			out << tab << tab << tab << tab << tab << tab << "t1.exit_mask[i_mask++] = would_exit(*i_cur);" << endl;

			out << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << "bool t1_preemted = false;" << endl;
			out << tab << tab << tab << tab << tab << "for (state::eval_data::eval_list::iterator i_t2 = eval.filtered.begin(); i_t2 != eval.filtered.end();) {" << endl;
			out << tab << tab << tab << tab << tab << tab << "bool erased = false;" << endl;
			out << tab << tab << tab << tab << tab << tab << "if ((t1.exit_mask & i_t2->exit_mask).any()) {" << endl;
			out << tab << tab << tab << tab << tab << tab << tab << "if (i_t2->s->is_descendant(t1.s)) i_t2 = eval.filtered.erase(i_t2), erased = true;" << endl;
			out << tab << tab << tab << tab << tab << tab << tab << "else t1_preemted = true;" << endl;
			out << tab << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << tab << "if (!erased) ++i_t2;" << endl;
			out << tab << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << tab << "if (!t1_preemted) eval.filtered.push_back(t1);" << endl;

			out << tab << tab << tab << tab << "}" << endl;
			out << tab << tab << tab << tab << "return r;" << endl;
			out << tab << tab << tab << "}" << endl;
			out << endl;
			out << tab << tab << tab << "bool enabled = false;" << endl;
			out << tab << tab << tab << "for (state::eval_data::enabled_list::const_iterator i = eval.enabled.begin(); i != eval.enabled.end() && !enabled; ++i) enabled = (i->get() == typeid(*this));" << endl; 
			out << tab << tab << tab << "if (!enabled) return " << empty << ';' << endl;
		}
		else {
			out << tab << tab << tab << "if(!transition_actions<E, S";
			for (int i = 0; i < sz; ++i) out << ", D" << i;
			out << ">::condition(sc.model)) return " << empty << ";" << endl;
		}
		
		if (opt.debug == "clog") {
			out << tab << tab << tab << "if (sc.model.debug) std::clog << \"" << classname() << ": transition [\" << ename << \"] \" << S::debug_name() << \" -> \"";
			for(int i = 0; i < sz; ++i) {
				if(i) out << " << \", \"";
				out << " << D" << i << "::debug_name()";
			}
			out << " << std::endl;" << endl;
		}
		// todo: missing debug output for scxmlgui. Multiple target transitions is not supported by scxmlgui

		for (int i = 0; i < sz; ++i) {
			out << tab << tab << tab << 'D' << i << "*d" << i << " = sc.get_state<D" << i << ">();" << endl;
		}
		out << tab << tab << tab << "s->exit_parallel(sc, s, d0);" << endl;
		if(sc.using_compound) out << tab << tab << tab << "s->exit_to_src(sc.model, S::id());" << endl;
		out << tab << tab << tab << "s->template exit<D0>(sc.model, static_cast<D0*>(nullptr));" << endl;

		out << tab << tab << tab << "transition_actions<E, S";
		for(int i = 0; i < sz; ++i) out << ", D" << i;
		out << ">::enter(sc.model);" << endl;

		out << tab << tab << tab << "d0->template enter<S>(sc.model, static_cast<S*>(nullptr));" << endl;
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

	if (sc.using_parallel) out << tab << tab << "virtual " << retp << " initial(" << classname() << "&, state::eval_data &eval) { return " << empty << "; }" << endl;
	else out << tab << tab << "virtual " << retp << " initial(" << classname() << "&) { return " << empty << "; }" << endl;

	// lca calculation
	out << tab << tab << "public:" << endl;
	out << tab << tab << "typedef P parent_t;" << endl;
	out << tab << tab << "// LCA calculation" << endl;
	out << tab << tab << "template<class T> void enter(data_model&, " << state_composite_t() << "*) {}" << endl;

	out << tab << tab << "template<class T> void enter(data_model &m, ...) { P::template enter<T>(m, static_cast<T*>(nullptr));";
	if(opt.debug == "clog") out << " if (m.debug) std::clog << \"" << classname() << ": enter \" << C::debug_name() << std::endl;";
	else if(opt.debug == "scxmlgui") out << " if (m.debug) std::clog << \"1 \" << C::debug_name() << std::endl;";
	out << " state_actions<C>::enter(m); }" << endl;

	out << tab << tab << "template<class T> void exit(data_model&, " << state_composite_t() << "*) {}" << endl;

	out << tab << tab << "template<class T> void exit(data_model &m, ...) {";
	if(opt.debug == "clog") out << " if (m.debug) std::clog << \"" << classname() << ": exit \" << C::debug_name() << std::endl;";
	else if(opt.debug == "scxmlgui") out << " if (m.debug) std::clog << \"0 \" << C::debug_name() << std::endl;";
	out << " state_actions<C>::exit(m); P::template exit<T>(m, static_cast<T*>(nullptr)); }" << endl;

	if(sc.using_compound) {
		out << tab << tab << "virtual void exit_to_src(data_model &m, const void *sti) { if (C::id() == sti) return;";
		if(opt.debug == "clog") out << " if (m.debug) std::clog << \"" << classname() << ": exit \" << C::debug_name() << std::endl;";
		else if(opt.debug == "scxmlgui") out << " if (m.debug) std::clog << \"0 \" << C::debug_name() << std::endl;";
		out << " state_actions<C>::exit(m); P::exit_to_src(m, sti); }" << endl;
	}
	if(sc.using_parallel) {
		out << tab << tab << "virtual bool is_descendant(state *s) { return !!dynamic_cast<C*>(s) && (typeid(*s) != typeid(C)); }" << std::endl;
	}
        out << tab << tab << "static const void* id() { static const struct{} _id; return &_id; }" << endl;
	out << tab << tab << "bool in(const void *si) { return (si == id() || P::in(si)); }" << endl;

	out << tab << "};" << endl;
	out << endl;
}

void cpp_output::gen_state_final_base()
{
	if (!sc.using_final) return;

	out << tab << "template<class C, class P> class " << state_final_t() << " : public " << state_composite_t() << "<C, P>" << endl;
	out << tab << '{' << endl;

	if (sc.using_parallel) {
		out << tab << tab << "virtual void exit_to_src(data_model &m, const void *sti) { if (C::id() == sti) return;";
		if(opt.debug == "clog") out << " if (m.debug) std::clog << \"" << classname() << ": exit \" << C::debug_name() << std::endl;";
		else if(opt.debug == "scxmlgui") out << " if (m.debug) std::clog << \"0 \" << C::debug_name() << std::endl;";
		out << " P::parallel_exit_final(m); state_actions<C>::exit(m); P::exit_to_src(m, sti); }" << endl;
	}

	out << tab << "};" << endl << endl;
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

		// todo: external transitions where src is child of dst or dst is child of src and src/dst is first child of parallel exits/enters parallel, but
		// parallel's other childs are not exited/entered. This can be tested with test/lcca_parallel.scxml, with e0 changed to type external

		// force exit when transition dst is child of parallel - parallel child has been exited and parallel child will be entered. So exit from lca to lcca
		out << tab << tab << "template<class T> void exit(data_model& m, C*) { " << state_composite_t() << "<C, P>::template exit<T>(m); }" << endl;
		out << tab << tab << "template<class T> void exit(data_model& m, state*) { " << state_composite_t() << "<C, P>::template exit<T>(m, static_cast<T*>(nullptr)); }" << endl;
		out << tab << tab << "template<class T> void exit(data_model& m) { " << state_composite_t() << "<C, P>::template exit<T>(m); }" << endl;
		out << endl;

		// force enter when transition src is child of parallel - enter from lcca to lca
		out << tab << tab << "template<class T> void enter(data_model& m, C*) { " << state_composite_t() << "<C, P>::template enter<T>(m); }" << endl;
		out << tab << tab << "template<class T> void enter(data_model& m, state*) { " << state_composite_t() << "<C, P>::template enter<T>(m, static_cast<T*>(nullptr)); }" << endl;
		out << tab << tab << "template<class T> void enter(data_model& m) { " << state_composite_t() << "<C, P>::template enter<T>(m); }" << endl;
		out << endl;

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
		out << tab << tab << tab << state_t() << "::" << ret << " r = P::template enter_parallel<S>(sc, d, static_cast<S*>(nullptr));" << endl;
		out << tab << tab << tab << "r.reserve(" << children << ");" << endl;

		// use composite instead S to prevent forced parallel enter on initial states. See state_enter generation for more details
		for(int c = 1; c < children; ++c) out << tab << tab << tab << 'd' << c << "->template enter<composite<C, typename C::parent_t> >(sc.model), r.push_back(d" << c << ");" << endl;

		out << tab << tab << tab << "return r;" << endl;
		out << tab << tab << '}' << endl;
		out << endl;

		for(int c = 0; c < children; ++c) {
			out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C" << c << "*, C" << c << "*) { return " << state_t() << "::" << ret << "(); }" << endl;
			out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << " &sc, C" << c << " *d, " << state_t() << " *s)" << endl;
			out << tab << tab << '{' << endl;
			out << tab << tab << tab << "// parallel state entered with C" << c << " or child of as target" << endl;
			out << tab << tab << tab << "return enter_parallel<S>(sc, d, s";
			for(int n = 0; n < children; ++n) {
				if (n == c) continue;
				out << ", " << "sc.get_state<C" << n << ">()";
			}
			out << ");" << endl;
			out << tab << tab << "}" << endl;
			out << tab << tab << endl;

			//todo to continue
		}

		// other cases, eg parallel state as target
		out << tab << tab << "template<class S> " << state_t() << "::" << ret << " enter_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) { return state::state_list(); }" << endl;

		out << endl;

		// parallel exit
		out << tab << tab << "bool parallel_parent(const std::type_info& pti) { return typeid(C) == pti; }" << endl;
		for (int n = 0; n < children; ++n) {
			out << tab << tab << "void exit_parallel(" << classname() << " &sc, C" << n << "*, C" << n << "*) {}" << endl;
		}
		out << tab << tab << "void exit_parallel(" << classname() << " &sc, state *s, state *d)" << endl;
		out << tab << tab << '{' << endl;
		out << tab << tab << tab << "// parallel state exited from C or child" << endl;
		out << tab << tab << tab << "for(" << classname() << "::" << state_t() << "::" << ret << "::iterator i = sc.model.cur_state.begin(); (i != sc.model.cur_state.end()) && *i; ++i) {" << endl;
		out << tab << tab << tab << tab << "if(typeid(*this) == typeid(**i)) continue;" << endl;
		out << tab << tab << tab << tab << "if(!(*i)->parallel_parent(typeid(C))) continue;" << endl;
		out << tab << tab << tab << tab << "(*i)->exit_to_src(sc.model, C::id());" << endl;
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

	constructs.push_back(make_pair("user", "um"));
        if(!opt.cpp14) {
           constructs.push_back(make_pair("event_queue_int", "allocator"));
           constructs.push_back(make_pair("event_queue_ext", "allocator"));
        }

	if(!opt.bare_metal) {
		out << tab << tab << "const std::string _sessionid;" << endl;
		constructs.push_back(make_pair("_sessionid", "std::to_string(reinterpret_cast<long long unsigned int>(this))"));
		out << tab << tab << "const std::string _name;" << endl;
		constructs.push_back(make_pair("_name", "\"" + sc.sc().name + "\""));
		if(opt.string_events) {
			// _event
			out << tab << tab << "struct EventStruct {" << endl;
			out << tab << tab << tab << "std::string name;" << endl;
			out << tab << tab << tab << any_ns << "any data;" << endl;
			out << tab << tab << tab << "void clear () { name.clear(); data = " << any_ns << "any(); };" << endl;
			out << tab << tab << "} _event;" << endl;
		}
	}
	for (scxml_parser::data_list::const_iterator i_data = datamodel.begin(); i_data != datamodel.end(); ++i_data) {
		string id = i_data->get()->id;
		const boost::optional<string> expr_opt = i_data->get()->expr;

		string type = "int"; // default type
                const auto sep = id.find_first_of(" ");
                if (sep != string::npos) {
                   // use first item as type
                   type = id.substr(0, sep);
                   id = id.substr(sep + 1);
                }
		out << tab << tab << type << ' ' << id << ';' << endl;
		if (expr_opt) {
			constructs.push_back(make_pair(id, *expr_opt));
		}
	}
        if(opt.cpp14) out << tab << tab << "data_model(user_model* um)" << endl;
        else out << tab << tab << "data_model(user_model* um, std::pmr::memory_resource* allocator)" << endl;
	char delim(':');

	for (pair_vect::const_iterator i_construct = constructs.begin(); i_construct != constructs.end(); ++i_construct) {
		out << tab << tab << delim << ' ' << i_construct->first << '(' << i_construct->second << ")" << endl;
		delim = ',';
	}	
	out << tab << tab << "{}" << endl;
}

scxml_parser::state_list cpp_output::states(const std::string &type)
{

	scxml_parser::state_list found_states;
	const scxml_parser::state_list &states = sc.sc().states;
	for (scxml_parser::state_list::const_iterator i_state = states.begin(); i_state != states.end(); ++i_state) {
		if (i_state->get()->type && *i_state->get()->type == type) {
			found_states.push_back(*i_state);
		}
	}
	return found_states;
}

std::set<std::string> cpp_output::get_event_names() const
{
	// collect events
	using namespace boost::algorithm;

	const scxml_parser::state_list &states = sc.sc().states;
	std::vector<string> events;

	for (scxml_parser::state_list::const_iterator i_state = states.begin(); i_state != states.end(); ++i_state) {

		// Final state events
		if (i_state->get()->type && *i_state->get()->type == "final") {
			if (!i_state->get()->parent) continue;
			string event = "done.state." + i_state->get()->parent->id;
			events.push_back(event);
		}

		// If parallel have a final child, the parallel state must have a done event also
		if (sc.using_final && sc.using_parallel && i_state->get()->type && *i_state->get()->type == "parallel") {
			string event = "done.state." + i_state->get()->id;
			events.push_back(event);
		}

		// Transition events
		for (scxml_parser::transition_list::const_iterator i_trans = i_state->get()->transitions.begin(); i_trans != i_state->get()->transitions.end(); ++i_trans) {
			for(scxml_parser::slist::const_iterator i_event = i_trans->get()->event.begin(); i_event != i_trans->get()->event.end(); ++i_event) {
				events.push_back(*i_event);
			}
		}

		// Raise events
		scxml_parser::plist<scxml_parser::action> actions;
		copy(i_state->get()->entry_actions.begin(), i_state->get()->entry_actions.end(), back_inserter(actions));
		copy(i_state->get()->exit_actions.begin(), i_state->get()->exit_actions.end(), back_inserter(actions));
		copy(i_state->get()->initial.actions.begin(), i_state->get()->initial.actions.end(), back_inserter(actions));
		for (scxml_parser::transition_list::const_iterator i_trans = i_state->get()->transitions.begin(); i_trans != i_state->get()->transitions.end(); ++i_trans) {
			copy(i_trans->get()->actions.begin(), i_trans->get()->actions.end(), back_inserter(actions));
		}
		for (scxml_parser::plist<scxml_parser::action>::const_iterator i = actions.begin(); i != actions.end(); ++i) {
			scxml_parser::action &a = *i->get();
			if (a.type == "raise" && ! opt.bare_metal) {
				events.push_back(a.attr["event"]);
			}
		}

	}

	// pass through set, to filter out duplicates
	set<string> event_set;
	for (vector<string>::const_iterator i_event = events.begin(); i_event != events.end(); ++ i_event) {
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
	return event_set;
}

void cpp_output::gen_model_base_finals()
{
	if (!(sc.using_final && sc.using_parallel)) return;
	scxml_parser::state_list parallel_states = states("parallel");

	out << tab << tab << "struct final_states {" << endl;
	for (scxml_parser::state_list::const_iterator i_state = parallel_states.begin(); i_state != parallel_states.end(); ++i_state) {
		out << tab << tab << tab << "int " << i_state->get()->id << ';' << endl;
	}
	out << tab << tab << tab << "final_states() : ";
	for (scxml_parser::state_list::const_iterator i_state = parallel_states.begin(); i_state != parallel_states.end(); ++i_state) {
		if (i_state != parallel_states.begin()) out << ", ";
		out << i_state->get()->id << "(0)";
	}
	out << " {}" << endl;
	out << tab << tab << "} finals;" << endl;
}

void cpp_output::gen_model_base()
{
	out << tab << "struct data_model" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "user_model *user;" << endl;
	if (!opt.bare_metal) {
                if (opt.cpp14) {
                      out << tab << tab << "std::deque<event> event_queue_int;" << endl;
                      out << tab << tab << "std::deque<event> event_queue_ext;" << endl;
                }
                else {
                      out << tab << tab << "std::pmr::deque<event> event_queue_int;" << endl;
                      out << tab << tab << "std::pmr::deque<event> event_queue_ext;" << endl;
                }
        }
	if(sc.using_parallel) {
		out << tab << tab << state_t() << "::state_list cur_state;" << endl;
		out << tab << tab << "template <class S> bool In()" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "for (state::state_list::const_iterator i = cur_state.begin(); i != cur_state.end(); ++i) if ((*i)->in(S::id())) return true;" << endl;
		out << tab << tab << tab << "return false;" << endl;
		out << tab << tab << "}" << endl;
	}
	else {
		out << tab << tab << state_t() << " *cur_state;" << endl;
		out << tab << tab << "template <class S> bool In() { return cur_state->in(S::id()); }" << endl;
	}
	if (opt.debug == "clog" || opt.debug == "scxmlgui") out << tab << tab << "bool debug = true;" << endl;
	gen_model_base_data();
	gen_model_base_finals();
	if (opt.thread_safe) {
		out << tab << tab << "void push_event(event&& e)" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "std::unique_lock<std::mutex> lock(queue_mutex);" << endl;
		out << tab << tab << tab << "event_queue_ext.emplace_back(std::move(e));" << endl;
		out << tab << tab << tab << "lock.unlock();" << endl;
		out << tab << tab << tab << "cv.notify_one();" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << "void push_event_front(event&& e)" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "std::unique_lock<std::mutex> lock(queue_mutex);" << endl;
		out << tab << tab << tab << "event_queue_ext.emplace_front(std::move(e));" << endl;
		out << tab << tab << tab << "lock.unlock();" << endl;
		out << tab << tab << tab << "cv.notify_one();" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << optional_ns << "optional<event> pop_event()" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "std::lock_guard<std::mutex> lock(queue_mutex);" << endl;
		out << tab << tab << tab << "if (event_queue_int.empty()) return std::nullopt;" << endl;
		out << tab << tab << tab << "event e = event_queue_int.front();" << endl;
		out << tab << tab << tab << "event_queue_int.pop_front();" << endl;
		out << tab << tab << tab << "return e;" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << optional_ns << "optional<event> wait_event()" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "std::unique_lock<std::mutex> lock(queue_mutex);" << endl;
		out << tab << tab << tab << "cv.wait(lock, [this]{ return !event_queue_ext.empty() || cancel_wait_; });" << endl;
		out << tab << tab << tab << "if (event_queue_ext.empty()) return std::nullopt;" << endl;
		out << tab << tab << tab << "event e = event_queue_ext.front();" << endl;
		out << tab << tab << tab << "event_queue_ext.pop_front();" << endl;
		out << tab << tab << tab << "return e;" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << "void cancel_wait()" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "cancel_wait_ = true;" << endl;
		out << tab << tab << tab << "cv.notify_all();" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << "void uncancel() { cancel_wait_ = false; }" << endl;
		out << tab << tab << "bool is_canceled() const { return cancel_wait_; }" << endl;
		out << tab << tab << "std::condition_variable cv;" << endl;
		out << tab << tab << "std::mutex queue_mutex;" << endl;
		out << tab << tab << "bool cancel_wait_ = false;" << endl;
	}
	out << tab << "} model;" << endl;
	out << endl;
}

void cpp_output::gen_state_base()
{
	// state actions class
	out << tab << "class " << state_t() << endl;
	out << tab << "{" << endl;
	out << tab << tab << "public:" << endl;
	out << tab << tab << "virtual ~" << state_t() << "(){}" << endl;

	if (sc.using_parallel) {
		out << tab << tab << "struct state_list : public std::vector<" << state_t() << "*>" << endl;
		out << tab << tab << "{" << endl;
		out << tab << tab << tab << "operator bool() { return !empty(); }" << endl;
		out << tab << tab << "};" << endl;
 		out << tab << tab << "struct eval_data" << std::endl;
 		out << tab << tab << "{" << std::endl;
		out << tab << tab << tab << "struct eval_item" << std::endl;
		out << tab << tab << tab << "{" << std::endl;
 		out << tab << tab << tab << tab << "state *s, *cur;" << std::endl;
		// todo: figure out bitset size
 		out << tab << tab << tab << tab << "std::bitset<32> exit_mask;" << std::endl;
		out << tab << tab << tab << "};" << std::endl;
 		out << tab << tab << tab << "typedef std::vector<eval_item> eval_list;" << std::endl;
 		out << tab << tab << tab << "eval_list filtered;" << std::endl;
 		out << tab << tab << tab << "typedef std::vector<std::reference_wrapper<const std::type_info> > enabled_list;" << std::endl;
 		out << tab << tab << tab << "enabled_list enabled;" << std::endl;
 		out << tab << tab << tab << "bool filtering;" << std::endl;
 		out << tab << tab << "};" << std::endl;
	}

	// events
	// generate events for each token, eg:
	// ev.a.b.c =>
	// ev        call '*' - if there
	// ev_a      call ev
	// ev_a_b    call ev_a
	// ev_a_b_c  call ev_a_b

	bool use_base_event = false;
	auto event_set =  get_event_names();

	for(auto evName : event_set) {
		if(evName == "*"){
			use_base_event = true;
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
		if (sc.using_parallel) out << ", eval_data &eval";
		out << ") { return ";
		string eparams = "(sc)";
		if (sc.using_parallel) eparams = "(sc, eval)";
		if (parent.size()) out << "event_" << parent << eparams;
		else if (event.size() && use_base_event) out << "event" << eparams;
		else out << empty;
		out << "; }" << endl;
	}
	string uiparams = '(' + classname() + "&)";
	if (sc.using_parallel) uiparams = '(' + classname() + "&, eval_data&)";
	out << tab << tab << "virtual " << ret << " unconditional" << uiparams << " { return " << empty << "; }" << endl;
	out << tab << tab << "virtual " << ret << " initial" << uiparams << " { return " << empty << "; }" << endl;

	out << endl;

	out << tab << tab << "template<class T> void enter(data_model&, ...) {}" << endl;
	out << tab << tab << "template<class T> void exit(data_model&, ...) {}" << endl;
	if(sc.using_compound) out << tab << tab << "virtual void exit_to_src(data_model&, const void*) {}" << endl;
	if(sc.using_parallel) {
		out << tab << tab << "template<class S> " << retp << " enter_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) { return state_list(); }" << endl;
		out << tab << tab << "virtual void exit_parallel(" << classname() << "&, " << state_t() << "*, " << state_t() << "*) {}" << endl;
		out << tab << tab << "virtual bool parallel_parent(const std::type_info&) { return false; }" << endl;
		if (sc.using_final) {
			out << tab << tab << "void parallel_enter_final(data_model &m) {}" << endl;
			out << tab << tab << "void parallel_exit_final(data_model &m) {}" << endl;
		}
		out << tab << tab << "virtual bool is_descendant(state*) = 0;" << endl;
	}
	out << tab << tab << "virtual bool in(const void*) { return false; }" << endl;

	// removed - this may require delete() wihich is'nt available in some embedded setups
	//out << tab << tab << "virtual ~" << state_t() << "() {}" << endl;

	out << tab << "};" << endl;
	out << endl;

	if (sc.using_parallel) out << tab << "typedef " << retp << " (" << state_t() << "::*event)(" << classname() << "&, state::eval_data&);" << endl;
	else out << tab << "typedef " << retp << " (" << state_t() << "::*event)(" << classname() << "&);" << endl;
	out << endl;
	if(opt.string_events) {
		out << tab << "std::unordered_map<std::string, event> event_map;" << endl;
    if(opt.thread_safe) {
      out << tab << "mutable std::mutex event_map_mutex;" << endl;
    }
	}
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
		scxml_parser::state_list state_children = children(state);
		for(scxml_parser::state_list::const_iterator i = state_children.begin(); i != state_children.end(); ++i) {
			out << tab << "struct state_" << (*i)->id << ';' << endl;
		}
	}
	out << tab << "struct " << state_classname << " : public ";
	if(parallel_state) out << state_parallel_t();
	else if(final_state) out << state_final_t();
	else out << state_composite_t();
	out << '<' << state_classname << ", " << parent;
	if(parallel_state) {
		scxml_parser::state_list state_children = children(state);
		for(scxml_parser::state_list::const_iterator i = state_children.begin(); i != state_children.end(); ++i) {
			out << ", state_" << (*i)->id;
		}
	}
	out << '>' << endl;

	out << tab << "{" << endl;

	if ((opt.debug == "scxmlgui") || opt.debug == "clog") {
		// todo: use non-trimmed version of state.id
		out << tab << tab << "static std::string debug_name() { return \"" << state.id << "\"; }" << endl;
	}

	if(state.initial.target.size()) {
		const int sz = state.initial.target.size();
		if (sc.using_parallel) out << tab << tab << ret << " initial" << "(" << classname() << " &sc, eval_data &eval) { return transition";
		else out << tab << tab << ret << " initial" << "(" << classname() << " &sc) { return transition";
		if (sz > 1) out << sz;
		out << "<&state::initial, " << state_classname;
		for (int i = 0; i < sz; ++i) out << ", state_" << state.initial.target[i];
		string tparams = "(this, sc";
		if (sc.using_parallel) tparams = "(this, sc, eval";
		if (opt.debug == "clog") tparams += ", __func__";
		tparams += ")";
		out << ", internal>()" << tparams << "; }" << endl;
	}

	if (final_state) {
		if (parent == "scxml") {
			// todo: how to handle final in scxml:
			// When the state machine reaches the <final> child of an <scxml> element, it must terminate
			cerr << "warning: root final states is currently not supported." << endl;
		}
		else {
			if (!opt.thread_safe) out << tab << tab << "template<class T> void enter(data_model &m, ...) { final::template enter<T>(m, static_cast<T*>(nullptr)); m.event_queue_int.emplace_back(&state::event_done_" << parent << "); ";
			else out << tab << tab << "template<class T> void enter(data_model &m, ...) { final::template enter<T>(m, static_cast<T*>(nullptr)); m.push_event(&state::event_done_" << parent << "); ";
			if (sc.using_parallel) out << "parallel_enter_final(m); }" << endl;
			else out << '}' << endl;
		}
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
			const bool has_target = !t->get()->target.empty();

			if(has_target) {
				target_classname = "state_" + t->get()->target.front(); //todo handle multiple targets
			}

			if (first) {
				string s = ret + ' ' + event + "(" + classname() + " &sc";
				if (sc.using_parallel) s += ", eval_data &eval";
				s += ") { ";
				if (multiple) s += ret + " s; ";
				s += "return ";
				out << tab << tab << s;
				indent = string(s.size() - 3, ' ');
			}
			else out << tab << tab << indent << "|| ";
			if (multiple) out << "(s = ";
			string tparams = "(this, sc";
			if (sc.using_parallel) tparams = "(this, sc, eval";
			if (opt.debug == "clog") tparams += ", __func__";
			tparams += ")";
			if (has_target) {
				// normal transition
				string type_str;
				if (internal) type_str = ", internal";
				out << "transition<&state::" << event << ", " << state_classname << ", " << target_classname << type_str << ">()" << tparams;
			}
			else {
				// transition without target
				out << "transition<&state::" << event << ", " << state_classname << ">()" << tparams;
			}

			string ancestor;
			if (use_ancestor) {
				string pparams = "(sc)";
				if (sc.using_parallel) pparams = "(sc, eval)";
				ancestor = " || (s = parent_t::" + event + pparams + ")";
			}

			if (multiple && last) out << ")" << ancestor << ", s";
			else if (multiple) out << ")" << endl;
			if (last) out << "; }" << endl;
		}
	}

	// parallel enter/exit final
	if (parallel_state && sc.using_final) {
		scxml_parser::state_list state_children = children(state);
		if (!opt.thread_safe) out << tab << tab << "void parallel_enter_final(data_model &m) { if (++m.finals." << state.id << " == " << state_children.size() << ") m.event_queue_int.emplace_back(&" << classname() << "::" << state_t() << "::event_done_state_" << state.id << "), parent_t::parallel_enter_final(m); }" << endl;
		else out << tab << tab << "void parallel_enter_final(data_model &m) { if (++m.finals." << state.id << " == " << state_children.size() << ") m.push_event(&" << classname() << "::" << state_t() << "::event_done_state_" << state.id << "), parent_t::parallel_enter_final(m); }" << endl;
		out << tab << tab << "void parallel_exit_final(data_model &m) { if (--m.finals." << state.id << " == " << state_children.size()-1 << ") parent_t::parallel_exit_final(m); }" << endl;
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
	gen_state_actions_base();
	gen_state_composite_base();
	gen_state_final_base();
	gen_state_parallel_base();
	gen_transition_base();

	// dispatch
	if(sc.using_parallel) {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;

		out << tab << tab << "bool cont = false;" << endl;
		out << tab << tab << "state::eval_data eval;" << endl;
		out << tab << tab << "eval.filtering = true;" << endl;
		out << tab << tab << "for(state::state_list::iterator i_cur = model.cur_state.begin(); i_cur != model.cur_state.end(); ++i_cur) if(*i_cur) {" << endl;
		out << tab << tab << tab << "((*i_cur)->*e)(*this, eval);" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << "eval.filtering = false;" << endl;
		out << endl;

			
		out << tab << tab << "state::state_list::iterator i_cur = model.cur_state.begin();" << endl;
		out << tab << tab << "for (state::eval_data::eval_list::iterator i_filtered = eval.filtered.begin(); i_filtered != eval.filtered.end(); ++i_filtered) {" << endl;
		out << tab << tab << tab << "while(i_cur != model.cur_state.end()) if(*i_cur) {" << endl;
		out << tab << tab << tab << tab << "if(*i_cur == i_filtered->cur) {" << endl;
		out << tab << tab << tab << tab << tab << "state::state_list r = ((*i_cur)->*e)(*this, eval);" << endl;
		out << tab << tab << tab << tab << tab << "cont = true;" << endl;
		out << tab << tab << tab << tab << tab << "std::vector<state*>::const_iterator i_new = r.begin();" << endl;
		out << tab << tab << tab << tab << tab << "*i_cur = *i_new++;" << endl;
		out << tab << tab << tab << tab << tab << "for(; i_new != r.end(); ++i_new) i_cur = model.cur_state.insert(++i_cur, *i_new);" << endl;
		out << tab << tab << tab << tab << tab << "++i_cur;" << endl;
		out << tab << tab << tab << tab << tab << "break;" << endl;
		out << tab << tab << tab << tab << "}" << endl;
		out << tab << tab << tab << tab << "++i_cur;" << endl;
		out << tab << tab << tab << "}" << endl;
		out << tab << tab << "}" << endl;
		out << tab << tab << "model.cur_state.erase(std::remove(model.cur_state.begin(), model.cur_state.end(), nullptr), model.cur_state.end());" << endl;
		out << tab << tab << "return cont;" << endl;

		out << tab << '}' << endl;
	}
	else {
		out << tab << "private: bool dispatch_event(event e)" << endl;
		out << tab << '{' << endl;
		out << tab << tab << "state *next_state;" << endl;
		out << tab << tab << "if ((next_state = (model.cur_state->*e)(*this))) model.cur_state = next_state;" << endl;
		out << tab << tab << "return !!next_state;" << endl;
		out << tab << '}' << endl;
	}
	out << endl;
	out << tab << "public: void dispatch_int(event e = &state::unconditional)" << endl;
	out << tab << "{" << endl;
	out << tab << tab << "bool cont = dispatch_event(e) || dispatch_event(&state::unconditional);" << endl;
	out << tab << tab << "while (cont) {" << endl;
	out << tab << tab << tab << "if ((cont = dispatch_event(&state::initial)));" << endl;
	out << tab << tab << tab << "else if ((cont = dispatch_event(&state::unconditional)));" << endl;
	if(!opt.bare_metal) {
		if (!opt.thread_safe) out << tab << tab << tab << "else if (model.event_queue_int.size()) cont = dispatch_event(model.event_queue_int.front()), model.event_queue_int.pop_front(), cont |= !model.event_queue_int.empty();" << endl;
		else out << tab << tab << tab << "else if (auto next_e = model.pop_event()) cont = dispatch_event(*next_e), cont |= !model.event_queue_int.empty();" << endl;
	}
	out << tab << tab << tab << "else break;" << endl;
	out << tab << tab << "}" << endl;
	out << tab << "}" << endl;
        out << endl;

	out << tab << "void dispatch_ext()" << endl;
        out << tab << "{" << endl;
        out << tab << tab << "while (!model.event_queue_ext.empty()) {" << endl;
        out << tab << tab << tab << "dispatch_int(model.event_queue_ext.front());" << endl;
        out << tab << tab << tab << "model.event_queue_ext.pop_front();" << endl;
        out << tab << tab << "}" << endl;
        out << tab << "}" << endl;
        out << endl;
	out << tab << "void dispatch(event e = &state::unconditional)" << endl;
        out << tab << "{" << endl;
        out << tab << tab << "model.event_queue_ext.push_back(e);" << endl;
        out << tab << tab << "if (model.event_queue_ext.size() == 1) do dispatch_ext(); while (!model.event_queue_ext.empty());" << endl;
        out << tab << "}" << endl;
        out << endl;

	// dispatch by name
	if(opt.string_events) {
		out << tab << "public: void dispatch(const std::string& ev_name, " << any_ns << "any data = " << any_ns << "any())" << endl;
		out << tab << "{" << endl;
		if(opt.thread_safe) {
			out << tab << tab << "event_map_mutex.lock();" << endl;
		}
		out << tab << tab << "model._event.name = ev_name;" << endl;
		out << tab << tab << "model._event.data = data;" << endl;

		out << tab << tab << "auto event_it = event_map.find(ev_name);" << endl;
		out << tab << tab << "if(event_it != event_map.end()) {" << endl;
		if(opt.thread_safe) {
			out << tab << tab << tab << "event_map_mutex.unlock();" << endl;
		}
		out << tab << tab << tab << "dispatch(event_it->second);" << endl;
		out << tab << tab << tab << "return;" << endl;
		out << tab << tab << '}' << endl;
		if(opt.thread_safe) {
			out << tab << tab << "event_map_mutex.unlock();" << endl;
		}
		out << tab << tab << "std::string name(ev_name);" << endl;
		out << tab << tab << "do {" << endl;
		out << tab << tab << tab << "// name not found -> remove the last part and try again" << endl;
		out << tab << tab << tab << "auto last_dot = name.rfind('.');" << endl;
		out << tab << tab << tab << "if(last_dot == std::string::npos) break;" << endl;
		out << tab << tab << tab << "name.erase(last_dot);" << endl;
		if(opt.thread_safe) {
			out << tab << tab << tab << "event_map_mutex.lock();" << endl;
		}
		out << tab << tab << tab << "event_it = event_map.find(name);" << endl;
		out << tab << tab << tab << "if(event_it != event_map.end()) {" << endl;
		out << tab << tab << tab << tab << "auto ev = event_it->second;" << endl;
		out << tab << tab << tab << tab << "event_map[ev_name] = ev; // cache the event name" << endl;
		if(opt.thread_safe) {
			out << tab << tab << tab << tab << "event_map_mutex.unlock();" << endl;
		}
		out << tab << tab << tab << tab << "dispatch(ev);" << endl;
		out << tab << tab << tab << tab << "return;" << endl;
		out << tab << tab << tab << '}' << endl;
		if(opt.thread_safe) {
			out << tab << tab << tab << "event_map_mutex.unlock();" << endl;
		}
		out << tab << tab << "} while (! name.empty());" << endl;
		out << tab << tab << "// signal name not found or empty" << endl;
		if(opt.thread_safe) {
			out << tab << tab << "event_map_mutex.lock();" << endl;
		}
		out << tab << tab << "event_map[ev_name] = &state::unconditional; // cache the event name" << endl;
		if(opt.thread_safe) {
			out << tab << tab << "event_map_mutex.unlock();" << endl;
		}
		out << tab << tab << "dispatch(&state::unconditional);" << endl;
		out << tab << "}" << endl;
	}

	if (opt.thread_safe) {
		out << tab << "void dispatch_loop()" << endl;
		out << tab << "{" << endl;
		out << tab << tab << "while (!model.is_canceled()) {" << endl;
		out << tab << tab << tab << "auto e = model.wait_event();" << endl;
		out << tab << tab << tab << "if (e) dispatch(*e);" << endl;
		out << tab << tab << "}" << endl;
		out << tab << "}" << endl;
		out << endl;
	}
	gen_model_base();

	// constructor
	auto event_names = get_event_names();

	if (opt.cpp14) out << tab << classname() << "(user_model *user = nullptr)" << endl;
	else out << tab << classname() << "(user_model *user = nullptr, std::pmr::memory_resource* allocator = std::pmr::get_default_resource())" << endl;
	out << tab << ": ";
	if(opt.string_events && ! event_names.empty()) {
		out << "event_map{";
		const char* delim("");
		for(auto ev_name : event_names) {
			out << delim << "{\"" << ev_name << "\", &" << classname() << "::state::" << event_name(ev_name) << "}";
			delim = ", ";
		}
		out << "}" << endl;
		out << tab << ", ";
	}
	if (opt.cpp14) out << "model(user)" << endl;
	else out << "model(user, allocator)" << endl;

	out << tab << "{" << endl;
	if(sc.using_parallel) out << tab << tab << "model.cur_state.push_back(get_state<scxml>());" << endl;
	else out << tab << tab << "model.cur_state = get_state<scxml>();" << endl;
        out << tab << tab << "model.event_queue_ext.push_back(&state::initial);" << endl;
	out << tab << "}" << endl;

	// init
	out << endl;
	out << tab << "void init() { dispatch_ext(); }" << endl;
	out << endl;

	//scxml base
	out << tab << "struct scxml : public composite<scxml, state>" << endl;
	out << tab << "{" << endl;

	if ((opt.debug == "scxmlgui") || opt.debug == "clog") {
		// todo: use non-trimmed version of state.id
		out << tab << tab << "static std::string debug_name() { return \"scxml\"; }" << endl;
	}

	const int sz = sc.sc().initial.target.size();
	if (sc.using_parallel) out << tab << tab << ret << " initial(" << classname() << "&sc, eval_data &eval) { return transition";
	else out << tab << tab << ret << " initial(" << classname() << "&sc) { return transition";
	if(sz > 1) out << sz;
	out << "<&state::initial, scxml";
	for(int i = 0; i < sz; ++i) out << ", state_" << sc.sc().initial.target[i];
	string tparams = "(this, sc";
	if (sc.using_parallel) tparams = "(this, sc, eval";
	if (opt.debug == "clog") tparams += ", __func__";
	tparams += ")";
	out << ", internal>()" << tparams << "; }" << endl;
	out << tab << "};" << endl;
	out << endl;
	
	// states
	for (scxml_parser::state_list::const_iterator s = states.begin(); s != states.end(); ++s) {
		gen_state(*s->get());
	}

	// get_state
	out << tab << "template<class T> T* get_state()" << endl;
	out << tab << '{' << endl;
	out << tab << tab << "static T t;" << endl;
	out << tab << tab << "return &t;" << endl;
	out << tab << "}" << endl;

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
	//out << tab << tab << expr << endl;
	out << tab << tab << regex_replace(expr, regex("([^\\\\]\\s*[\\r\\n])\\s*"), string("$1") + tab + tab) << endl;
}

void cpp_output::gen_action_part_raise(scxml_parser::action &a)
{
	const string ev = a.attr["event"];

	//out << tab << tab << "// " << a.type << " event=" << ev << endl;
	if (!opt.thread_safe) out << tab << tab << "event_queue_int.emplace_back(&" << classname() << "::state::" <<  event_name(ev)  << ");" << endl;
	else out << tab << tab << "push_event(&" << classname() << "::state::" <<  event_name(ev) << ");" << endl;
}

void cpp_output::gen_action_part(scxml_parser::action &a)
{
	if(a.type == "raise") {
		if(opt.bare_metal) cerr << "warning: raise ignored for bare-metal" << endl;
		else gen_action_part_raise(a);
	}
	else if(a.type == "log") gen_action_part_log(a);
	else if(a.type == "assign") gen_action_part_assign(a);
	else if(a.type == "script") gen_action_part_script(a);
	else {
		out << tab << tab << "// warning: unknown action type '" << a.type << "'" << endl;
		cerr << "warning: unknown action type '" << a.type << "'" << endl;
	}
}

void cpp_output::gen_condition_part_In(string cond)
{
	// convert 'In' to C++ syntax
	string s;
	size_t n;
        s = "('", n = 0; 
	while ((n = cond.find(s, n)) != std::string::npos) cond.replace(n, s.size(), "<state_");
	s = "')", n = 0; 
	while ((n = cond.find(s, n)) != std::string::npos) cond.replace(n, s.size(), ">()");
	out << tab << tab << "return " << cond << ';' << endl;
}

void cpp_output::gen_condition_part(string cond)
{
	if (cond.find("In(") == 0) gen_condition_part_In(cond);
	else out << tab << tab << "return " << cond << ';' << endl;
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
				gen_condition_part(*itrans->get()->condition);
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
	map<string, string> changes;

	// remove '.*' postfix in events
	// replace '-' with '_' in event names
	for (scxml_parser::state_list::const_iterator istate = states.begin(); istate != states.end(); ++istate) {
		for (scxml_parser::transition_list::const_iterator itrans = istate->get()->transitions.begin(); itrans != istate->get()->transitions.end(); ++itrans) {
			for (scxml_parser::slist::iterator ievent = itrans->get()->event.begin(); ievent != itrans->get()->event.end(); ++ievent) {
				if (ievent->size() >= 2 && string(ievent->rbegin(), ievent->rbegin() + 2) == "*.") ievent->erase(ievent->size()-2, 2);
				string org = *ievent;
				replace(ievent->begin(), ievent->end(), '-', '_');
				if (org != *ievent) changes[org] = *ievent;
			}

		}
	}

	// replace above event changes in actions as well
	for (scxml_parser::state_list::const_iterator istate = states.begin(); istate != states.end(); ++istate) {
		for (scxml_parser::transition_list::const_iterator itrans = istate->get()->transitions.begin(); itrans != istate->get()->transitions.end(); ++itrans) {
			// transition actions
			for (scxml_parser::plist<scxml_parser::action>::const_iterator iaction = itrans->get()->actions.begin(); iaction != itrans->get()->actions.end(); ++iaction) {
				for (map<string, string>::iterator iattr = iaction->get()->attr.begin(); iattr != iaction->get()->attr.end(); ++iattr) {
					for (map<string, string>::iterator ichanges = changes.begin(); ichanges != changes.end(); ++ichanges) {
						iattr->second = regex_replace(iattr->second, regex(ichanges->first), ichanges->second);
					}
				}
			}
		}

		// entry actions
		for (scxml_parser::plist<scxml_parser::action>::const_iterator ai = istate->get()->entry_actions.begin(); ai != istate->get()->entry_actions.end(); ++ai) {
			for (map<string, string>::iterator iattr = ai->get()->attr.begin(); iattr != ai->get()->attr.end(); ++iattr) {
				for (map<string, string>::iterator ichanges = changes.begin(); ichanges != changes.end(); ++ichanges) {
					iattr->second = regex_replace(iattr->second, regex(ichanges->first), ichanges->second);
				}
			}
		}

		// exit actions
		for (scxml_parser::plist<scxml_parser::action>::const_iterator ai = istate->get()->exit_actions.begin(); ai != istate->get()->exit_actions.end(); ++ai) {
			for (map<string, string>::iterator iattr = ai->get()->attr.begin(); iattr != ai->get()->attr.end(); ++iattr) {
				for (map<string, string>::iterator ichanges = changes.begin(); ichanges != changes.end(); ++ichanges) {
					iattr->second = regex_replace(iattr->second, regex(ichanges->first), ichanges->second);
				}
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
	if (opt.bare_metal && opt.debug == "clog") {
		cerr << "error: The debug option is not currenty supported with bare metal C++" << endl;
		exit(1);
	}
	if (opt.bare_metal && opt.thread_safe) {
		cerr << "error: The threadsafe option is not currenty supported with bare metal C++" << endl;
		exit(1);
	}
	if (opt.bare_metal && opt.string_events) {
		cerr << "error: String events are not supported with bare metal C++" << endl;
		exit(1);
	}

	// include guard
	out << "// This file is automatically generated by scxmlcc (version " << version() << ")" << endl;
	out << "// For more information, see http://scxmlcc.org" << endl;
	out << endl;
	out << "#ifndef SC_" << boost::to_upper_copy(sc.sc().name) << "_INCLUDED" << endl;
	out << "#define SC_" << boost::to_upper_copy(sc.sc().name) << "_INCLUDED" << endl;
	out << endl;

	if(sc.using_parallel) {
                out << "#include <typeinfo>" << endl;
		out << "#include <functional>" << endl;
		out << "#include <bitset>" << endl;
		out << "#include <algorithm>" << endl;
	}
	if(!opt.bare_metal) {
		out << "#include <deque>" << endl;
		out << "#include <vector>" << endl;
		out << "#include <string>" << endl;
	}
	if(opt.thread_safe) {
		out << "#include <condition_variable>" << endl;
		if(opt.cpp14) {
			out << "#include <boost/optional.hpp>" << endl;
		} else {
			out << "#include <optional>" << endl;
		}
	}
	if(opt.string_events) {
		out << "#include <unordered_map>" << endl;
		if(opt.cpp14) {
			out << "#include <boost/any.hpp>" << endl;
		} else {
			out << "#include <any>" << endl;
		}
		if(opt.thread_safe) {
			out << "#include <mutex>" << endl;
		}
	}
	if(sc.using_log || opt.debug == "clog" || opt.debug == "scxmlgui") {
		out << "#include <iostream>" << endl;
	}
        if(!opt.cpp14) {
		out << "#include <memory_resource>" << endl;
        }
	out << endl;

	if(! sc.sc().scripts.empty()) {
		out << "// user initialisations" << endl;
		for(const auto script : sc.sc().scripts) {
			// this would work, but ugly indention:
			// gen_action_part(*script);
			out << script->attr["expr"] << endl;
		}
		out << endl;
	}

	if(!opt.ns.empty()) {
		out << "namespace " << opt.ns << " {" << endl;
	}
	trim();
	gen_sc();
	gen_actions();

	if(!opt.ns.empty()) {
		out << "} //namespace " << opt.ns << endl;
	}
	// end of include guard
	out << "#endif" << endl;

}

