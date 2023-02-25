/**
 *  @file   lzss_enc.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/19  T. Ishitani     coding start
 */

#ifndef LZSS_ENC_H_
#define LZSS_ENC_H_

#include <vector>
#include <string>
#include "systemc.h"
#include "lzss_type.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int reference_size = 32, int coding_size = 9>
class LzssEnc :
    public  sc_module
{
    LzssTypeDefine(reference_size, coding_size)

    struct BufferData {
        BufferData() :
            data    (0),
            valid   (false)
        {}

        BufferData(const data_t& data, bool valid) :
            data    (data),
            valid   (valid)
        {}

        bool operator ==(const BufferData& other) const {
            return (valid && other.valid && (data == other.data)) ? true : false;
        }

        data_t  data;
        bool    valid;
    };

    typedef BufferData              buffer_data_t;
    typedef vector<buffer_data_t>   data_buffer_t;
    typedef vector<int>             length_buffer_t;

public:
    sc_port<data_in_if_t>   data_in_if;
    sc_port<code_out_if_t>  code_out_if;

    typedef LzssEnc<reference_size, coding_size>    SC_CURRENT_USER_MODULE;
    LzssEnc(const sc_module_name& name);

    virtual void trace(sc_trace_file* tr) const;

protected:
    data_packet_t   input_data;
    code_packet_t   output_code;

    flag_t          flag;
    length_t        length;
    offset_t        offset;

    data_buffer_t   data_buffer;
    length_buffer_t length_buffer;

    void main_thread();
    void update_buffer(bool with_new_data, bool update_length);
    void clear_buffer();
};

template <int reference_size, int coding_size>
LzssEnc<reference_size, coding_size>::LzssEnc(const sc_module_name& name) :
    sc_module       (name),
    data_buffer     (constants::window_size),
    length_buffer   (reference_size)
{
    SC_THREAD(main_thread);
}

template <int reference_size, int coding_size>
void LzssEnc<reference_size, coding_size>::trace(sc_trace_file* tr) const {
    sc_trace(tr, input_data , string(name()) + ".data"  );
    sc_trace(tr, output_code, string(name()) + ".code"  );
    sc_trace(tr, flag       , string(name()) + ".flag"  );
    sc_trace(tr, offset     , string(name()) + ".offset");
    sc_trace(tr, length     , string(name()) + ".length");
}

template <int reference_size, int coding_size>
void LzssEnc<reference_size, coding_size>::main_thread() {
    int     shift;
    bool    last_done;
    bool    update_length;
    int     max_offset;
    int     max_length;
    code_t  code;
    bool    last_flag;

    shift       = coding_size + 1;
    last_done   = false;
    while (true) {
        while (shift > 0) {
            shift   -= 1;

            //  データ入力
            if (!last_done) {
                data_in_if->read(input_data);
            }

            //  バッファ更新
            update_length   = (shift == 0) ? true : false;
            update_buffer(!last_done, update_length);
            last_done       = input_data.last;
        }

        //  最長一致系列の検索
        max_offset  = 0;
        max_length  = 1;
        for (int i = 0;i < reference_size;i++) {
            if (max_length <= length_buffer[i]) {
                max_length  = length_buffer[i];
                max_offset  = i;
            }
        }

        //  エンコード
        flag    = (max_length > 1);;
        if (flag) {
            offset  = max_offset;
            length  = max_length - 2;
            code    = (flag, offset, length);
            shift   = max_length;
        }
        else {
            offset  = 0;
            length  = 0;
            code    = (flag, data_buffer[reference_size - 1].data);
            shift   = 1;
        }
        if (last_done && (!data_buffer[reference_size + max_length - 1].valid)) {
            last_flag   = true;
        }
        else {
            last_flag   = false;
        }

        //  出力
        output_code.value   = code;
        output_code.last    = last_flag;
        code_out_if->write(output_code);

        //  バッファクリア
        if (last_flag) {
            clear_buffer();
            shift       = coding_size + 1;
            last_done   = false;
        }
    }
}

template <int reference_size, int coding_size>
void LzssEnc<reference_size, coding_size>::update_buffer(bool with_new_data, bool update_length) {
    buffer_data_t   new_data;

    //  一致長バッファのアップデート
    if (update_length) {
        length_buffer.clear();
        for (int offset_pos = 0;offset_pos < reference_size;offset_pos++) {
            int matching_length = 0;
            for (int i = 0;i < coding_size;i++) {
                buffer_data_t&  d1  = data_buffer[offset_pos     + i];
                buffer_data_t&  d2  = data_buffer[reference_size + i];
                if (d1 == d2) {
                    matching_length += 1;
                }
                else {
                    break;
                }
            }
            length_buffer.push_back(matching_length);
        }
    }

    //  データバッファのアップデート
    if (with_new_data) {
        new_data.data   = input_data.value;
        new_data.valid  = true;
    }
    data_buffer.erase(data_buffer.begin());
    data_buffer.push_back(new_data);
}

template <int reference_size, int coding_size>
void LzssEnc<reference_size, coding_size>::clear_buffer() {
    data_buffer.clear();
    data_buffer.assign(constants::window_size, buffer_data_t());

    length_buffer.clear();
    length_buffer.assign(reference_size, 0);
}

#endif /* LZSS_ENC_H_ */
