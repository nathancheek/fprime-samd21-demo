// ======================================================================
// \title  Main.cpp
// \brief main program for the F' application. Intended for Arduino-based systems
//
// ======================================================================
// Used to access topology functions
#include <SAMD21Demo/SAMD21DemoDeployment/Top/SAMD21DemoDeploymentTopologyAc.hpp>
#include <SAMD21Demo/SAMD21DemoDeployment/Top/SAMD21DemoDeploymentTopology.hpp>

// Used for Baremetal TaskRunner
#include <fprime-baremetal/Os/TaskRunner/TaskRunner.hpp>

// Used for logging
#include <Arduino/Os/Console.hpp>

// Stack space for the core's linker script to reserve at the top of RAM, so a link that leaves less than this for the
// stack fails. The script counts every .stack section; nobits keeps it out of the image. setup() refers to it so the
// linker doesn't discard it.
static U8 s_stackReserve[2048] __attribute__((section(".stack,\"aw\",%nobits@")));

/**
 * \brief setup the program
 *
 * This is an extraction of the Arduino setup() function.
 * 
 */
void setup() {
    asm volatile("" : : "r"(s_stackReserve));

    // Initialize OSAL
    Os::init();

    // Setup console logging on Serial3 (SERCOM4)
    Serial3.begin(115200);
    static_cast<Os::Arduino::StreamConsoleHandle*>(Os::Console::getSingleton().getHandle())->setStreamHandler(Serial3);

    // Object for communicating state to the reference topology
    SAMD21Demo::TopologyState inputs;
    inputs.uartNumber = 1;
    inputs.uartBaud = 115200;

    // Setup topology
    SAMD21Demo::setupTopology(inputs);

    Fw::Logger::log("Program Started\n");
}

/**
 * \brief run the program
 *
 * This is an extraction of the Arduino loop() function.
 * 
 */
void loop() {
#ifdef USE_BASIC_TIMER
    rateDriver.cycle();
#endif
    Os::Baremetal::TaskRunner::getSingleton().run();
}
