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
private:
    std::ostream &out;
    const scxml_parser &sc;
    const options &opt;

};

#endif // DOT_OUTPUT_H
