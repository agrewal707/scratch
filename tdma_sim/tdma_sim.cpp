/*
MIT License

Copyright (c) [2020] [Ajay Pal Singh Grewal]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
*
Build:
$ g++ -Werror -std=c++11 -o tdma_sim tdma_sim.cpp

Run:
$ ./tdma_sim
*
*/


#include <cstdio>
#include <cstring>
#include <ctime>
#include <vector>
#include <memory>
#include <unordered_map>

#include "simulator.h"

namespace tdma {

struct config
{
  // beacon interval (us)
  uint32_t interval = 10000;
  // phase adjustment factor
  float alpha =  0.5f;
  // frequency adjustment factor
  float beta = 0.5*alpha*alpha;
  // clock frequency offset
  float cfo = 0.2f;
  // propagation delay
  uint32_t prop_delay = 0;
  // message drop probability
  float drop_prob = 0;
  // max slots per frame
  uint8_t slots_per_frame = 10;
  // slot duration (us)
  uint32_t slot_duration = 1000;
  // guard time
  uint32_t guard_time = 10;
  // enable debug logs
  bool debug = false;
};

//
// mac messages
//
struct slot
{
  slot ():
    m_rid (0),
    m_ul (false)
  {}

  // remote id to which this slot is assigned
  uint16_t m_rid;
  // true for uplink slot
  bool m_ul;
};
using slot_schedule = std::vector<slot>;

struct message
{
  enum Type
  {
    BEACON,
    DATA,
    ACK // TODO
  };

  message (Type type, uint16_t src_rid, uint16_t dst_rid):
    m_id (m_id_counter),
    m_type (type),
    m_src_rid (src_rid),
    m_dst_rid (dst_rid)
  {
    ++m_id_counter;
  }

  virtual ~message ()
  {}

  uint32_t m_id;
  Type m_type;
  uint16_t m_src_rid;
  uint16_t m_dst_rid;

  static uint32_t m_id_counter;
};
uint32_t message::m_id_counter = 1;

struct beacon : public message
{
  beacon (ns::time::point tx):
    message (message::BEACON, 0, 0xFFFF),
    m_tx_time (tx),
    m_beacon_id (m_beacon_id_counter)
  {
    ++m_beacon_id_counter;
  }
  ns::time::point m_tx_time;
  slot_schedule m_slot_schedule;
  // this helps remote determine #beacons missed
  uint32_t m_beacon_id;

  static uint32_t m_beacon_id_counter;
};
uint32_t beacon::m_beacon_id_counter = 1;

struct data : public message
{
  data (uint16_t src_rid, uint16_t dst_rid):
    message (message::DATA, src_rid, dst_rid)
  {}
};

//
// wireless channel
//
class channel
{
public:
  //
  // wireless device/node on this channel
  //
  class node
  {
  public:
    node () :
      m_rid (m_rid_counter)
    {
      m_rid_counter++;
    }

  void virtual receive (std::shared_ptr<message> msg) = 0;

  protected:
    friend class channel;
    uint16_t m_rid;
  };

  channel (const config &cfg):
    m_cfg (cfg)
  {
    srandom (time(nullptr));
  }

  void add (node *n)
  {
    const auto ret = m_node_map.emplace (n->m_rid, n);
    if (!ret.second)
    {
      printf ("node %u exists\n", n->m_rid);
      exit (-1);
    }
  }
  void remove (node *n)
  {
    const auto it = m_node_map.find (n->m_rid);
    if (it != m_node_map.end ())
    {
      m_node_map.erase (it);
    }
    else
    {
      printf ("unknown node %u\n", n->m_rid);
      exit (-1);
    }
  }

  void send (std::shared_ptr<message> msg)
  {
    float r = float(random())/RAND_MAX;
    if (r > m_cfg.drop_prob)
    {
      ns::simulator::schedule (
        ns::time::now() + ns::time::duration (m_cfg.prop_delay),
        std::bind (&channel::receive, this, msg));
    }
    else
    {
      printf ("--------PACKET DROPPED-------\n");
    }
  }

  void receive (std::shared_ptr<message> msg)
  {
    const auto it = m_node_map.find (msg->m_dst_rid);
    if (it != m_node_map.end ())
    {
      it->second->receive (msg);
    }
    else if (msg->m_dst_rid == 0xFFFF)
    {
      for (auto &val : m_node_map)
      {
        // only send broadcast to remotes
        if (val.second->m_rid)
          val.second->receive (msg);
      }
    }
    else
    {
      printf ("unknown node %u\n", msg->m_dst_rid);
    }
  }

private:
  const config &m_cfg;
  // list of nodes/devices on the channel
  std::unordered_map<uint16_t, node*> m_node_map;

  static uint16_t m_rid_counter;
};
uint16_t channel::m_rid_counter = 0;

//
// access point (ap)
//
class ap : public channel::node
{
public:
  //
  // mac scheduler
  //
  class scheduler
  {
  public:
    scheduler (const config &cfg):
      m_cfg (cfg),
      m_slot_schedule (m_cfg.slots_per_frame)
    {
      // setup a fixed schedule

      // SLOT    : 0  |  1 |     2      |  ..           | 9
      // SCHEDULE: DL | DL | UL (RID 1) | UL RID (2) |..| UL (RID 8)
      for (size_t i  = 0; i < m_slot_schedule.size(); ++i)
      {
        auto &slot = m_slot_schedule[i];
        if (i == 0)
        {
          slot.m_rid = 0;
          slot.m_ul = false;
        }
        else if (i == 1)
        {
          slot.m_rid = 0;
          slot.m_ul = false;
        }
        else
        {
          slot.m_rid = i - 1;
          slot.m_ul = true;
        }
      }
    }

    slot_schedule get_schedule ()
    {
      return m_slot_schedule;
    }

  private:
    const config &m_cfg;
    slot_schedule m_slot_schedule;
  };

  ap (const config &cfg, channel &chan):
    m_cfg (cfg),
    m_channel (chan),
    m_scheduler (cfg)
  {
    m_channel.add (this);
    tx_beacon ();
  }

  ~ap ()
  {
    m_channel.remove (this);
  }

  virtual void receive (std::shared_ptr<message> msg)
  {
    auto now = ns::time::now ();
    if (msg->m_type == message::DATA)
    {
      rx_data (std::static_pointer_cast<data> (msg), now);
    }
  }

  void tx_beacon ()
  {
    m_beacon_tx_time = ns::time::now ();
    printf ("%10s: %10u\n", "TX BEACON", m_beacon_tx_time.m_val);
    auto b = std::make_shared<beacon> (m_beacon_tx_time);
    b->m_slot_schedule = m_scheduler.get_schedule ();
    m_channel.send (b);

    auto next = m_beacon_tx_time + ns::time::duration (m_cfg.interval * (1 + m_cfg.cfo));
    ns::simulator::schedule (next, std::bind (&ap::tx_beacon, this));
  }

  void rx_data (std::shared_ptr<data> d, const ns::time::point &rx_time)
  {
    auto slot_schedule = m_scheduler.get_schedule ();
    // determine the slot where this packet landed
    auto delta_from_beacon = rx_time - m_beacon_tx_time;
    int slot_i = (delta_from_beacon + m_cfg.guard_time) / m_cfg.slot_duration;
    if (0 <= slot_i && slot_i < 10)
    {
      auto &slot = slot_schedule[slot_i];
      if (slot.m_rid == d->m_src_rid)
        printf ("%10s: %10u %10d %10u %12s\n", "RX DATA", rx_time.m_val, slot_i, d->m_src_rid, "GOOD");
      else
        printf ("%10s: %10u %10d %10u %12s\n", "RX DATA", rx_time.m_val, slot_i, d->m_src_rid, "VIOLATION");
    }
    else
    {
      printf ("%10s: %10u %10d %10u %12s\n", "RX DATA", rx_time.m_val, slot_i, d->m_src_rid, "VIOLATION");
    }
  }

private:
  const config &m_cfg;
  channel &m_channel;
  scheduler m_scheduler;
  ns::time::point m_beacon_tx_time;
};

//
// remote (rm)
//
class rm : public channel::node
{
public:
  rm (const config &cfg, channel &chan):
    m_cfg (cfg),
    m_channel (chan),
    m_current_time (0),
    m_cfo_est (0.0f),
    m_last_beacon_id (0)
  {
    m_channel.add (this);
  }
  ~rm ()
  {
    m_channel.remove (this);
  }

  virtual void receive (std::shared_ptr<message> msg)
  {
    if (msg->m_type == message::BEACON)
    {
      rx_beacon (std::static_pointer_cast<beacon> (msg));
    }
  }

  void rx_beacon (std::shared_ptr<beacon> b)
  {
    // adjust time to account for missed beacons with last estimated cfo
    const auto missed_beacons = b->m_beacon_id - m_last_beacon_id - 1;
    m_current_time = m_current_time + ns::time::duration (missed_beacons * m_cfg.interval * (1+m_cfo_est));
    m_last_beacon_id = b->m_beacon_id;

    // run pll to adjust time and frequency offset
    const auto time_error  = b->m_tx_time - m_current_time;
    printf ("%10s: %10u %10u %10d %12.8f\n",
      "RX BEACON", b->m_tx_time.m_val, m_current_time.m_val, time_error.m_val, m_cfo_est);

    // adjust our time
    m_current_time = m_current_time + ns::time::duration (m_cfg.alpha * time_error.m_val);
    // adjust our frequency offset with ap
    m_cfo_est += m_cfg.beta * (float(time_error.m_val) / m_cfg.interval);

    process_schedule (b->m_slot_schedule);

    // update current time for next beacon
    m_current_time = m_current_time + ns::time::duration (m_cfg.interval * (1+m_cfo_est));
  }

  void process_schedule (const slot_schedule &slot_schedule)
  {
    for (int i = 0; i < slot_schedule.size (); ++i)
    {
      auto &slot = slot_schedule[i];
      // schedule tx if it is an uplink slot for me
      if (slot.m_rid == m_rid && slot.m_ul)
      {
        auto next = m_current_time + ns::time::duration (i * m_cfg.slot_duration);
        printf ("%10s: %10u %10u %10u\n", "TX DATA", next.m_val, i, m_rid);
        ns::simulator::schedule (next, std::bind (&rm::tx_data, this));
      }
    }
  }

  void tx_data ()
  {
    auto d = std::make_shared<data> (m_rid, 0);
    m_channel.send (d);
  }

private:
  const config &m_cfg;
  channel &m_channel;
  ns::time::point m_current_time;
  float m_cfo_est;
  uint32_t m_last_beacon_id;
};

} // namespace tdma

void usage (const char *name, FILE *f, int ret, const char *info=NULL)
{
  fprintf(f, "Usage: %s [options]\n", name);
  fprintf
    (f,
     "\nOptions:\n"
     "  --runs INT           Number of simulator runs\n"
     "  --interval UINT      Beacon interval\n"
     "  --alpha FLOAT        time adjustment factor\n"
     "  --beta FLOAT         frequency adjustment factor\n"
     "  --cfo FLOAT          AP's clock frequency offset\n"
     "  --prop-delay UINT    Propagation delay\n"
     "  --drop-prob UINT     Message drop/error probablity\n"
     "  -d                   show debug logs\n"
     );
  if (info)
    fprintf(f, "Error while processing '%s'\n", info);
  exit(ret);
}

int main (int C, char *V[])
{
  ns::simulator::config scfg;
  tdma::config cfg;
  for (int i = 1; i < C; ++i)
  {
    if (!strcmp(V[i], "--runs") && i+1<C)
      scfg.num_runs = atoi(V[++i]);
    else if (!strcmp(V[i], "--interval") && i+1<C)
      cfg.interval = atoi(V[++i]);
    else if (!strcmp(V[i], "--alpha") && i+1<C)
      cfg.alpha = atof(V[++i]);
    else if (!strcmp(V[i], "--beta") && i+1<C)
      cfg.beta = atof(V[++i]);
    else if (!strcmp(V[i], "--cfo") && i+1<C)
      cfg.cfo = atof(V[++i]);
    else if (!strcmp(V[i], "--prop-delay") && i+1<C)
      cfg.prop_delay = atoi(V[++i]);
    else if (!strcmp(V[i], "--drop-prob") && i+1<C)
      cfg.drop_prob = atof(V[++i]);
    else if (!strcmp(V[i], "-d"))
      cfg.debug = true;
    else if (!strcmp(V[i], "-h"))
      usage (V[0], stdout, 0);
    else
      usage(V[0], stderr, 1, V[i]);
  }

  // data format
  printf ("%10s:\n", "LOG FORMAT");
  printf ("%10s: %10s\n", "TX BEACON", "TX TIME");
  printf ("%10s: %10s %10s %10s %12s\n", "RX BEACON", "TX TIME", "RX TIME", "ERROR", "CFO EST");
  printf ("%10s: %10s %10s %10s\n", "TX DATA", "RX TIME", "SLOT ID", "RID");
  printf ("%10s: %10s %10s %10s %12s\n\n", "RX DATA", "RX TIME", "SLOT ID", "RID", "VALID?");

  printf ("%10s:\n\n", "BEGIN SIMULATION");

  ns::simulator::configure (scfg);
  tdma::channel chan (cfg);
  tdma::ap a (cfg, chan);
  tdma::rm r1 (cfg, chan);
  ns::simulator::run();

  return 0;
}
