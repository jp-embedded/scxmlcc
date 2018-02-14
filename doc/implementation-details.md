[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](index.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Implementation Details
## The State Pattern
The design of the generated state machines is based on the [State Pattern](https://en.wikipedia.org/wiki/State_pattern).

From Wikipedia:
>The state pattern is a behavioral software design pattern that implements a state machine in an object-oriented way. With the state pattern, a state machine is implemented by implementing each individual state as a derived class of the state pattern interface, and implementing state transitions by invoking methods defined by the pattern's superclass.

todo: events 

The state pattern is easily extended into hierarchical statemachines by using class inheritance. Consider the folowing hierarchical state machine:

todo: pic

We can translate this into a C++ class hierarchy:

todo: pic

todo: describe function


## Transition Handling
