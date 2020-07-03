#include "iwf.h"
#include "mac.h"
#include "phy.h"

#ifndef BENCHMARK_PERFORMANCE

int main (void)
{
  iwf i;
  mac m;
  phy p;

  i.set_iwf_mac_sap_provider (&m);
  m.set_iwf_mac_sap_user (&i);
  m.set_mac_phy_sap_provider (&p);
  p.set_mac_phy_sap_user (&m);

  p.receive ("RX-DATA");
  i.send ("TX-DATA");

  return 0;
}

#else

#include <benchmark/benchmark.h>

#define REPEAT2(x) x x
#define REPEAT4(x) REPEAT2(x) REPEAT2(x)
#define REPEAT8(x) REPEAT4(x) REPEAT4(x)
#define REPEAT16(x) REPEAT8(x) REPEAT8(x)
#define REPEAT32(x) REPEAT16(x) REPEAT16(x)
#define REPEAT(x) REPEAT32(x)

void BM_stack (benchmark::State &state)
{
  iwf i;
  mac m;
  phy p;

  i.set_iwf_mac_sap_provider (&m);
  m.set_iwf_mac_sap_user (&i);
  m.set_mac_phy_sap_provider (&p);
  p.set_mac_phy_sap_user (&m);

  for (auto _ : state)
  {
    REPEAT(p.receive ("RX-DATA");)
    REPEAT(i.send ("TX-DATA");)
  }
  state.SetItemsProcessed(32*state.iterations());
}

BENCHMARK(BM_stack);

BENCHMARK_MAIN();

#endif
