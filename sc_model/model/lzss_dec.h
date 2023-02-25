/**
 *  @file   lzss_dec.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/23  T. Ishitani     coding start
 */

#ifndef LZSS_DEC_H_
#define LZSS_DEC_H_

#include <vector>
#include <string>
#include "systemc.h"
#include "lzss_type.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int reference_size = 32, int coding_size = 9>
class LzssDec :
    public  sc_module
{
    LzssTypeDefine(reference_size, coding_size)

    typedef vector<data_t>  data_buffer_t;

public:
    sc_port<code_in_if_t>   code_in_if;
    sc_port<data_out_if_t>  data_out_if;

    typedef LzssDec<reference_size, coding_size>    SC_CURRENT_USER_MODULE;
    LzssDec(const sc_module_name& name);

    virtual void trace(sc_trace_file* tr) const;

protected:
    code_packet_t   input_code;
    data_packet_t   output_data;

    flag_t          flag;
    offset_t        offset;
    length_t        length;

    data_buffer_t   data_buffer;

    void main_thread();
    void update_buffer();
    void clear_buffer();
};

template <int reference_size, int coding_size>
LzssDec<reference_size, coding_size>::LzssDec(const sc_module_name& name) :
    sc_module   (name),
    data_buffer (reference_size)
{
    SC_THREAD(main_thread);
}

template <int reference_size, int coding_size>
void LzssDec<reference_size, coding_size>::trace(sc_trace_file* tr) const {
    sc_trace(tr, input_code , string(name()) + ".code"  );
    sc_trace(tr, output_data, string(name()) + ".data"  );
    sc_trace(tr, flag       , string(name()) + ".flag"  );
    sc_trace(tr, offset     , string(name()) + ".offset");
    sc_trace(tr, length     , string(name()) + ".length");
}

template <int reference_size, int coding_size>
void LzssDec<reference_size, coding_size>::main_thread() {
    while (true) {
        //  コード入力
        code_in_if->read(input_code);

        //  デコード
        flag    = input_code.value[constants::code_width - 1];
        if (flag) {
            offset  = input_code.value.range(constants::code_width   - 2, constants::length_width);
            length  = input_code.value.range(constants::length_width - 1, 0);

            for (int shift = (length + 2);shift > 0;shift--) {
                //  出力
                output_data.value   = data_buffer[offset];
                output_data.last    = (input_code.last && (shift == 1));
                data_out_if->write(output_data);

                //  バッファ更新
                update_buffer();
            }
        }
        else {
            offset  = 0;
            length  = 0;

            //  出力
            output_data.value   = input_code.value.range(constants::data_width - 1, 0);
            output_data.last    = input_code.last;
            data_out_if->write(output_data);

            //  バッファ更新
            update_buffer();
        }

        //  バッファクリア
        if (input_code.last) {
            clear_buffer();
        }
    }
}

template <int reference_size, int coding_size>
void LzssDec<reference_size, coding_size>::update_buffer() {
    data_buffer.erase(data_buffer.begin());
    data_buffer.push_back(output_data.value);
}

template <int reference_size, int coding_size>
void LzssDec<reference_size, coding_size>::clear_buffer() {
    data_buffer.clear();
    data_buffer.assign(reference_size, 0);
}

#endif /* LZSS_DEC_H_ */
