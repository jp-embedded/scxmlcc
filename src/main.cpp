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
#include "cpp_output.h"
#include "dot_output.h"
#include "options.h"
#include "version.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

using namespace boost::property_tree;
using namespace std;
namespace fs = boost::filesystem;

bool c_pred(char c)
{
	return !isalnum(c);
}

void scxmlcc(const options &opt)
{
	ptree pt;
	read_xml(opt.input.string(), pt);
	std::string sc_name = opt.input.stem().string();
	replace_if(sc_name.begin(), sc_name.end(), c_pred, '_');
	scxml_parser sc(sc_name.c_str(), opt.ignore_unknown, pt);
	ofstream ofs(opt.output.string().c_str());
	cpp_output out(ofs, sc, opt);
    out.gen();
    if(!opt.dotFile.empty())
    {
        ofstream dotFile(opt.dotFile.string().c_str());
        dot_output dot(dotFile, sc, opt);
        dot.gen();
    }
}

int main(int argc, char *argv[])
{
	using namespace boost::program_options;
	options_description desc("Options");
	desc.add_options()
		("help,h",				"This help message")
		("input,i",	value<string>(),	"Input file.")
		("output,o",	value<string>(),	"Output file.")
        ("dot,D", value<string>(), "Generate Graphviz Dot file")
        ("debug,d",	value<string>(),	"Enable debug output (to clog or scxmlgui)")
		("ignore-unknown,u",	value<string>(),"ignore unknown xml elements matching regex")
		("baremetal,b",				"Generate code for bare metal C++")
		("threadsafe,t",			"Generate threadsafe code for event_queue")
		("stringevents,s",			"Enable triggering events by name")
		("namespace,n",	value<string>(),	"Generate code in given namespace")
		("cpp14", "Generate code for C++14 instead of C++17, needs boost")
		("version,v",				"Version and copyright information");
	positional_options_description pdesc;
	pdesc.add("input", -1);
	variables_map vm;
	try {
		store(parse_command_line(argc, argv, desc), vm);
		store(command_line_parser(argc, argv).options(desc).positional(pdesc).run(), vm);
		notify(vm);
	}
	catch (error &e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}

	options opt;

	if(vm.count("input")) opt.input = vm["input"].as<string>();
	if(vm.count("output")) opt.output = vm["output"].as<string>();
    if(vm.count("dot")) opt.dotFile = vm["dot"].as<string>();
	if(vm.count("ignore-unknown")) opt.ignore_unknown = vm["ignore-unknown"].as<string>();
	if(vm.count("debug")) opt.debug = vm["debug"].as<string>();
	if(vm.count("baremetal")) opt.bare_metal = true;
	if(vm.count("threadsafe")) opt.thread_safe = true;
	if(vm.count("stringevents")) opt.string_events = true;
	opt.cpp14 = (vm.count("cpp14") != 0);

	if(vm.count("namespace")) opt.ns = vm["namespace"].as<string>();

	if(!opt.input.empty() && opt.output.empty()) {
		opt.output = opt.input;
		opt.output.replace_extension(".h");
	}

	if(vm.count("version")) {
		cout << "scxmlcc version: " << version() << endl;
		cout << "For more information, see http://scxmlcc.org" << endl;
		cout << endl; 
		cout << "  Copyright (C) 2013-2016 Jan Pedersen <jp@jp-embedded.com>" << endl;
		cout << endl; 
		cout << "  This program is free software: you can redistribute it and/or modify" << endl;
		cout << "  it under the terms of the GNU General Public License as published by" << endl;
		cout << "  the Free Software Foundation, either version 3 of the License, or" << endl;
		cout << "  (at your option) any later version." << endl;
		cout << endl;  
		cout << "  This program is distributed in the hope that it will be useful," << endl;
		cout << "  but WITHOUT ANY WARRANTY; without even the implied warranty of" << endl;
		cout << "  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << endl;
		cout << "  GNU General Public License for more details." << endl;
		cout << endl; 
		cout << "  You should have received a copy of the GNU General Public License" << endl;
		cout << "  along with this program.  If not, see <http://www.gnu.org/licenses/>." << endl;
		cout << endl;  
		return 0;
	}

	if(vm.count("help") || opt.input.empty()) {
		cout << "Usage: " << argv[0] << " [options] [input]" << endl;
		cout << desc << endl;
		return 0;
	}

	if (!opt.debug.empty()) {
		if ( (opt.debug != "clog")
		&&   (opt.debug != "scxmlgui") ) {
			cout << "Error: Unknown debug output specified." << endl;
			return -1;
		}
	}

	try {
		scxmlcc(opt);
	}
	catch (const std::exception& exc) {
		cerr << "Unhandled exception: " << exc.what() << '\n';
		return 1;
  }
	return 0;
}
