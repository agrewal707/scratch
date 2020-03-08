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

//
// A simple framework for discrete event simulation.
//

#ifndef NS_SIMULATOR_H
#define NS_SIMULATOR_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <list>
#include <unordered_map>
#include <functional>

namespace ns {

//
// disrete event simulator time
//
struct time
{
  //
  // represents a point in time
  //
  struct point
  {
    point ():
      m_val (0)
    {}

    point (uint32_t val):
      m_val (val)
    {}
    uint32_t m_val;
  };

  //
  // represents difference between two time points
  //
  struct duration
  {
    duration (int32_t val):
      m_val (val)
    {}

    operator float ()
    {
      return m_val;
    }

    int32_t m_val;
  };

  static point now ()
  {
    return m_current;
  }

  static void set (point val)
  {
    m_current = val;
  }

  // this represent simulator clock tick
  static point m_current;
};
time::point time::m_current;

bool operator < (const time::point &lhs, const time::point& rhs)
{
  return lhs.m_val < rhs.m_val;
}
time::point operator + (const time::point &lhs, const time::duration& rhs)
{
  return time::point (lhs.m_val + rhs.m_val);
}

time::duration operator - (const time::point &lhs, const time::point& rhs)
{
  return time::duration (lhs.m_val - rhs.m_val);
}

//
// discrete-event simulator
//
class simulator
{
public:
  using Handler = std::function<void(void)>;

  struct config
  {
    config ():
      num_runs (100)
    {}

    int num_runs;
  };

  simulator()
  {
    time::set (0);
  }

  static void configure (const config &cfg)
  {
    auto &sim = get ();
    sim.do_configure (cfg);
  }

  static void schedule (const time::point &tp, const Handler &handler)
  {
    auto &sim = get ();
    sim.do_schedule (tp, handler);
  }

  static void run ()
  {
    auto &sim = get ();
    sim.do_run ();
  }

  // factory
  static simulator& get ()
  {
    static simulator instance;
    return instance;
  }

private:
  struct event
  {
    event (const time::point &tp, const Handler &handler):
      m_tp (tp),
      m_handler (handler)
    {}

    time::point m_tp;
    Handler m_handler;
  };

  void do_configure (const config &cfg)
  {
    m_cfg = cfg;
  }

  void do_schedule (const time::point &tp, const Handler &handler)
  {
    event ev (tp, handler);

    // insert this event in the right place
    auto it = m_ev_list.begin ();
    for (; it != m_ev_list.end (); ++it)
    {
      if (ev.m_tp < it->m_tp)
      {
        m_ev_list.insert (it, ev);
        break;
      }
    }
    if (it == m_ev_list.end ())
    {
      // inert event at the end
      m_ev_list.insert (it, ev);
    }
  }

  void do_run ()
  {
    static int counter = 0;
    while (!m_ev_list.empty () && counter++ < m_cfg.num_runs)
    {
      // pick first event in the list
      auto it = m_ev_list.begin();

      // current time is tp of this event
      time::set (it->m_tp);

      // execute the associated handler
      it->m_handler ();

      // erase from the event queue
      it = m_ev_list.erase (it);
    }
  }

  config m_cfg;
  std::list<event> m_ev_list;
};

} // namespace ns

#endif // NS_SIMULATOR_H
