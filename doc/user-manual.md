[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](index.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Users Manual
## Current Status
All the following tags from the scxml standard are supported: `<scxml>`, `<state>`, `<parallel>`, `<transition>`, `<initial>`, `<final>`, `<onentry>`, `<onexit>`, `<raise>`, `<log>`, `<assign>`, `<script>`, `<datamodel>`, `<data>`. For some tags, not all attributes may be supported. See below for further description.
  
## Core Constructs
This is a description of the supported core constructs and attributes from the scxml standard.
### State Machine (`<scxml>`)
The top-level wrapper element. The actual state machine consists of its children.

#### Attributes
##### `name`
Specifies the name of the state machine. This name will be used for the class name of the generated state machine code. If omitted, the filename will be used as name.

##### `initial`
The id of the initial state(s) for the scxml document. If not specified, the default initial state is the first child state in document order.

If multiple states are specified, any two of them must have a `<parallel>` state as their nearest common ancestor and they must not be descendants of each other.

If not all children of a `<paralel>` state is included, the remaining children will enter their initial state.

##### `xmlns`
This attribute is required and must be `"http://www.w3.org/2005/07/scxml"`.

##### `version`
This attribute is required and must be `"1.0"`

##### `datamodel`
The datamodel that this scxml document requires. The current supported datamodels are `"null"` and `"cplusplus"`. For further description of the datamodels, see section [Data Models](user-manual.md#data-models).

##### `binding`
This attribute is currently ignored. Only `"early"` binding is currently supported, which means all data model objects are constructed when the state machine is constructed.

#### Valid Children

- [`<state>`](user-manual.md#state-state)
- [`<parallel>`](user-manual.md#parallel-state-parallel)
- [`<final>`](user-manual.md#final-state-final)
- [`<datamodel>`](user-manual.md#data-model-datamodel)

#### Example
```
<scxml initial="hello" version="1.0" xmlns="http://www.w3.org/2005/07/scxml">
 ...
</scxml>
```

### State (`<state>`)
This element represents a state in the state machine. A state can hold child states to form a hierarchical state machine.

A state may specify either an `initial` attribute or an `<initial>` child element, but not both. If none of them are present, the state's first child state in document order is used as inital state.

If the state has any child states, any transition which takes this state as its target will result in the state machine also taking the transition to its inital state. 

#### Attributes

##### `id`
An identifier for the state. This attribute is required.

##### `initial`
The id of the default initial state (or states) for this state. Alternatively, the initial state can be set with the `<initial>` child.

The initial state(s) specified must be descendant(s) of the containing `<state>`.

If multiple states are specified, any two of them must have a `<parallel>` state as their nearest common ancestor and they must not be descendants of each other.

If not all children of a `<paralel>` state is included, the remaining children will enter their initial state.

#### Valid Children
- [`<onentry>`](user-manual.md#enter-action-onentry)
- [`<onexit>`](user-manual.md#exit-action-onexit)
- [`<transition>`](user-manual.md#transition-transition)
- [`<initial>`](user-manual.md#initial-initial)
- [`<state>`](user-manual.md#state-state)
- [`<parallel>`](user-manual.md#parallel-state-parallel)
- [`<final>`](user-manual.md#final-state-final)
- [`<datamodel>`](user-manual.md#data-model-datamodel)

#### Example
```
<state id="active" initial="collect_coins">
  ...
</state>
```

### Parallel State (`<parallel>`)

This construct is currently only partially implemented. Also, other constructs may currently not work correctly in combination with parallel states.

scxmlcc is made to generate efficient code, so it is worth mentioning that when using parallel states, performance will degrade compared to state machines without parallel states, because the state machine now has to maintain a list of active states instead of a single active state. However if perfomance is not critical this will probaly not be significant.

The `<parallel>` element encapsulates a set of child states which are simultaneously active.

#### Attributes

##### `id`
An identifier for the state. This attribute is required.

#### Valid Children
- [`<onentry>`](user-manual.md#enter-action-onentry)
- [`<onexit>`](user-manual.md#exit-action-onexit)
- [`<transition>`](user-manual.md#transition-transition)
- [`<state>`](user-manual.md#state-state)
- [`<parallel>`](user-manual.md#parallel-state-parallel)
- [`<datamodel>`](user-manual.md#data-model-datamodel)

#### Example
```
<parallel id="s11p1">
  ...
</parallel>

```
### Transition (`<transition>`)
This element specifies transitions between states. The transitions are triggered by events, They may contain execute conditions and executeable content.

The source state of the transition is the state in which the transition is a child of.

When a transition is executed, the current state and its parents are exited up to the nearest common ancestor of the current state and the transition target. Then, all states are entered from the ancestor to the transition's target.

If a parallel state is exited, the other parallel state's children are exited also.

At least one of the attributes `event`, `cond` or `target` must be specified.

If multiple transitions may be triggered by an event, one of these transitions are proiritized. If T1 and T2 are two conflicting transitions and their condition evaluates to true, T1 is prioritized if T1's source state is a descendant of T2's source state. Otherwise they are prioritized in document order.

#### Attributes

##### `event`
A space seperated list of events that trigger this transition.

todo: event tokens

##### `cond`
An execute condition. The transition is only executed if the condition evaluates to true. The format of the condition depends on the data model specified in [`<scxml>`](user-manual.md#state-machine-scxml)


##### `target`
The id of the state to transition to.

##### `type`
Can be `"internal"` or `"external"`. If omitted, the type is external. If the type is internal and the target state is a descendant of the transitions source state, the transition will not exit and re-enter its source state, while an external one will. Internal transitions are useful for initial transitions as a child of [`<initial>`](user-manual.md#initial-initial)

#### Valid Children
- [`<raise>`](user-manual.md#raise-raise)
- [`<log>`](user-manual.md#log-log)
- [`<script>`](user-manual.md#script-script)
- [`<assign>`](user-manual.md#assign-assign)

#### Example
```
<transition event="cancel" target="coin_return">
  ...
</transition>
```

#### Custom Actions and Conditions

The transition's executable content can also be defined in C++ code instead of the scxml document, so the state machine and it's actions can be seperated. To do this the transition must not contain executable content in the scxml document.

Transition actions are implemented in a `transition_actions` template inside the state machine class. This template can be specialized for each transition to implement custom actions. If the state machine is called `sc`, a transition action can be specialized like this in the C++ file that includes the generated state machine:

```
template<> void sc::transition_actions<E, S, D>::enter(sc::data_model &m)	{ ... }
```
Where `E` is the event, `S` is the source state, `D` is the destination state. `D` is omitted if the transition has no target.

The event is a member method in the sc::state struct, and the states are structs defined in the sc class. The data model is passed in the `m` parameter so the data model can be accessed.

So, for example. This is for a transition without target:
```
template<> void sc::transition_actions<&sc::state::event_timer, sc::state_on>::enter(sc::data_model &m)
{
        ++m.user->timer;
}
```

Custom transition conditions can be made the same way by specializing this template:
```
template<> bool sc::transition_actions<E, S, D>::condition(sc::data_model &m) { ... }
```
Where `E` is the event, `S` is the source state, `D` is the destination state. `D` is omitted if the transition has no target.

For example:
```
template<> bool sc::transition_actions<&sc::state::unconditional, sc::state_on, sc::state_off>::condition(sc::data_model &m)
{
        return m.user->timer >= 5;
}
```

### Initial (`<initial>`)
This element represents the default initial state for a <state> element.

Note that the child transition must not contain `cond` or `event` attributes, and must specify a `target` whose value is state(s) consisting solely of descendants of the containing state.

#### Attributes
None.

#### Valid Children
- [`<transition>`](user-manual.md#transition-transition)

#### Example
```
<initial>
  ...
</initial>
```

### Final State (`<final>`)
`<final>` represents a final state of the state machine.

When a final state is entered, a `done.state.ID` event is raised, where ID is the id of the parent state. If this parent state is a child of a `<parallel>` element, and all of the `<parallel>`'s other children are also in final states, a `done.state.ID` event is raised where ID is the id of the `<parallel>` element.

If the entered final state is a child of the `<scxml>` element, the state machine must terminate. This is currently not implemented.

#### Attributes

##### `id`
An identifier for the state. This attribute is optional.

#### Valid Children
- [`<onentry>`](user-manual.md#enter-action-onentry)
- [`<onexit>`](user-manual.md#exit-action-onexit)

#### Example
```
<final id="dispense_coke">
  ...
</final>
```

### Enter Action (`<onentry>`)
A wrapper element containing executable content to be executed when the state is entered

#### Attributes
None

#### Valid Children
- [`<raise>`](user-manual.md#raise-raise)
- [`<log>`](user-manual.md#log-log)
- [`<script>`](user-manual.md#script-script)
- [`<assign>`](user-manual.md#assign-assign)

#### Example
```
<onentry>
  <raise event="foo"/>
  <raise event="bar"/>
</onentry>
```

#### Custom Enter Action
A state's entry action can also be defined in C++ code instead of the scxml document, so the state machine and it's actions can be seperated. To do this the enter action must not contain executable content in the scxml document.

Entry actions are implemented in a `state_actions` template inside the state machine class. This template can be specialized for each state to implement custom actions. If the state machine is called `sc`, an entry action can be specialized like this in the C++ file that includes the generated state machine:
```
template<> void sc::state_actions<S>::enter(sc::data_model &m) { ... }
```
Where S is the state.

For example:
```
template<> void sc::state_actions<sc::state_hello>::enter(sc::data_model &m)
{
        cout << "enter hello" << endl;
}
```

### Exit Action (`<onexit>`)
A wrapper element containing executable content to be executed when the state is exited.

#### Attributes
None

#### Valid Children
- [`<raise>`](user-manual.md#raise-raise)
- [`<log>`](user-manual.md#log-log)
- [`<script>`](user-manual.md#script-script)
- [`<assign>`](user-manual.md#assign-assign)

#### Example
```
<onexit>
  <raise event="foo"/>
  <raise event="bar"/>
</onexit>
```

#### Custom Exit Action
A state's exit action can also be defined in C++ code instead of the scxml document, so the state machine and it's actions can be seperated. To do this the exit action must not contain executable content in the scxml document.

Exit actions are implemented in a `state_actions` template inside the state machine class. This template can be specialized for each state to implement custom actions. If the state machine is called `sc`, an exit action can be specialized like this in the C++ file that includes the generated state machine:
```
template<> void sc::state_actions<S>::exit(sc::data_model &m) { ... }
```
Where S is the state.

For example:
```
template<> void sc::state_actions<sc::state_hello>::exit(sc::data_model &m)
{
        cout << "exit hello" << endl;
}
```


## Executable Content
Executable content allows the state machine to do things. It provides the hooks that allow an SCXML session to modify its data model and interact with external entities. Executable content consists of actions that are performed as part of taking transitions. In particular, executable content occurs inside [`<onentry>`](user-manual.md#enter-action-onentry) and [`<onexit>`](user-manual.md#exit-action-onexit) elements as well as inside transitions. When the state machine takes a transition, it executes the [`<onexit>`](user-manual.md#exit-action-onexit) executable content in the states it is leaving, followed by the content in the transition, followed by the [`<onentry>`](user-manual.md#enter-action-onentry) content in the states it is entering.

### Raise (`<raise>`)
todo

### Log (`<log>`)
todo

### Script (`<script>`)
todo

### Assign (`<assign>`)
todo

## Data Model and Data Manipulation
todo

### Data Model (`<datamodel>`)
todo

#### Attributes
todo

#### Valid Children
todo

#### Example
todo

## Data Models
### The `null` datamodel
todo

### The `cplusplus` datamodel
todo

## Invoking scxmlcc
todo

## Including scxmlcc in your makefile
todo

## Performance
todo
