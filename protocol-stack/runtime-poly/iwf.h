#ifndef IWF_H
#define IWF_H

#include "log.h"
#include "interfaces.h"

class iwf :
  public iwf_mac_sap_user
{
public:
  iwf ()
  {}

  void set_iwf_mac_sap_provider (iwf_mac_sap_provider *p)
  {
    m_p = p;
  }

  void send (std::string isdu)
  {
    auto msdu = "IWF:" + isdu;
    m_p->send (msdu);
  }

  virtual void receive (std::string ipdu)
  {
    LOG (ipdu + ":IWF");
  }

private:
  iwf_mac_sap_provider *m_p;
};

#endif
