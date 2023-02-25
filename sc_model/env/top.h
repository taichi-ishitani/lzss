/**
 *  @file   top.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/21  T. Ishitani     coding start
 */

#ifndef TOP_H_
#define TOP_H_

#include "systemc.h"
#include "lzss_type.h"
#include "lzss_utility.h"
#include "env.h"
#include "lzss_enc.h"
#include "lzss_dec.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int reference_size = 32, int coding_size = 9>
class Top :
    public  sc_module
{
    typedef Env<reference_size, coding_size>        env_t;
    typedef LzssEnc<reference_size, coding_size>    enc_t;
    typedef LzssDec<reference_size, coding_size>    dec_t;

public:
    Top(const sc_module_name& name, double cycle_time, double time_out, sc_time_unit unit);
    virtual ~Top();

protected:
    Stimulus        stimulus;
    TimeConstraint  time_constraint;

    env_t*          env;
    enc_t*          enc;
    dec_t*          dec;

    sc_trace_file*  tr;

    virtual void start_of_simulation();
    virtual void end_of_simulation();
    void set_stimulus();
};

template <int reference_size, int coding_size>
Top<reference_size, coding_size>::Top(const sc_module_name& name, double cycle_time, double time_out, sc_time_unit unit) :
    time_constraint (cycle_time, time_out, unit),
    tr              (0)
{
    stimulus.data_in_dir    = "../sample";
    stimulus.code_out_dir   = "./encode";
    stimulus.data_out_dir   = "./decode";
    stimulus.dump_dir       = "./dump";
    set_stimulus();

    env = new env_t("env", stimulus, time_constraint);
    enc = new enc_t("enc");
    dec = new dec_t("dec");

    enc->data_in_if(env->enc_data_if);
    enc->code_out_if(env->enc_code_if);
    dec->code_in_if(env->dec_code_if);
    dec->data_out_if(env->dec_data_if);
};

template <int reference_size, int coding_size>
Top<reference_size, coding_size>::~Top() {
    delete env;
    delete enc;
    delete dec;
}

template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::start_of_simulation() {
#ifdef DUMP
    string  file;
    file    = stimulus.dump_dir + "/" + "dump";
    tr      = sc_create_vcd_trace_file(file.c_str());
    env->trace(tr);
    enc->trace(tr);
    dec->trace(tr);
#endif
}

template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::end_of_simulation() {
#ifdef DUMP
    sc_close_vcd_trace_file(tr);
#endif
}

#ifdef STIMULUS
#include STIMULUS
#else
template <int reference_size, int coding_size>
void Top<reference_size, coding_size>::set_stimulus() {
#ifdef FULL_STIMULUSES
    stimulus.add_file("test1.txt");
    stimulus.add_file("test2.txt");
    stimulus.add_file("test3.txt");
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
#else
    stimulus.add_file("test1.txt");
    stimulus.add_file("test2.txt");
#endif
}
#endif

#endif /* TOP_H_ */
