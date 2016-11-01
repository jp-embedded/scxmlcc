[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](index.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Users Manual
## Current Status
All the following tags from the scxml standard are supported: `<scxml>`, `<state>`, `<parallel>`, `<transition>`, `<initial>`, `<final>`, `<onentry>`, `<onexit>`, `<raise>`, `<log>`, `<assign>`, `<script>`, `<datamodel>`, `<data>`. For some tags, not all attributes may be supported. See below for further description.
  
## Core Constructs
This is a description of the supported core constructs and attributes from the scxml standard.
### Machine (`<scxml>`)
The top-level wrapper element. The actual state machine consists of its children.

#### Attribute `name`
Specifies the name of the state machine. This name will be used for the class name of the generated state machine code. If omitted, the filename will be used as name.

#### Attribute `initial`
The id of the initial state(s) for the scxml document. If not specified, the default initial state is the first child state in document order.

#### Attribute `xmlns`
The value must be `"http://www.w3.org/2005/07/scxml"`.

#### Valid Children

- `<state>`
- `<parallel>`
- `<final>`
- `<datamodel>`

#### Example
```
<scxml initial="hello" version="0.9" xmlns="http://www.w3.org/2005/07/scxml">
 ...
</scxml>
```

### State (`<state>`)
- id
- initial

### Enter Action (`<onentry>`)

### Custom Enter Action

### Parallel State (`<parallel>`)
- id


This construct is currently only partially implemented. Other constructs may not work correctly in combination with parallel states.
### \<transition\>
- event
- target

### \<initial\>
### \<final\>
- id


## Invoking scxmlcc
## Custom Constructs
todo what is this
### Transition Actions
```
template<> void sc::transition_actions<&sc::state::event_N, sc::state_collect_coins, sc::state_collect_coins>::enter(sc::data_model &m)	
```
#### Transition Actions With Multiple Eventes
## Including scxmlcc in your makefile
