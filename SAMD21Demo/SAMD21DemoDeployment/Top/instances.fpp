module SAMD21Demo {

  # ----------------------------------------------------------------------
  # Passive component instances
  #
  # No active components: everything runs from the rate groups in loop().
  # ----------------------------------------------------------------------

  instance cmdDisp: SAMD21Demo.LiteCmdDispatcher base id 0x0100

  instance tlmSend: SAMD21Demo.PassiveTlmPacketizer base id 0x0400

  instance prmDb: SAMD21Demo.DefaultPrmDb base id 0x0700

  instance resetHandler: SAMD21Demo.SAMD21Reset base id 0x0800

  instance lifeLed: Arduino.LifeLed base id 0x0E00

  instance rateGroup10Hz: Svc.PassiveRateGroup base id 0x1000

  instance rateGroup1Hz: Svc.PassiveRateGroup base id 0x1100

  instance comSpacePacket: SAMD21Demo.ComSpacePacket base id 0x2000

  @ Communications driver. May be swapped with other com drivers like Arduino.StreamDriver, Arduino.TcpServer, or Arduino.TcpClient.
  instance comDriver: Arduino.StreamDriver base id 0x4000

  instance timeHandler: Arduino.ArduinoTime base id 0x4400

  instance rateGroupDriver: Svc.RateGroupDriver base id 0x4500

  instance rateDriver: Arduino.HardwareRateDriver base id 0x4900

}
