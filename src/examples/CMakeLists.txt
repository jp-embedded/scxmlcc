cmake_minimum_required(VERSION 2.8.11)

include( scxmlcc_generator )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set(S ${CMAKE_CURRENT_SOURCE_DIR} )
scxmlcc_generator( ${S}/hello_world.scxml hello_gen_src )
scxmlcc_generator( ${S}/timer_switch.scxml timer_gen_src )
scxmlcc_generator( ${S}/microwave-01-cplusplus.scxml microwave_gen_src )
scxmlcc_generator( ${S}/microwave-02-cplusplus.scxml microwave_gen_src )
scxmlcc_generator( ${S}/vending_machine/vending_machine.scxml vending_gen_src )

set (CMAKE_CXX_STANDARD 11)

add_executable( hello_world
    hello_world.cpp
    ${hello_gen_src} 
    )

add_executable( timer_switch
    timer_switch.cpp
    ${timer_gen_src}
    )

add_executable( microwave-01
    microwave-01.cpp
    ${microwave_gen_src}
    )

add_executable( microwave-02
    microwave-02.cpp
    ${microwave_gen_src}
    )

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
    ${vending_gen_src}
    )
