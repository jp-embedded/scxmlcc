[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](index.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Implementation Details
## The State Pattern
The design of the generated state machines is based on the [State Pattern](https://en.wikipedia.org/wiki/State_pattern).

From Wikipedia:
>The state pattern is a behavioral software design pattern that implements a state machine in an object-oriented way. With the state pattern, a state machine is implemented by implementing each individual state as a derived class of the state pattern interface, and implementing state transitions by invoking methods defined by the pattern's superclass.

Here, the state pattern interface is implemented in the `state` class. This interface consists of a virtual method for each possible event in the state machine. Each state in the state machine is a derived class of `state`, which implemets the events to handle.

todo: composite?
todo: events 

The state pattern is easily extended into hierarchical statemachines by using class inheritance. Consider the folowing hierarchical state machine:

todo: pic

We can translate this into a C++ class hierarchy:

todo: pic state<-s0<-s1<-s2

Now, if the current state is s2 and an event is invoked, it will be handled automatically by the most derived implementation. If invoking e0 it will be handled by `s2::e0()` and e1 will be handled by `s1::e1()`.

todo: describe function


## Transition Handling
