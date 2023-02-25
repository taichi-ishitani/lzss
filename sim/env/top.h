/**
 *  @file   top.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/27  T. Ishitani     coding start
 */

#ifndef TOP_H_
#define TOP_H_

#include "systemc.h"
#include "lzss_type.h"
#include "lzss_utility.h"
#include "env.h"
#include "lzss_enc.h"
#include "lzss_dec.h"
#include "lzss_enc_dut.h"
#include "lzss_dec_dut.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int reference_size = 32, int coding_size = 9>
class Top :
    public  sc_module
{
    typedef Env<reference_size, coding_size>        env_t;
    typedef LzssEnc<reference_size, coding_size>    model_enc_t;
    typedef LzssDec<reference_size, coding_size>    model_dec_t;
    typedef LzssEncDut<reference_size, coding_size> dut_enc_t;
    typedef LzssDecDut<reference_size, coding_size> dut_dec_t;

public:
    Top(const sc_module_name& name, double cycle_time, double time_out, sc_time_unit unit, int fifo_size);
    virtual ~Top();

protected:
    Stimulus        stimulus;
    TimeConstraint  time_constraint;

    sc_clock        clk;
    sc_signal<bool> rst_x;

    env_t*          env;
    model_enc_t*    model_enc;
    model_dec_t*    model_dec;
    dut_enc_t*      dut_enc;
    dut_dec_t*      dut_dec;

    VerilatedVcdSc* dump_rtl;
    sc_trace_file*  dump_env;

    virtual void start_of_simulation();
    virtual void end_of_simulation();
    void set_stimulus();
};

template <int reference_size, int coding_size>
Top<reference_size, coding_size>::Top(const sc_module_name& name, double cycle_time, double time_out, sc_time_unit unit, int fifo_size) :
    sc_module       (name),
    time_constraint (cycle_time, time_out, unit),
    clk             ("clk", cycle_time, unit)
{
    stimulus.data_in_dir    = "../sample";
    stimulus.code_out_dir   = "./encode";
    stimulus.data_out_dir   = "./decode";
    stimulus.dump_dir       = "./dump";
    set_stimulus();

    env         = new       env_t("env", stimulus, time_constraint, fifo_size);
    dut_enc     = new   dut_enc_t("dut_enc");
    model_enc   = new model_enc_t("model_enc");
    dut_dec     = new   dut_dec_t("dut_dec");
    model_dec   = new model_dec_t("model_dec");

    env->rst_x(rst_x);
    dut_enc->clk(clk);
    dut_enc->rst_x(rst_x);
    dut_enc->data_in_if(env->dut_enc_data_if);
    dut_enc->code_out_if(env->dut_enc_code_if);
    model_enc->data_in_if(env->model_enc_data_if);
    model_enc->code_out_if(env->model_enc_code_if);
    dut_dec->clk(clk);
    dut_dec->rst_x(rst_x);
    dut_dec->code_in_if(env->dut_dec_code_if);
    dut_dec->data_out_if(env->dut_dec_data_if);
    model_dec->code_in_if(env->model_dec_code_if);
    model_dec->data_out_if(env->model_dec_data_if);
}

template <int reference_size, int coding_size>
Top<reference_size, coding_size>::~Top() {
    delete  env;
    delete  dut_enc;
    delete  model_enc;
    delete  dut_dec;
    delete  model_dec;
#ifdef DUMP_RTL
    delete  dump_rtl;
#endif
}

template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::start_of_simulation() {
    string  file;
#ifdef DUMP_RTL
    Verilated::traceEverOn(true);
    file        = stimulus.dump_dir + "/" + "dump_rtl.vcd";
    dump_rtl    = new VerilatedVcdSc;
    dut_enc->trace(dump_rtl);
    dut_dec->trace(dump_rtl);
    dump_rtl->open(file.c_str());
#endif
#ifdef DUMP_ENV
    file        = stimulus.dump_dir + "/" + "dump_env";
    dump_env    = sc_create_vcd_trace_file(file.c_str());
    model_enc->trace(dump_env);
    model_dec->trace(dump_env);
    dut_dec->trace(dump_env);
#endif
}

template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::end_of_simulation() {
#ifdef DUMP_RTL
    dump_rtl->close();
#endif
#ifdef DUMP_ENV
    sc_close_vcd_trace_file(tr);
#endif
}

#ifdef STIMULUS
#include STIMULUS
#else
template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::set_stimulus() {
#if defined SIMPLE_STIMULUSES
    stimulus.add_file("test1.txt");
    stimulus.add_file("test2.txt");
    stimulus.add_file("test3.txt");
#elif defined FULL_STIMULUSES
    stimulus.add_file("alice29.txt");
    stimulus.add_file("cp.html");
    stimulus.add_file("grammar.lsp");
    stimulus.add_file("lcet10.txt");
    stimulus.add_file("ptt5");
    stimulus.add_file("asyoulik.txt");
    stimulus.add_file("kennedy.xls");
    stimulus.add_file("plrabn12.txt");
    stimulus.add_file("sum");
    stimulus.add_file("xargs.1");
    stimulus.add_file("test1.txt");
    stimulus.add_file("test2.txt");
    stimulus.add_file("test3.txt");
#else
    stimulus.add_file("alice29.txt");
    stimulus.add_file("test3.txt");
    stimulus.add_file("test2.txt");
    stimulus.add_file("test1.txt");
#endif
}
#endif

#endif /* TOP_H_ */
