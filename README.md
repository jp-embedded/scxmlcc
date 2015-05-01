[Documentation](https://github.com/jp-embedded/scxmlcc/blob/master/doc/getting-started.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://code.google.com/p/scxmlgui/)
# Welcome to scxmlcc.
The scxml to c++ compiler.

## What?
The goal of scxmlcc is to make a Open Source full featured state machine compiler that can gererate simple and effective C++ state machines from scxml state charts.

This project is developed and professionally supported by [jp-embedded ApS](http://jp-embedded.com) and is released under the [GNU GPL v3](http://www.gnu.org/licenses/gpl.html). However, the generated state machine code is not covered by license.

## Why?

There are several reasons why this project started.

* Typically, state machine implementations depend on external libraries or specific, often expensive, design tools. The generated state machines from this scxml compiler has no external dependencies. It uses STL only.
* Instead of inventing yet another standard for describing state machines, scxmlcc uses the [scxml standard](http://www.w3.org/TR/scxml/), so the behavior is well defined, and gives freedom to use any xml editor or statechart design tool which supports scxml.
* State machine implementations often lack features to make it usable for real applications. scxmlcc uses the scxml standard which is featured for real applications.
* From a programming perspective, implementing state machines often lead to using if-else or switch-case constructs, because it seems like the simplest. Most programs start out fairly simple and well structured, but as new features are implemented, this very often leads to deeply nested constructs. Scxmlcc aims at being easy to implement to make it a real alternative.

## Features

scxmlcc does not yet support all features of the scxml standard. Most of the core constructs are in place though. Besides the standard, custom constructs is implemented. This means that constructs like the data model, actions and conditions can be implemented in C++ and then be attached to the state machine, instead of implementing them in scxml. Thus, seperating the state machine and the logic.

The generated state machines use standard C++ constructs as much as possible such as abstract classes and templates to minimize the code needed to implement the state machine. However we try not to overdo templates to keep the code readable, while making the code as efficient as possible. Macros are totally avoided. If you like macros to for example define custom actions, you can easily add them on your own.

The folowing features are currently implemented:
* Hierarchical states.
* Internal, external, targetless, eventless and conditional transitions.
* Custom data model.
* Custom enter/exit state actions.
* Custom transition actions and conditons.
* Zero runtime overhead for emtpy actions and conditions.
* LCA calculation is done (mostly) compile time.

Next todo on the feature list is to complete the core constructs of the scxml standard. Most importantly, this include:

* Parallel states (in progress)
* History states
* Final states

If you have any suggestions on features to prioritize, please create an [issue](https://github.com/jp-embedded/scxmlcc/issues).
