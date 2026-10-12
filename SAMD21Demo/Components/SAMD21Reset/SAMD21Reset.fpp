module SAMD21Demo {

    @ Why the MCU last reset
    enum ResetReason : U8 {
        UNKNOWN = 0 @< No reset cause was set
        POWER_ON = 1
        RESET_PIN = 2
        BROWN_OUT = 3 @< 1.2 V or 3.3 V brown-out detector
        WATCHDOG = 4 @< Watchdog timeout: the main loop stopped petting the watchdog
        @ System reset request from something other than this component, such as a debugger
        SYSTEM = 5
        ASSERT = 6 @< FW_ASSERT
        COMMANDED = 7 @< The REBOOT command
        HARD_FAULT = 8 @< A hard fault, such as an unaligned access. FaultPc has its address.
    }

    @ Arguments of the last assert
    array AssertArgs = [4] I32

    @ The assert that caused the last reset
    struct AssertInfo {
        fileId: U32 format "0x{x}" @< Hash of the file's path (look up with fprime-util hash-to-file)
        lineNo: U32
        numArgs: U8 @< Arguments passed to FW_ASSERT; only the first 4 are kept
        args: AssertArgs
    }

    @ Reset handling for SAMD21 MCUs. Reports why the MCU last reset, resets on FW_ASSERT
    @ (printing the assert on the console) and on hard faults, keeping their details in RAM
    @ that survives the reset, and runs the watchdog, which the run port pets.
    passive component SAMD21Reset {

        @ Pets the watchdog. Call it well within the 2 s timeout.
        sync input port run: Svc.Sched

        @ Whether the watchdog runs (2 s timeout). Takes effect immediately on PRM_SET.
        param WATCHDOG_ENABLED: Fw.Enabled default Fw.Enabled.ENABLED

        @ Reboot the MCU. ResetReason reads COMMANDED afterwards.
        sync command REBOOT

        @ Cause an assert with the given arguments, to test assert handling
        sync command TEST_ASSERT(
            arg1: I32
            arg2: I32
        )

        @ Cause a hard fault (an unaligned load), to test fault handling
        sync command TEST_FAULT

        @ Stop the main loop to test the watchdog. Fails if the watchdog is disabled.
        sync command TEST_HANG

        @ Why the MCU last reset
        telemetry ResetReason: ResetReason id 0

        @ PM RCAUSE register at boot: SYST 0x40, WDT 0x20, EXT 0x10, BOD33 0x04, BOD12 0x02, POR 0x01
        telemetry ResetCause: U8 id 1 format "0x{x}"

        @ The assert that caused the last reset, or all zeros if the last reset was not an assert
        telemetry LastAssert: AssertInfo id 2

        @ Whether the watchdog is running
        telemetry WatchdogEnabled: Fw.Enabled id 3

        @ Address of the instruction that caused the last reset's hard fault, or 0
        telemetry FaultPc: U32 id 4 format "0x{x}"

        @ Port for requesting the current time
        time get port timeCaller

        @ Port for sending command registrations
        command reg port cmdRegOut

        @ Port for receiving commands
        command recv port cmdIn

        @ Port for sending command responses
        command resp port cmdResponseOut

        @ Port for sending telemetry channels to downlink
        telemetry port tlmOut

        @ Port to return the value of a parameter
        param get port prmGetOut

        @ Port to set the value of a parameter
        param set port prmSetOut
    }
}
