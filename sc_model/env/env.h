/**
 *  @file   env.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/21  T. Ishitani     coding start
 */

#ifndef ENV_H_
#define ENV_H_

#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include "systemc.h"
#include "lzss_type.h"
#include "lzss_utility.h"

using namespace std;
using namespace sc_core;

template <int reference_size = 32, int coding_size = 9>
class Env :
    public  sc_module
{
    LzssTypeDefine(reference_size, coding_size)

public:
    sc_export<data_in_if_t>     enc_data_if;
    sc_export<code_out_if_t>    enc_code_if;
    sc_export<code_in_if_t>     dec_code_if;
    sc_export<data_out_if_t>    dec_data_if;

    typedef Env<reference_size, coding_size>    SC_CURRENT_USER_MODULE;
    Env(const sc_module_name& name, Stimulus& stimulus, TimeConstraint& time_constrant, int fifo_size = 16);

    virtual void trace(sc_trace_file* tr) const;

protected:
    sc_fifo<data_packet_t>  enc_data_fifo;
    sc_fifo<code_packet_t>  enc_code_fifo;
    sc_fifo<code_packet_t>  dec_code_fifo;
    sc_fifo<data_packet_t>  dec_data_fifo;

    Stimulus&               stimulus;
    TimeConstraint&         time_constraint;

    sc_event                enc_data_thread_done;
    sc_event                code_thread_done;
    sc_event                dec_data_thread_done;

    vector<data_t>          enc_data_stream;
    vector<code_t>          code_stream;
    vector<data_t>          dec_data_stream;

    data_packet_t           enc_data;
    code_packet_t           code;
    data_packet_t           dec_data;

    void main_thread();
    void enc_data_thread();
    void code_thread();
    void dec_data_thread();
    void watch_dog_thread();
};

template <int reference_size, int coding_size>
Env<reference_size, coding_size>::Env(const sc_module_name& name, Stimulus& stimulus, TimeConstraint& time_constrant, int fifo_size) :
    sc_module       (name),
    stimulus        (stimulus),
    time_constraint (time_constrant),
    enc_data_fifo   (fifo_size),
    enc_code_fifo   (1),
    dec_code_fifo   (fifo_size),
    dec_data_fifo   (1)
{
    SC_THREAD(main_thread);
    SC_THREAD(enc_data_thread);
    SC_THREAD(code_thread);
    SC_THREAD(dec_data_thread);
    SC_THREAD(watch_dog_thread);

    enc_data_if(enc_data_fifo);
    enc_code_if(enc_code_fifo);
    dec_code_if(dec_code_fifo);
    dec_data_if(dec_data_fifo);
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::trace(sc_trace_file* tr) const {
    sc_trace(tr, enc_data, string(name()) + ".enc_data");
    sc_trace(tr, code    , string(name()) + ".code"    );
    sc_trace(tr, dec_data, string(name()) + ".dec_data");
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::main_thread() {
    stringstream    message;

    message << sc_time_stamp() << "\nSimulation started";
    SC_REPORT_INFO(name(), message.str().c_str());

    wait(enc_data_thread_done & code_thread_done & dec_data_thread_done);
    for (int i = 0;i < 10;i++) {
        wait(time_constraint.cycle_time);
    }

    message.str("");
    message << sc_time_stamp() << "\nSimulation finished";
    SC_REPORT_INFO(name(), message.str().c_str());
    sc_stop();
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::watch_dog_thread() {
    stringstream    message;

    wait(time_constraint.time_out);
    message << sc_time_stamp() << "\nTime Out!!";
    SC_REPORT_FATAL(name(), message.str().c_str());
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::enc_data_thread() {
    Stimulus::iterator                  stimulus_pos    = stimulus.begin();
    typename vector<data_t>::iterator   data_pos;
    stringstream                        message;
    string                              file;

    while (stimulus_pos != stimulus.end()) {
        //  ファイル入力
        file    = stimulus.data_in_dir + "/" + (*stimulus_pos);
        read_file<data_t, constants::data_width>(file, enc_data_stream, name());
        ++stimulus_pos;

        data_pos    = enc_data_stream.begin();
        while (data_pos != enc_data_stream.end()) {
            enc_data.value  = *data_pos;
            ++data_pos;
            enc_data.last   = (data_pos == enc_data_stream.end()) ? true : false;

#ifdef VERBOSE
            message.str("");
            message << sc_time_stamp() << "\n" << enc_data;
            SC_REPORT_INFO(name(), message.str().c_str());
#endif
            enc_data_fifo.write(enc_data);
            wait(time_constraint.cycle_time);
        }
    }
    enc_data_thread_done.notify();
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::code_thread() {
    Stimulus::iterator  stimulus_pos    = stimulus.begin();
    stringstream        message;
    string              file;

    while (stimulus_pos != stimulus.end()) {
        enc_code_fifo.read(code);
        dec_code_fifo.write(code);
#ifdef VERBOSE
        message.str("");
        message << sc_time_stamp() << "\n" << code;
        SC_REPORT_INFO(name(), message.str().c_str());
#endif

        code_stream.push_back(code.value);
        if (code.last) {
            //  ファイル出力
            file    = stimulus.code_out_dir + "/" + (*stimulus_pos) + ".bin";
            write_file<code_t, constants::code_width>(file, code_stream, name());

            ++stimulus_pos;
        }
    }

    code_thread_done.notify();
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::dec_data_thread() {
    Stimulus::iterator  stimulus_pos    = stimulus.begin();
    stringstream        message;
    string              file;
    string              orignal_file;
    string              diff_command;
    data_packet_t       data;

    while (stimulus_pos != stimulus.end()) {
        dec_data_fifo.read(dec_data);

#ifdef VERBOSE
        message.str("");
        message << sc_time_stamp() << "\n" << dec_data;
        SC_REPORT_INFO(name(), message.str().c_str());
#endif

        dec_data_stream.push_back(dec_data.value);
        if (dec_data.last) {
            //  ファイル出力
            file    = stimulus.data_out_dir + "/" + (*stimulus_pos);
            write_file<data_t, constants::data_width>(file, dec_data_stream, name());

            //  一致比較
            orignal_file    = stimulus.data_in_dir  + "/" + (*stimulus_pos);
            diff_command    = string("diff ") + orignal_file + " " + file;
            system(diff_command.c_str());

            ++stimulus_pos;
        }
    }

    dec_data_thread_done.notify();
}

#endif /* ENV_H_ */
