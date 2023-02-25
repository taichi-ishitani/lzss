/**
 *  @file   env.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/27  T. Ishitani     coding start
 */

#ifndef ENV_H_
#define ENV_H_

#include <vector>
#include <string>
#include <sstream>
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
    sc_out<bool>                rst_x;

    sc_export<data_in_if_t>     dut_enc_data_if;
    sc_export<code_out_if_t>    dut_enc_code_if;
    sc_export<data_in_if_t>     model_enc_data_if;
    sc_export<code_out_if_t>    model_enc_code_if;

    sc_export<code_in_if_t>     dut_dec_code_if;
    sc_export<data_out_if_t>    dut_dec_data_if;
    sc_export<code_in_if_t>     model_dec_code_if;
    sc_export<data_out_if_t>    model_dec_data_if;

    typedef Env<reference_size, coding_size>    SC_CURRENT_USER_MODULE;
    Env(const sc_module_name& name, Stimulus& stimulus, TimeConstraint& time_constraint, int fifo_size = 16);

protected:
    sc_fifo<data_packet_t>  dut_enc_data_fifo;
    sc_fifo<code_packet_t>  dut_enc_code_fifo;
    sc_fifo<data_packet_t>  model_enc_data_fifo;
    sc_fifo<code_packet_t>  model_enc_code_fifo;
    sc_fifo<code_packet_t>  dut_dec_code_fifo;
    sc_fifo<data_packet_t>  dut_dec_data_fifo;
    sc_fifo<code_packet_t>  model_dec_code_fifo;
    sc_fifo<data_packet_t>  model_dec_data_fifo;

    Stimulus&               stimulus;
    TimeConstraint&         time_constraint;

    sc_event                reset_done;
    sc_event                enc_data_thread_done;
    sc_event                code_thread_done;
    sc_event                dec_data_thread_done;

    vector<data_t>          data_stream;

    data_packet_t           input_data;
    code_packet_t           dut_enc_code;
    code_packet_t           model_enc_code;
    data_packet_t           dut_dec_data;
    data_packet_t           model_dec_data;

    void main_thread();
    void enc_data_thread();
    void code_thread();
    void dec_data_thread();
    void watch_dog_thread();

    template <typename type>
    void compare(type& model_value, type& dut_value, const char* id) const;
};

template <int reference_size, int coding_size>
Env<reference_size, coding_size>::Env(const sc_module_name& name, Stimulus& stimulus, TimeConstraint& time_constrant, int fifo_size) :
    sc_module           (name),
    stimulus            (stimulus),
    time_constraint     (time_constrant),
    dut_enc_data_fifo   (fifo_size),
    dut_enc_code_fifo   (fifo_size),
    dut_dec_code_fifo   (fifo_size),
    dut_dec_data_fifo   (fifo_size)
{
    SC_THREAD(main_thread);
    SC_THREAD(enc_data_thread);
    SC_THREAD(code_thread);
    SC_THREAD(dec_data_thread);
    SC_THREAD(watch_dog_thread);

    dut_enc_data_if(dut_enc_data_fifo);
    dut_enc_code_if(dut_enc_code_fifo);
    model_enc_data_if(model_enc_data_fifo);
    model_enc_code_if(model_enc_code_fifo);
    dut_dec_code_if(dut_dec_code_fifo);
    dut_dec_data_if(dut_dec_data_fifo);
    model_dec_code_if(model_dec_code_fifo);
    model_dec_data_if(model_dec_data_fifo);
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::main_thread() {
    stringstream    message;

    message << sc_time_stamp() << "\nSimulation started";
    SC_REPORT_INFO(name(), message.str().c_str());

    //  リセット
    rst_x->write(1);
    wait(time_constraint.cycle_time * 6);
    rst_x->write(0);
    wait(time_constraint.cycle_time * 6);
    rst_x->write(1);
    reset_done.notify(time_constraint.cycle_time);
    message.str("");
    message << sc_time_stamp() << "\nReset done";
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

    wait(reset_done);
    wait(time_constraint.time_out);
    message << sc_time_stamp() << "\nTime Out!!";
    SC_REPORT_FATAL(name(), message.str().c_str());
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::enc_data_thread() {
    Stimulus::iterator                  stimulus_pos    = stimulus.begin();
    typename vector<data_t>::iterator   data_pos;
    string                              file;

    wait(reset_done);
    while (stimulus_pos != stimulus.end()) {
        //  ファイル入力
        file    = stimulus.data_in_dir + "/" + (*stimulus_pos);
        read_file<data_t, constants::data_width>(file, data_stream, name());
        ++stimulus_pos;

        data_pos    = data_stream.begin();
        while (data_pos != data_stream.end()) {
            input_data.value    = *data_pos;
            ++data_pos;
            input_data.last     = (data_pos == data_stream.end()) ? true : false;

            model_enc_data_fifo.write(input_data);
            dut_enc_data_fifo.write(input_data);
        }
    }
    enc_data_thread_done.notify();
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::code_thread() {
    Stimulus::iterator  stimulus_pos    = stimulus.begin();
    string              id              = string(name()) + "(enc)";

    wait(reset_done);
    while (stimulus_pos != stimulus.end()) {
        dut_enc_code_fifo.read(dut_enc_code);
        model_enc_code_fifo.read(model_enc_code);

        compare(model_enc_code, dut_enc_code, id.c_str());

        model_dec_code_fifo.write(model_enc_code);
        dut_dec_code_fifo.write(model_enc_code);

        if (model_enc_code.last) {
            ++stimulus_pos;
        }
    }
    code_thread_done.notify();
}

template <int reference_size, int coding_size>
void Env<reference_size, coding_size>::dec_data_thread() {
    Stimulus::iterator  stimulus_pos    = stimulus.begin();
    string              id              = string(name()) + "(dec)";

    wait(reset_done);
    while (stimulus_pos != stimulus.end()) {
        dut_dec_data_fifo.read(dut_dec_data);
        model_dec_data_fifo.read(model_dec_data);

        compare(model_dec_data, dut_dec_data, id.c_str());

        if (model_dec_data.last) {
            ++stimulus_pos;
        }
    }
    dec_data_thread_done.notify();
}

template <int reference_size, int coding_size>
template <typename type>
void Env<reference_size, coding_size>::compare(type& model_value, type& dut_value, const char* id) const {
    stringstream    message;

    if (model_value != dut_value) {
        message << sc_time_stamp()
                << "\nModel :: " << model_value
                << "\nDUT   :: " << dut_value;
        SC_REPORT_ERROR(id, message.str().c_str());
    }
    else {
#ifdef VERBOSE
        message << sc_time_stamp()
                << "\n"
                << dut_value;
        SC_REPORT_INFO(id, message.str().c_str());
#endif
    }
}

#endif /* ENV_H_ */
