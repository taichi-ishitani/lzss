/**
 *  @file   main.cpp
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/28  T. Ishitani     coding start
 */

#ifndef CYCLE_TIME
#define CYCLE_TIME  10
#endif

#ifndef TIME_OUT
#define TIME_OUT    40000000
#endif

#ifndef TIME_UNIT
#define TIME_UNIT   SC_NS
#endif

#ifndef FIFO_SIZE
#define FIFO_SIZE   1
#endif

#ifndef REFERENCE_SIZE
#define REFERENCE_SIZE  64
#endif

#ifndef CODING_SIZE
#define CODING_SIZE 5
#endif

#ifndef MAX_ERROR_NUM
#define MAX_ERROR_NUM       10
#endif

#ifndef FULL_STIMULUSES
#define DUMP_RTL
#endif

#include <iostream>
#include "systemc.h"
#include "top.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

typedef Top<REFERENCE_SIZE, CODING_SIZE>    top_t;

int sc_main(int argc, char* argv[]) {
    top_t*  top;

    sc_report_handler::stop_after(SC_ERROR, MAX_ERROR_NUM);
    sc_report_handler::set_actions(SC_ERROR, SC_DISPLAY);
    sc_report_handler::set_actions(SC_FATAL, SC_DISPLAY | SC_STOP);

    top = new top_t("top", CYCLE_TIME, TIME_OUT, TIME_UNIT, FIFO_SIZE);
    sc_start();
    delete top;

    cout << "------------------------------------------------"   << endl;
    cout << "Message Information"                                << endl;
    cout << "Info  : " << sc_report_handler::get_count(SC_INFO)  << endl;
    cout << "Error : " << sc_report_handler::get_count(SC_ERROR) << endl;
    cout << "Fatal : " << sc_report_handler::get_count(SC_FATAL) << endl;

    return 0;
}
