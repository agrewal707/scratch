#ifndef MAC_H
#define MAC_H

#include "interfaces.h"

class mac :
  public mac_phy_sap_user,
  public iwf_mac_sap_provider
{
public:
  mac ()
  {}
  void set_mac_phy_sap_provider (mac_phy_sap_provider *p)
  {
    m_p = p;
  }
  void set_iwf_mac_sap_user (iwf_mac_sap_user *u)
  {
    m_u = u;
  }

  void send (std:: string msdu)
  {
    // create MPDU - add MAC header
    std::string mpdu = "MAC:" + msdu;
    m_p->send (mpdu);
  }

  virtual void receive (std::string mpdu)
  {
    // process mpdu
    // extract isdu
    std::string isdu = mpdu + ":MAC";
    m_u->receive (isdu);
  }

private:
  mac_phy_sap_provider *m_p;
  iwf_mac_sap_user *m_u;
};

#endif
