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
The datamodel that this document requires. The current supported datamodels are `"null"` and `"cplusplus"`. For further description of the datamodels, see section [Data Models](user-manual.md#data-models).

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
### Initial (`<initial>`)

#### Attributes

#### Valid Children

#### Example

### Final State (`<final>`)

#### Attributes

#### Valid Children

#### Example

### Enter Action (`<onentry>`)

#### Attributes

#### Valid Children

#### Custom Enter Action

#### Example

### Exit Action (`<onexit>`)

#### Attributes

#### Valid Children

#### Custom Enter Action

#### Example

## Executable Content

### Raise (`<raise>`)

### Log (`<log>`)

### Script (`<script>`)

### Assign (`<assign>`)

## Data Model and Data Manipulation

### Data Model (`<datamodel>`)

#### Attributes

#### Valid Children

#### Example

## Data Models
### The `null` datamodel
todo

### The `cplusplus` datamodel
todo

## Invoking scxmlcc
## Custom Constructs
todo what is this
### Transition Actions
```
template<> void sc::transition_actions<&sc::state::event_N, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
```
#### Transition Actions With Multiple Eventes
## Including scxmlcc in your makefile

## Performance

