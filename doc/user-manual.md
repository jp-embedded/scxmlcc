[Home](https://github.com/jp-embedded/scxmlcc) | [Documentation](index.md) | [Latest Release](https://github.com/jp-embedded/scxmlcc/releases) | [scxmlgui](https://github.com/fmorbini/scxmlgui/)
# Users Manual
## Current Status
All the following tags from the scxml standard are supported: `<scxml>`, `<state>`, `<parallel>`, `<transition>`, `<initial>`, `<final>`, `<onentry>`, `<onexit>`, `<raise>`, `<log>`. For some tags, not all attributes may be supported. See below for further description.
  
## Core Constructs
This is a description of the supported constructs and attributes from the scxml standard.
### \<scxml\>
- initial


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
