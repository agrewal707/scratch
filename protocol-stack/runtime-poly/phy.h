#ifndef PHY_H
#define PHY_H

#include "log.h"
#include "interfaces.h"

class phy :
  public mac_phy_sap_provider
{
public:
  phy ()
  {}

  void set_mac_phy_sap_user (mac_phy_sap_user *u)
  {
    m_u = u;
  }

  void send (std::string psdu)
  {
    auto ppdu = "PHY:" + psdu;
    LOG (ppdu);
  }

  void receive (std::string ppdu)
  {
    // process ppdu
    // extract mpdu
    auto mpdu = ppdu + ":PHY";
    m_u->receive (mpdu);
  }

private:
  mac_phy_sap_user *m_u;
};

#endif
