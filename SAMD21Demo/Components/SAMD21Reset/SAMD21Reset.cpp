// ======================================================================
// \title  SAMD21Reset.cpp
// \brief  cpp file for SAMD21Reset component implementation class
// ======================================================================

#include "SAMD21Demo/Components/SAMD21Reset/SAMD21Reset.hpp"
#include <sam.h>
#include <stddef.h>
#include <Arduino/Os/Console.hpp>
#include <Fw/Logger/Logger.hpp>

namespace SAMD21Demo {

namespace {
// ----------------------------------------------------------------------
// Reset record, kept in .noinit RAM across the reset
// ----------------------------------------------------------------------

// What the record holds. Asserts, hard faults and REBOOT all reset through a system reset request, so
// the record tells them apart from each other and from a debugger's reset.
constexpr U32 ASSERT_MAGIC = 0xA55E27ED;
constexpr U32 FAULT_MAGIC = 0xFA017ED0;
constexpr U32 REBOOT_MAGIC = 0x2EB0071D;

struct ResetRecord {
    U32 magic;
    U32 fileId;
    U32 lineNo;
    U32 faultPc;
    U8 numArgs;
    I32 args[AssertArgs::SIZE];
    U16 check;  //!< Fletcher-16 of everything above, so leftover RAM contents are not mistaken for a record
};
ResetRecord s_record __attribute__((section(".noinit")));

U16 recordCheck(const ResetRecord& record) {
    const U8* bytes = reinterpret_cast<const U8*>(&record);
    U8 sum1 = 0;
    U8 sum2 = 0;
    for (FwSizeType i = 0; i < offsetof(ResetRecord, check); i++) {
        sum1 = static_cast<U8>(sum1 + bytes[i]);
        sum2 = static_cast<U8>(sum2 + sum1);
    }
    return static_cast<U16>((sum2 << 8) | sum1);
}

void saveRecord(U32 magic) {
    s_record.magic = magic;
    s_record.check = recordCheck(s_record);
}

// ----------------------------------------------------------------------
// Watchdog
// ----------------------------------------------------------------------

void watchdogSync() {
    while (WDT->STATUS.bit.SYNCBUSY) {
    }
}

void watchdogEnable() {
    // Clock the watchdog at 1.024 kHz: generic clock generator 2 (which the Arduino core leaves alone) from the
    // 32 kHz ultra low power oscillator, divided by 2^(4 + 1)
    GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4);
    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2;

    // CONFIG can only be written while the watchdog is disabled
    WDT->CTRL.reg = 0;
    watchdogSync();
    WDT->CONFIG.reg = WDT_CONFIG_PER_2K;  // 2048 cycles: 2 s
    WDT->CTRL.reg = WDT_CTRL_ENABLE;
    watchdogSync();
}

void watchdogDisable() {
    WDT->CTRL.reg = 0;
    watchdogSync();
}
}  // namespace

// ----------------------------------------------------------------------
// Hard faults
// ----------------------------------------------------------------------

extern "C" {
//! Saves the faulting instruction's address and resets. frame is the exception stack frame.
void samd21Reset_hardFault(const U32* frame) __attribute__((used, noreturn));
void samd21Reset_hardFault(const U32* frame) {
    s_record.faultPc = frame[6];  // r0, r1, r2, r3, r12, lr, pc, xPSR
    saveRecord(FAULT_MAGIC);
    NVIC_SystemReset();
}

//! Replaces the Arduino core's handler, which hangs. Passes the stack frame from whichever stack was in use.
void HardFault_Handler() __attribute__((naked));
void HardFault_Handler() {
    asm volatile(
        "movs r0, #4\n"
        "mov r1, lr\n"
        "tst r0, r1\n"  // EXC_RETURN bit 2: 0 for the main stack, 1 for the process stack
        "mrs r0, msp\n"
        "beq 1f\n"
        "mrs r0, psp\n"
        "1:\n"
        "bl samd21Reset_hardFault\n");
}
}

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

SAMD21Reset ::SAMD21Reset(const char* const compName)
    : SAMD21ResetComponentBase(compName),
      m_resetReason(ResetReason::UNKNOWN),
      m_resetCause(PM->RCAUSE.reg),
      m_faultPc(0),
      m_watchdogEnabled(false) {
    this->m_hook.registerHook();

    // RCAUSE holds only the cause of the last reset. Its contents are kept by every reset but power-on and
    // brown-out, which make the record meaningless.
    const U8 cause = this->m_resetCause;
    const bool recorded = (s_record.check == recordCheck(s_record));
    const U32 magic = recorded ? s_record.magic : 0;
    if (cause & PM_RCAUSE_POR) {
        this->m_resetReason = ResetReason::POWER_ON;
    } else if (cause & (PM_RCAUSE_BOD12 | PM_RCAUSE_BOD33)) {
        this->m_resetReason = ResetReason::BROWN_OUT;
    } else if (cause & PM_RCAUSE_WDT) {
        this->m_resetReason = ResetReason::WATCHDOG;
    } else if (cause & PM_RCAUSE_SYST) {
        this->m_resetReason = (magic == ASSERT_MAGIC)   ? ResetReason::ASSERT
                              : (magic == FAULT_MAGIC)  ? ResetReason::HARD_FAULT
                              : (magic == REBOOT_MAGIC) ? ResetReason::COMMANDED
                                                        : ResetReason::SYSTEM;
    } else if (cause & PM_RCAUSE_EXT) {
        this->m_resetReason = ResetReason::RESET_PIN;
    }

    if (this->m_resetReason == ResetReason::ASSERT) {
        AssertArgs args;
        for (U8 i = 0; i < AssertArgs::SIZE; i++) {
            args[i] = s_record.args[i];
        }
        this->m_lastAssert.set(s_record.fileId, s_record.lineNo, s_record.numArgs, args);
    } else if (this->m_resetReason == ResetReason::HARD_FAULT) {
        this->m_faultPc = s_record.faultPc;
    }
    s_record.magic = 0;  // Report each record once
}

SAMD21Reset ::~SAMD21Reset() {}

// ----------------------------------------------------------------------
// Parameters
// ----------------------------------------------------------------------

void SAMD21Reset ::parametersLoaded() {
    this->tlmWrite_ResetReason(this->m_resetReason);
    this->tlmWrite_ResetCause(this->m_resetCause);
    this->tlmWrite_LastAssert(this->m_lastAssert);
    this->tlmWrite_FaultPc(this->m_faultPc);
    this->applyWatchdogParam();
}

void SAMD21Reset ::parameterUpdated(FwPrmIdType id) {
    if (id == PARAMID_WATCHDOG_ENABLED) {
        this->applyWatchdogParam();
    }
}

void SAMD21Reset ::applyWatchdogParam() {
    Fw::ParamValid valid;
    this->m_watchdogEnabled = (this->paramGet_WATCHDOG_ENABLED(valid) == Fw::Enabled::ENABLED);
    if (this->m_watchdogEnabled) {
        watchdogEnable();
    } else {
        watchdogDisable();
    }
    this->tlmWrite_WatchdogEnabled(this->m_watchdogEnabled ? Fw::Enabled::ENABLED : Fw::Enabled::DISABLED);
}

// ----------------------------------------------------------------------
// Ports and commands
// ----------------------------------------------------------------------

void SAMD21Reset ::run_handler(FwIndexType portNum, U32 context) {
    // A clear is still synchronizing for a few ms after it's written. Writing another then would stall the bus.
    if (this->m_watchdogEnabled && !WDT->STATUS.bit.SYNCBUSY) {
        WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    }
}

void SAMD21Reset ::REBOOT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // The response can't reach the ground before the reset; ResetReason reports the reboot instead
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    saveRecord(REBOOT_MAGIC);
    NVIC_SystemReset();
}

void SAMD21Reset ::TEST_ASSERT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq, I32 arg1, I32 arg2) {
    FW_ASSERT(0, arg1, arg2);
}

void SAMD21Reset ::TEST_FAULT_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    // Cortex-M0+ faults on unaligned word loads
    const volatile U32* unaligned =
        reinterpret_cast<const volatile U32*>(reinterpret_cast<uintptr_t>(&this->m_faultPc) + 1);
    (void)*unaligned;
}

void SAMD21Reset ::TEST_HANG_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    if (!this->m_watchdogEnabled) {
        this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::EXECUTION_ERROR);
        return;
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
    for (;;) {
        asm volatile("");  // Keeps the compiler from assuming the loop ends
    }
}

// ----------------------------------------------------------------------
// Assert hook
// ----------------------------------------------------------------------

void SAMD21Reset::Hook ::reportAssert(FILE_NAME_ARG file,
                                      FwSizeType lineNo,
                                      FwSizeType numArgs,
                                      FwAssertArgType arg1,
                                      FwAssertArgType arg2,
                                      FwAssertArgType arg3,
                                      FwAssertArgType arg4,
                                      FwAssertArgType arg5,
                                      FwAssertArgType arg6) {
    static_assert(sizeof(FwAssertArgType) == sizeof(I32), "Assert args are stored as I32");
#if FW_ASSERT_LEVEL == FW_FILEID_ASSERT
    s_record.fileId = file;
#else
    s_record.fileId = 0;  // Only file ID asserts have a number to keep
#endif
    s_record.lineNo = static_cast<U32>(lineNo);
    s_record.faultPc = 0;
    s_record.numArgs = static_cast<U8>(numArgs);
    s_record.args[0] = arg1;
    s_record.args[1] = arg2;
    s_record.args[2] = arg3;
    s_record.args[3] = arg4;
    saveRecord(ASSERT_MAGIC);

    // Print it on the console (the stream Main.cpp gives it), unless printing is what asserted. The Arduino
    // core's UART only drains through its interrupt, so this only waits for the output in thread mode with
    // interrupts enabled (everything run from loop()). Elsewhere the end of the message may be lost.
    static bool s_printing = false;
    const bool canFlush = ((__get_PRIMASK() & 1) == 0) && ((SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) == 0);
    if (!s_printing && canFlush) {
        s_printing = true;
        const ResetRecord& r = s_record;
#if FW_ASSERT_LEVEL == FW_FILEID_ASSERT
        Fw::Logger::log(
            "ASSERT file 0x%08" PRIx32 " line %" PRIu32 " args %" PRId32 " %" PRId32 " %" PRId32 " %" PRId32 " (%u)\n",
            r.fileId, r.lineNo, r.args[0], r.args[1], r.args[2], r.args[3], static_cast<unsigned>(r.numArgs));
#else
        Fw::Logger::log("ASSERT %s line %" PRIu32 " args %" PRId32 " %" PRId32 " %" PRId32 " %" PRId32 " (%u)\n", file,
                        r.lineNo, r.args[0], r.args[1], r.args[2], r.args[3], static_cast<unsigned>(r.numArgs));
#endif
        Stream* const console =
            static_cast<Os::Arduino::StreamConsoleHandle*>(Os::Console::getSingleton().getHandle())->m_stream;
        if (console != nullptr) {
            console->flush();
        }
    }
}

void SAMD21Reset::Hook ::doAssert() {
    NVIC_SystemReset();
}

}  // namespace SAMD21Demo
