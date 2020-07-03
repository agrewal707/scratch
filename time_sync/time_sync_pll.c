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

This is PLL based time synchronization algorithm adapted from following PLL
example https://liquidsdr.org/blog/pll-simple-howto/
*/

/*
*
# Build
$ gcc -Werror time_sync_pll.c -o time_sync_pll -lm

# Run
$ ./time_sync_pll
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <complex.h>
#include <math.h>

int main()
{
  // parameters and simulation options
  uint32_t t1       =  0;      // TX time in the beacon from master
  uint32_t interval = 10000;   // beacon interval
  float alpha       = 0.5f;   // phase adjustment factor
  float cfo         = 0.2f;    // clock frequency offset
  unsigned int n    = 50;     // number of beacons

  // initialize states
  float beta        = 0.5*alpha*alpha; // frequency adjustment factor
  uint32_t t2       = 0;            // beacon receive time
  float cfo_est     = 0.0f;            // clock frequency offset estimate

  // print line legend to standard output
  printf("# %6s %10s %10s %10s %12s\n",
    "index", "t1", "t2", "error", "cfo_est");

  // run basic simulation
  int i;
  for (i=0; i<n; i++) {
    int time_error = t1 - t2;

    // print results to standard output for plotting
    printf("  %6u %10u %10d %10d %12.8f\n",
      i, t1, t2, time_error, cfo_est);

    t2 += alpha * time_error;
    cfo_est += beta * ((float)time_error / interval);

    // increment input and output time values
    t1 += interval * (1+cfo);
    t2 += interval * (1+cfo_est);
  }
  return 0;
}
