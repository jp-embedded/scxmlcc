[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](getting-started.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Getting started
## Prerequisites
To build scxmlcc, you will need:
 * The boost library
 * Basic build tools

On Ubuntu, install these with e.g. apt-get:
```
sudo apt-get install build-essential libboost-all-dev autorevision
```
## Optaining And Building scxmlcc
scxmlcc is available as compressed releases or through github. The latest and previous releases is available at [Releases](https://github.com/jp-embedded/scxmlcc/releases).

To build scxmlcc, simply execute make:
```
cd src
make
```
If you also want to build the examples:
```
cd examples
make
```
## The Examples
This sections describes the examples which can be found at src/examples.
### Hello World
This is really not a practical state machine. It is just a simple expample for getting started with scxmlcc. This statechart is drawn in [scxmlgui](https://github.com/fmorbini/scxmlgui/), just two states connected with an unconditional transition. 

![hello_img](hello.png)

scxmlgui stores this as the following scxml:

```
<scxml initial="hello" version="0.9" xmlns="http://www.w3.org/2005/07/scxml">
 <state id="hello">
  <transition target="world"></transition>
 </state>
 <state id="world"></state>
</scxml>
```

This statechart can be compiled to an .h file using scxmlcc:
```
scxmlcc -o hello_world.h hello_world.scxml
```

This will generate hello_world.h which contains a sc_hello_world class implementing the statechart. 

Custom actions can be added to this statechart in C++ by specializing members in the sc_hello_world class. For example, consider the hello_world.cpp example which specializes the enter action on both states and instantiates the state machine:
```
#include "hello_world.h"
#include <iostream>

using namespace std;

typedef sc_hello_world sc;

template<> void sc::state_actions<sc::state_hello>::enter(sc::data_model &m)
{
	cout << "hello" << endl;	
}

template<> void sc::state_actions<sc::state_world>::enter(sc::data_model &m)
{
	cout << "world" << endl;	
}

int main(int argc, char *argv[])
{
	sc sc0;
	sc.init();
	return 0;
}
```
As expected, this will output the hello world words:
```
hello
world
```
The init method initializes the state machine and makes it enter its initial state.

Note that, as stated on the [home](https://github.com/jp-embedded/scxmlcc) page, no external libraries is nedded, and zero code is generated for the non specialized actions making it an efficient implementation.
### Timer Switch
This example implements a simple timer switch:

![timer_switch](timer_switch.png)

The idea is that the switch can be turned on or off by sending a button event to the state machine. In the on state the state machine also returns to the off state after a timeout. 
In the on state, a timer event counts up a timer value. The unconditional event from on to off then have a condition, so it will be executed when this timer value is >= 5. Finaly, the off state will reset the timer value to 0.

The dotted timer transition means that it is a transition without target. This means that the on state's enter/exit actions won't be executed on timer events, as opposed to if the transition had target.

These actions implement this in timer_switch.cpp as:
```
template<> void sc::state_actions<sc::state_on>::enter(sc::data_model &m)
{
	cout << "on" << endl;	
}

template<> void sc::state_actions<sc::state_off>::enter(sc::data_model &m)
{
	cout << "off" << endl;	
	m.user->timer = 0;
}

template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_on, sc::state_off>::condition(sc::data_model &m)
{
	return m.user->timer >= 5;
}

template<> void sc::transition_actions<&sc::state::event_timer, sc::state_on>::enter(sc::data_model &m)
{
	++m.user->timer;
}
```

For this to work, a data model must be defined containing the timer variable. The generated state machine class has a user_model struct forward declaration, so a custom data model can be defined. This is done by defining user_model:
```
struct sc::user_model 
{
	int timer;
};
```

The state machine stores this struct in an auto pointer, declared as user_model_p. Thus the model is automatically destructed and deleted, when the state machine is destructed. The custom data model can be constructed and passed to the state machine's constructor as:
```
sc::user_model_p m(new sc::user_model);
sc sc(m);
```

For the full example source, see timer_switch.cpp.
### Vending machine
This example brings a bit more realistic example, where diffrent components need to interact with a state machine.The example implemets a simplified vending machine which can dispense three types of coke, after the customer has inserted currency into the machine. Below is a drawing of the diffrent components:
![vending_components](vending_components.png)
 * The 'State Machine' is responsible for the logic. This is described later.
 * The 'Coin Sensor' is responsible for detecting coin inserts.
 * The 'Keypad' is responsible for detecting key presses.
 * The 'Display' is responsible for displaying text messages.
 * The 'Coin refund' is responsible for refunding coins to the user.
 * The 'Dispenser' is responsible for dispensing the three coke types to the user.
 * The 'Input simulator' is reading input from your PC's keyboard and trigger the coin sensor and the keypad.

The input simulator is added so we can simulate the vending machine, since we don't have a real coin sensor and keypad. Also since this example is only for simulating, the component implementations are really small and does not imlpement any real hardware support. But the machine are split into these components anyway to make the example a bit more realistic.

The communication between the componets is made with a signaling mechanism, which is a way of decoubling dependencies between components. So the individual components does not know of each other.

The state machine is implemented as follows:
![vending_machine](vending_machine.png)

After initialization, the state machine will enter the 'collect_coins' state, waiting for coins. When coins are inserted, a 'credit' variable in the user model is incremented through the 'N' and 'D' events. When the 'zero', 'coke' or 'diet' button is pressed on the machine, the corersponing event 'zero', 'coke' or 'diet' is signaled. The three transitions to the 'dispense_\*' states have a condition that the credit must be equal or above the price for the coke. If this condition is met, the state machine will transition to the according dispense_\* state, where the corresponding coke is dispensed. The three dispense states are final states, so they will signal a 'done' and the state machine will transition to the 'coin_return' state. While in the 'active' state, if the 'cancel' button is pressed, the corresponding 'cancel' event is issued, and the state machine will as well transition to the 'coin_return' state.

In the 'coin_return' state, the state machine 
