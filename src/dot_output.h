#ifndef DOT_OUTPUT_H
#define DOT_OUTPUT_H

#include <ostream>

#include "scxml_parser.h"
#include "options.h"

class dot_output
{
public:
    dot_output(std::ostream &ofs, const scxml_parser &sc, const options &op)
        : out(ofs)
        , sc(sc)
        , opt(op)
    {}

    void gen();
protected:
    void gen_states();
    void gen_state(const scxml_parser::state& state);
    void gen_transition(const scxml_parser::state& state, const scxml_parser::transition &transition);
    void gen_actions(const scxml_parser::plist<scxml_parser::action>& actions);
private:
    std::ostream &out;
    const scxml_parser &sc;
    const options &opt;

};

#endif // DOT_OUTPUT_H
