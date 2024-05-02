cmake_minimum_required(VERSION 2.8.11)

include( scxmlcc_generator )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set(S ${CMAKE_CURRENT_SOURCE_DIR} )

set (CMAKE_CXX_STANDARD 17)

add_executable( hello_world hello_world.cpp )
scxmlcc_generator(INPUTS ${S}/hello_world.scxml CLIARGUMENTS --debug=clog --cpp14 TARGETS hello_world )

add_executable( timer_switch timer_switch.cpp )
scxmlcc_generator(INPUTS ${S}/timer_switch.scxml TARGETS timer_switch )

add_executable( microwave-01 microwave-01.cpp )
scxmlcc_generator(INPUTS ${S}/microwave-01-cplusplus.scxml TARGETS microwave-01 )

add_executable( microwave-02 microwave-02.cpp )
scxmlcc_generator(INPUTS ${S}/microwave-02-cplusplus.scxml TARGETS microwave-02 )

add_executable( vending_machine
    vending_machine/coin_refund.cpp
    vending_machine/coin_refund.h
    vending_machine/coin_sensor.cpp
    vending_machine/coin_sensor.h
    vending_machine/dispenser.cpp
    vending_machine/dispenser.h
    vending_machine/display.cpp
    vending_machine/display.h
    vending_machine/input.cpp
    vending_machine/input.h
    vending_machine/keypad.cpp
    vending_machine/keypad.h
    vending_machine/machine.h
    vending_machine/main.cpp
    vending_machine/signal.h
    )
scxmlcc_generator(INPUTS ${S}/vending_machine/vending_machine.scxml TARGETS vending_machine )
