#ifndef INTERFACES_H
#define INTERFACES_H

#include <string>

//
// IWF-MAC
//
template <typename iwf>
struct iwf_mac_sap_user
{
  void receive (std::string ipdu)
  {
    derived ()->receive (ipdu);
  }
  iwf* derived () { return static_cast<iwf*>(this); }
};

template <typename mac>
struct iwf_mac_sap_provider
{
  virtual void send (std::string msdu)
  {
    derived ()->send (msdu);
  }
  mac* derived () { return static_cast<mac*>(this); }
};

//
// MAC -PHY
//
template <typename mac>
struct mac_phy_sap_user
{
  void receive (std::string mpdu)
  {
    derived ()->receive (mpdu);
  }
  mac* derived () { return static_cast<mac*>(this); }
};

template <typename phy>
struct mac_phy_sap_provider
{
  void send (std::string psdu)
  {
    derived ()-> send (psdu);
  }
  phy* derived () { return static_cast<phy*>(this); }
};

#endif
