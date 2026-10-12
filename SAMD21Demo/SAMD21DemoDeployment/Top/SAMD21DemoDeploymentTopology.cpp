// ======================================================================
// \title  SAMD21DemoDeploymentTopology.cpp
// \brief cpp file containing the topology instantiation code
//
// ======================================================================
// Provides access to autocoded functions
#include <SAMD21Demo/SAMD21DemoDeployment/Top/SAMD21DemoDeploymentTopologyAc.hpp>
#include <SAMD21Demo/SAMD21DemoDeployment/Top/SAMD21DemoDeployment_SAMD21DemoPacketsTlmPacketsAc.hpp>
#include <config/FppConstantsAc.hpp>
#include <Fw/Logger/Logger.hpp>

// Necessary project-specified types
#include <Arduino/config/FprimeArduino.hpp>

// Allows easy reference to objects in FPP/autocoder required namespaces
using namespace SAMD21Demo;

// Base tick of the system, polled from loop() (USE_BASIC_TIMER)
static constexpr U32 BASE_TICK_MS = 100;

// The rate group driver divides the 10Hz base tick into sub-signals: 10Hz and 1Hz
Svc::RateGroupDriver::DividerSet rateGroupDivisors{{{1, 0}, {10, 0}}};

// Rate groups may supply a context token to each of the attached children whose purpose is set by the project. The
// reference topology sets each token to zero as these contexts are unused in this project.
Svc::PassiveRateGroup::ContextArray rateGroup10HzContext(0);
Svc::PassiveRateGroup::ContextArray rateGroup1HzContext(0);

/**
 * \brief configure/setup components in project-specific way
 *
 * This is a *helper* function which configures/sets up each component requiring project specific input. This includes
 * allocating resources, passing-in arguments, etc. This function may be inlined into the topology setup function if
 * desired, but is extracted here for clarity.
 */
void configureTopology() {
    // Rate group driver needs a divisor list
    rateGroupDriver.configure(rateGroupDivisors);

    // Rate groups require context arrays.
    rateGroup10Hz.configure(rateGroup10HzContext);
    rateGroup1Hz.configure(rateGroup1HzContext);

    tlmSend.setPacketList(SAMD21DemoDeployment_SAMD21DemoPacketsTlmPackets::packetList);

    lifeLed.configure(Arduino::DEF_LED_BUILTIN);
}

// Public functions for use in main program are namespaced with deployment namespace SAMD21Demo
namespace SAMD21Demo {
void setupTopology(const TopologyState& state) {
    // Autocoded initialization. Function provided by autocoder.
    initComponents(state);
    // Autocoded id setup. Function provided by autocoder.
    setBaseIds();
    // Autocoded connection wiring. Function provided by autocoder.
    connectComponents();
    // Autocoded configuration. Function provided by autocoder.
    configComponents(state);
    // Project-specific component configuration. Function provided above. May be inlined, if desired.
    configureTopology();
    // Autocoded command registration. Function provided by autocoder.
    regCommands();
    // Autocoded parameter loading. Function provided by autocoder. Parameters have no storage yet, so these are
    // the defaults (prmDb).
    loadParameters();
    // Autocoded task kick-off (active components). Function provided by autocoder.
    startTasks(state);

    // Space Packet link on Serial1 (SERCOM0). Serial (SERCOM5) is used for console output.
    Serial1.begin(state.uartBaud);
    comDriver.configure(&Serial1);
    
    rateDriver.configure(BASE_TICK_MS);
    rateDriver.start();
}

void teardownTopology(const TopologyState& state) {
    // Autocoded (active component) task clean-up. Functions provided by topology autocoder.
    stopTasks(state);
    freeThreads(state);
}
};  // namespace SAMD21Demo
