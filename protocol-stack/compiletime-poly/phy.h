#ifndef PHY_H
#define PHY_H

#include "interfaces.h"

class mac;

class phy :
  public mac_phy_sap_provider<phy>
{
public:
  phy ()
  {}

  void set_mac_phy_sap_user (mac_phy_sap_user<mac> *u)
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
  mac_phy_sap_user<mac> *m_u;
};

#endif
