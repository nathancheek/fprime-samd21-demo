// ======================================================================
// \title  DefaultPrmDb.cpp
// \brief  cpp file for DefaultPrmDb component implementation class
// ======================================================================

#include "SAMD21Demo/Components/DefaultPrmDb/DefaultPrmDb.hpp"

namespace SAMD21Demo {

DefaultPrmDb ::DefaultPrmDb(const char* const compName) : DefaultPrmDbComponentBase(compName) {}

DefaultPrmDb ::~DefaultPrmDb() {}

Fw::ParamValid DefaultPrmDb ::getPrm_handler(FwIndexType portNum, FwPrmIdType id, Fw::ParamBuffer& val) {
    // Nothing is stored, so the component uses its default
    return Fw::ParamValid::INVALID;
}

}  // namespace SAMD21Demo
