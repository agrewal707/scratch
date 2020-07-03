#ifndef IWF_H
#define IWF_H

#include "log.h"
#include "interfaces.h"

class mac;

class iwf :
  public iwf_mac_sap_user<iwf>
{
public:
  iwf ()
  {}

  void set_iwf_mac_sap_provider (iwf_mac_sap_provider<mac> *p)
  {
    m_p = p;
  }

  void send (std::string isdu)
  {
    auto msdu = "IWF:" + isdu;
    m_p->send (msdu);
  }

  void receive (std::string ipdu)
  {
    LOG (ipdu + ":IWF");
  }

private:
  iwf_mac_sap_provider<mac> *m_p;
};

#endif
