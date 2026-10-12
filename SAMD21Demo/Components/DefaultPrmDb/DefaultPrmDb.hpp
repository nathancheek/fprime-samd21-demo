// ======================================================================
// \title  DefaultPrmDb.hpp
// \brief  hpp file for DefaultPrmDb component implementation class
// ======================================================================

#ifndef SAMD21Demo_DefaultPrmDb_HPP
#define SAMD21Demo_DefaultPrmDb_HPP

#include "SAMD21Demo/Components/DefaultPrmDb/DefaultPrmDbComponentAc.hpp"

namespace SAMD21Demo {

class DefaultPrmDb final : public DefaultPrmDbComponentBase {
  public:
    DefaultPrmDb(const char* const compName);
    ~DefaultPrmDb();

  private:
    Fw::ParamValid getPrm_handler(FwIndexType portNum, FwPrmIdType id, Fw::ParamBuffer& val) override;
};

}  // namespace SAMD21Demo

#endif
