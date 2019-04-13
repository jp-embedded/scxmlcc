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
        , currentClusterNumber(1)
    {}

    void gen();
protected:
    void gen_states();
    void gen_state(const scxml_parser::state& state, std::ostream &os);
    void gen_state_with_children(const scxml_parser::state& state, std::ostream &os);
    void gen_state_simple(const scxml_parser::state& state, std::ostream &os);
    void gen_state_final(const scxml_parser::state& state, std::ostream &os);
    void gen_transition(const scxml_parser::state& sourceState, const scxml_parser::transition &transition, std::ostream &os);
    void gen_actions(const scxml_parser::plist<scxml_parser::action>& actions, std::ostream &os);


    bool stateAdded(const std::string &stateName) const;
    bool addState(const std::string &stateName, int clusterNumber = 0);
    int getStateClusterNumber(const std::string& stateName) const;
    bool hasChildren(const scxml_parser::state& state) const;
    std::vector<std::string> getChildrenNames(const scxml_parser::state& state) const;
    /**
     * @brief getState
     * @param stateName
     * @return
     * @throws exception if not found
     */
    const scxml_parser::state& getState(const std::string& stateName);
    const scxml_parser::state& getFirstLeafState(const scxml_parser::state& state);

private:
    std::ostream &out;
    const scxml_parser &sc;
    const options &opt;


    std::map<std::string, int> addedStateNames; // state name, cluster nr (0 = none)
    int currentClusterNumber;

};

#endif // DOT_OUTPUT_H
