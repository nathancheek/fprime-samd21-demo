module SAMD21Demo {

  # ----------------------------------------------------------------------
  # Symbolic constants for port numbers
  # ----------------------------------------------------------------------

    enum Ports_RateGroups {
      rateGroup10Hz
      rateGroup1Hz
    }

  deployment topology SAMD21DemoDeployment {

    # ----------------------------------------------------------------------
    # Instances used in the topology
    # ----------------------------------------------------------------------

    instance cmdDisp
    instance comDriver
    instance comSpacePacket
    instance rateDriver
    instance rateGroup10Hz
    instance rateGroup1Hz
    instance rateGroupDriver
    instance timeHandler
    instance tlmSend
    instance lifeLed
    instance prmDb
    instance resetHandler

    # ----------------------------------------------------------------------
    # Telemetry packets
    # ----------------------------------------------------------------------

    include "SAMD21DemoPackets.fppi"

    # ----------------------------------------------------------------------
    # Pattern graph specifiers
    # ----------------------------------------------------------------------

    command connections instance cmdDisp

    telemetry connections instance tlmSend

    time connections instance timeHandler

    # ----------------------------------------------------------------------
    # Direct graph specifiers
    # ----------------------------------------------------------------------

    connections RateGroups {
      # Polled from loop(). Rate groups run from the main loop.
      rateDriver.CycleOut -> rateGroupDriver.CycleIn

      # 10 Hz rate group: poll the link UART
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup10Hz] -> rateGroup10Hz.CycleIn
      rateGroup10Hz.RateGroupMemberOut[0] -> comDriver.schedIn
      # Pet the watchdog (2 s timeout)
      rateGroup10Hz.RateGroupMemberOut[1] -> resetHandler.run
      # Blink the built-in LED once a second (LED_PERIOD of 10 ticks)
      rateGroup10Hz.RateGroupMemberOut[2] -> lifeLed.run

      # 1 Hz rate group: telemetry
      rateGroupDriver.CycleOut[Ports_RateGroups.rateGroup1Hz] -> rateGroup1Hz.CycleIn
      rateGroup1Hz.RateGroupMemberOut[0] -> tlmSend.Run
    }

    connections Communications {
      # ComDriver <-> ComSpacePacket. ComSpacePacket lends the driver its receive buffer.
      comDriver.allocate          -> comSpacePacket.drvAllocateIn
      comDriver.deallocate        -> comSpacePacket.drvDeallocateIn
      comDriver.$recv             -> comSpacePacket.drvReceiveIn
      comSpacePacket.drvReceiveReturnOut -> comDriver.recvReturnIn
      comSpacePacket.drvSendOut   -> comDriver.$send

      # Received commands
      comSpacePacket.comCmdOut -> cmdDisp.seqCmdBuff

      # Sent telemetry
      tlmSend.PktSend -> comSpacePacket.comIn
    }

    connections Parameters {
      # Parameters have no storage yet (see DefaultPrmDb): components use their defaults, and PRM_SAVE fails
      lifeLed.prmGetOut -> prmDb.getPrm
      resetHandler.prmGetOut -> prmDb.getPrm
    }

    connections SAMD21DemoDeployment {
      # Add here connections to user-defined components
    }

  }

}
