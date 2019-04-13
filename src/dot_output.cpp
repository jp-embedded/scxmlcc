#include "dot_output.h"
#include "version.h"


void dot_output::gen()
{
    out << "// This file is automatically generated by scxmlcc (version " << version() << ")\n"
        << "// For more information, see http://scxmlcc.org\n"
        << "// View result online: https://dreampuf.github.io/GraphvizOnline/\n"
        << '\n';

    out << "digraph finite_state_machine {\n"
        << "\tnode [shape = Mrecord];\n"
        << "\tcompound=true;\n"
        << "\tsize=\"8,5;\"\n";

    gen_states();

    out << "}\n";
}

void dot_output::gen_states()
{
    const auto& states = sc.sc().states;

    // states
    for(const auto& state : states)
    {
        gen_state(*state);

        for(const auto& transition : state->transitions)
        {
            gen_transition(*state, *transition);
        }
    }
}

void dot_output::gen_state(const scxml_parser::state &state)
{
    //out << "// State " << state.id << " Type = " << state.type << '\n';
    if(! state.type)
    {
        out << '\t' << state.id << "[\n"
            << "\t\tlabel=<\n"
            << "\t\t\t<table border='0' cellborder='0' style='rounded'>\n"
            << "\t\t\t\t<tr><td colspan='3'><b>" << state.id << "</b></td></tr>\n";

        if(!state.entry_actions.empty())
        {
            out << "\t\t\t\t<tr><td rowspan='" << state.entry_actions.size() << "'>entry:</td>";
            for(unsigned int i = 0; i < state.entry_actions.size(); i++)
            {
                auto& action = state.entry_actions[i];
                if(i > 0)
                {
                    out << "</tr><tr>";
                }
                out << "<td>" << action->type << ": </td>"
                    << "<td> TODO </td>";
            }
            out << "</tr>\n";
        }
        if(!state.exit_actions.empty())
        {
            out << "\t\t\t\t<tr><td rowspan='" << state.exit_actions.size() << "'>exit:</td>";
            for(unsigned int i = 0; i < state.exit_actions.size(); i++)
            {
                auto& action = state.exit_actions[i];
                if(i > 0)
                {
                    out << "</tr><tr>";
                }
                out << "<td>" << action->type << ": </td>"
                    << "<td> TODO </td>";
            }
            out << "</tr>\n";
        }
        out << "\t\t\t</table>\n"
            << "\t\t>]\n";
    }else
    {
        if(*state.type == "final")
        {
            out << '\t' << state.id
                << "[label=\"\","
                << "shape=doublecircle,"
                << "style=filled,"
                << "fixedsize=true,"
                << "fillcolor=black,"
                << "width=0.2];"
                << " // Final\n";
        }
    }
}

void dot_output::gen_transition(const scxml_parser::state& state, const scxml_parser::transition& transition)
{
    scxml_parser::slist targets = transition.target;
    if(targets.empty())
    {
        targets.push_back(state.id);
    }
    for(const auto& target : targets)
    {
        std::string events;
        for(const auto& event : transition.event)
        {
            events += event + ",";
        }
        if(!events.empty())
        {
            events.pop_back();
        }
        out << "\t\t" << state.id << "->" << target << "[";
        if(transition.type && *transition.type == "internal")
        {
            out << "style=\"dashed\",";
        }

        out << "label=<\n"
            << "\t\t\t<table border='0'>\n"
            << "\t\t\t\t<tr><td colspan='2'>" << events;
        if(transition.condition)
        {
          out << " ["<< transition.condition << "]";
        }
        out << "</td></tr>\n";
        if(!transition.actions.empty())
        {
            gen_actions(transition.actions);
        }

        out << "\t\t\t</table>\n"
            << "\t\t\n";
        out << "\t>];\n";
    }
}

void dot_output::gen_actions(const scxml_parser::plist<scxml_parser::action> &actions)
{
    for(const auto& action : actions)
    {
        out << "\t\t\t\t<tr><td>" << action->type << "</td>"
            << "<td> TODO </td></tr>";
    }

}
