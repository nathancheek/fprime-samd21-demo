module SAMD21Demo {
    @ Parameter database with no storage, until the deployment has one. Every component gets
    @ its parameter defaults at startup, and PRM_SET changes a value until the next reset.
    @
    @ It has no prmSet port, so wire each component's prmGetOut to it by hand instead of
    @ with a param connections pattern. A component's PRM_SAVE commands then fail (counted
    @ in cmdDisp.CommandErrors) rather than reporting a save that didn't happen.
    passive component DefaultPrmDb {

        @ Parameter requests from components. Always answers that there is no stored value.
        sync input port getPrm: Fw.PrmGet

    }
}
