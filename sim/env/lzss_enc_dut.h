/**
 *  @file   lzss_enc_dut.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/07/07  T. Ishitani     coding start
 */

#ifndef LZSS_ENC_DUT_H_
#define LZSS_ENC_DUT_H_

#include <string>
#include <stdint.h>
#include "systemc.h"
#include "lzss_type.h"
#include "Vdut_enc.h"
#include "verilated_vcd_sc.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int reference_size = 32, int coding_size = 9>
class LzssEncDut :
    public  sc_module
{
    LzssTypeDefine(reference_size, coding_size)

public:
    sc_in_clk               clk;
    sc_in<bool>             rst_x;
    sc_port<data_in_if_t>   data_in_if;
    sc_port<code_out_if_t>  code_out_if;

    typedef LzssEncDut<reference_size, coding_size> SC_CURRENT_USER_MODULE;
    LzssEncDut(const sc_module_name& name);

    virtual void trace(sc_trace_file* tr) const;
    virtual void trace(VerilatedVcdSc* tr);

protected:
    data_packet_t   data;
    code_packet_t   code;

    //  DUT
    Vdut_enc        dut;

    //  入出力信号
    sc_signal<bool>     i_valid;
    sc_signal<bool>     ow_ready;
    sc_signal<uint32_t> i_data;
    sc_signal<bool>     i_last;
    sc_signal<bool>     o_valid;
    sc_signal<bool>     i_ready;
    sc_signal<uint32_t> o_code;
    sc_signal<bool>     o_last;

    void input_thread();
    void output_thread();
};

template <int reference_size, int coding_size>
LzssEncDut<reference_size, coding_size>::LzssEncDut(const sc_module_name& name) :
    sc_module   (name),
    dut         ("dut")
{
    SC_CTHREAD(input_thread, clk.pos());
    reset_signal_is(rst_x, false);
    SC_CTHREAD(output_thread, clk.pos());
    reset_signal_is(rst_x, false);

    dut.clk(clk);
    dut.rst_x(rst_x);
    dut.i_valid(i_valid);
    dut.ow_ready(ow_ready);
    dut.i_data(i_data);
    dut.i_last(i_last);
    dut.o_valid(o_valid);
    dut.i_ready(i_ready);
    dut.o_code(o_code);
    dut.o_last(o_last);
}

template <int reference_size, int coding_size>
void LzssEncDut<reference_size, coding_size>::trace(sc_trace_file* tr) const {
    sc_trace(tr, data, string(name()) + ".data");
    sc_trace(tr, code, string(name()) + ".code");
}

template <int reference_size, int coding_size>
void LzssEncDut<reference_size, coding_size>::trace(VerilatedVcdSc* tr) {
    dut.trace(tr, 1);
}

template <int reference_size, int coding_size>
void LzssEncDut<reference_size, coding_size>::input_thread() {
    //  リセット
    i_valid.write(0);
    i_data.write(0);
    i_last.write(0);
    wait();

    while (true) {
        //  入力待ち
        while (data_in_if->num_available() == 0) {
            wait();
        }
        data_in_if->nb_read(data);

        //  入力セット
        i_valid.write(1);
        i_data.write((uint32_t)data.value);
        i_last.write(data.last);

        //  レディ待ち
        do {
            wait();
        } while (ow_ready.read() == 0);

        //  入力クリア
        i_valid.write(0);
        i_data.write(0);
        i_last.write(0);
    }
}

template <int reference_size, int coding_size>
void LzssEncDut<reference_size, coding_size>::output_thread() {
    //  リセット
    i_ready.write(0);
    wait();

    while (true) {
        //  出力待ち
        i_ready.write(1);
        do {
            wait();
        } while (o_valid.read() == 0);
        i_ready.write(0);

        //  出力取りだし
        code.value  = o_code.read();
        code.last   = o_last.read();
        while (code_out_if->num_free() == 0) {
            wait();
        }
        code_out_if->nb_write(code);
    }
}

#endif /* LZSS_ENC_DUT_H_ */
