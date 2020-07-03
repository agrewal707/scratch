#ifndef INTERFACES_H
#define INTERFACES_H

#include <string>

//
// IWF-MAC
//
struct iwf_mac_sap_user
{
  virtual void receive (std::string ipdu) = 0;
};

struct iwf_mac_sap_provider
{
  virtual void send (std::string msdu) = 0;
};

//
// MAC -PHY
//
struct mac_phy_sap_user
{
  virtual void receive (std::string mpdu) = 0;

};

struct mac_phy_sap_provider
{
  virtual void send (std::string psdu) = 0;
};

#endif
