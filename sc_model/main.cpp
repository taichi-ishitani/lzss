/**
 *  @file   main.cpp
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/18  T. Ishitani     coding start
 */

#include "systemc.h"
#include "top.h"

using namespace sc_core;
using namespace sc_dt;

#ifndef CYCLE_TIME
#define CYCLE_TIME  10
#endif

#ifndef TIME_OUT
#define TIME_OUT    40000000
#endif

#ifndef TIME_UNIT
#define TIME_UNIT   SC_NS
#endif

#ifndef REFERENCE_SIZE
#define REFERENCE_SIZE  64
#endif

#ifndef CODING_SIZE
#define CODING_SIZE 5
#endif

int sc_main(int argc, char* argv[]) {
    Top<REFERENCE_SIZE, CODING_SIZE>    top("top", CYCLE_TIME, TIME_OUT, TIME_UNIT);
    sc_start();

    return 0;
}
