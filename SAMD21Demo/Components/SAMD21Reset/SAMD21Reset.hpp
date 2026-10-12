// ======================================================================
// \title  SAMD21Reset.hpp
// \brief  hpp file for SAMD21Reset component implementation class
// ======================================================================

#ifndef SAMD21Demo_SAMD21Reset_HPP
#define SAMD21Demo_SAMD21Reset_HPP

#include "Fw/Types/Assert.hpp"
#include "SAMD21Demo/Components/SAMD21Reset/SAMD21ResetComponentAc.hpp"

namespace SAMD21Demo {

class SAMD21Reset final : public SAMD21ResetComponentBase {
  public:
    //! Registers the assert hook, so asserts reset the MCU from construction on
    SAMD21Reset(const char* const compName);
    ~SAMD21Reset();

  private:
    //! Saves the assert in .noinit RAM and prints it on the console, then resets
    class Hook : public Fw::AssertHook {
      public:
        void reportAssert(FILE_NAME_ARG file,
                          FwSizeType lineNo,
                          FwSizeType numArgs,
                          FwAssertArgType arg1,
                          FwAssertArgType arg2,
                          FwAssertArgType arg3,
                          FwAssertArgType arg4,
                          FwAssertArgType arg5,
                          FwAssertArgType arg6) override;
        void doAssert() override;
    };

    void run_handler(FwIndexType portNum, U32 context) override;
    void REBOOT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void TEST_ASSERT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, I32 arg1, I32 arg2) override;
    void TEST_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;
    void TEST_HANG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) override;

    //! Report the reset and start or stop the watchdog once parameters are available
    void parametersLoaded() override;
    void parameterUpdated(FwPrmIdType id) override;

    //! Start or stop the watchdog to match WATCHDOG_ENABLED
    void applyWatchdogParam();

    Hook m_hook;
    ResetReason m_resetReason;
    U8 m_resetCause;
    AssertInfo m_lastAssert;
    U32 m_faultPc;
    bool m_watchdogEnabled;
};

}  // namespace SAMD21Demo

#endif
