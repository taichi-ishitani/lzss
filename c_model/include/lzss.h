/**
 *  @file   lzss.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/04  T. Ishitani     coding start
 */

#ifndef LZSS_H_
#define LZSS_H_

#include <vector>
#include <stdint.h>
#include "utility.h"

using namespace std;

typedef uint8_t     data_t;
typedef uint16_t    code_t;

typedef vector<data_t>  data_stream_t;
typedef vector<code_t>  code_stream_t;

template <int reference_size = 16, int coding_size = 17>
class Lzss {
public:
    static const int    window_size = reference_size + coding_size;
    static const int    code_width  = Log2<reference_size >::value
                                    + Log2<coding_size - 1>::value
                                    + 1;

    code_stream_t*  encode(data_stream_t* input_stream);
    data_stream_t*  decode(code_stream_t* input_stream);

    void clear();

protected:
    data_stream_t   buffer;

    int compare(int offset, int ref_size);

};

template <int reference_size, int coding_size>
code_stream_t* Lzss<reference_size, coding_size>::encode(data_stream_t* input_stream) {
    code_stream_t*          output_stream   = new code_stream_t;
    data_stream_t::iterator pos             = input_stream->begin();
    bool                    input_done      = false;
    int                     total_size      = 0;
    int                     ref_size        = 0;
    int                     offset;
    int                     length;
    int                     max_offset;
    int                     max_length;
    code_t                  code;

    for (int i = 0;i < coding_size;i++) {
        buffer.push_back(*pos);
        ++pos;
    }
    while (1) {
        //  最長一致系列の検索
        if (!input_done) {
            ref_size    = buffer.size() - coding_size;
        }
        max_offset  = 0;
        max_length  = 0;
        length      = 0;
        for (offset = 0;offset < ref_size;offset++) {
            length  = compare(offset, ref_size);
            if (length >= max_length) {
                max_length  = length;
                max_offset  = reference_size - ref_size + offset;
            }
        }

        if (max_length > 1) {
            code     = (1 << (code_width - 1));
            code    |= (max_offset << Log2<coding_size - 1>::value);
            code    |= max_length - 2;
        }
        else {
            max_length  = 1;
            code        = buffer.at(ref_size);
        }
        output_stream->push_back(code);

        //  次のループへの処理
        total_size  += max_length;
        for (int i = 0;i < max_length;i++) {
            if (pos != input_stream->end()) {
                buffer.push_back(*pos);
                ++pos;
            }
            if (pos == input_stream->end()) {
                input_done  = true;
            }
            if ((ref_size < reference_size) && (!input_done)) {
                ref_size    += 1;
            }
            if ((buffer.size() > window_size) || input_done) {
                buffer.erase(buffer.begin());
            }
        }
        if (total_size == input_stream->size()) {
            break;
        }
    }

    return output_stream;
}

template <int reference_size, int coding_size>
data_stream_t* Lzss<reference_size, coding_size>::decode(code_stream_t* input_stream) {
    data_stream_t*          output_stream   = new data_stream_t;
    code_stream_t::iterator pos             = input_stream->begin();
    const code_t            mask1           = 1 << (code_width - 1);
    const code_t            mask2           = mask1 - 1;
    int                     total_size      = 0;
    bool                    flag;
    code_t                  code;
    data_t                  data;
    int                     offset;
    int                     length;

    buffer.assign(reference_size, 0);
    while (pos != input_stream->end()) {
        //  入力コードの取りだし
        flag    = ((*pos) & mask1) ? true : false;
        code    =  (*pos) & mask2;
        ++pos;

        if (flag) {
            offset  = ((code >> Log2<coding_size - 1>::value)) & ((1 << Log2<reference_size>::value) - 1);
            length  = (code & ((1 << Log2<coding_size - 1>::value) - 1)) + 2;
            for (int i = 0;i < length;i++) {
                data    = (data_t)buffer.at(i + offset);
                output_stream->push_back(data);
                buffer.push_back(data);
            }
        }
        else {
            length  = 1;
            data    = (data_t)code;
            output_stream->push_back(data);
            buffer.push_back(data);
        }

        while (buffer.size() > reference_size) {
            buffer.erase(buffer.begin());
        }
        total_size  += length;
    }

    return output_stream;
}

template <int reference_size, int coding_size>
void Lzss<reference_size, coding_size>::clear() {
    buffer.clear();
}

template <int reference_size, int coding_size>
int Lzss<reference_size, coding_size>::compare(int offset, int ref_size) {
    int     length  = 0;
    int     size;
    data_t  d1, d2;

    size    = buffer.size() - ref_size;
    for (int i = 0;i < size;i++) {
        d1  = buffer.at(offset   + i);
        d2  = buffer.at(ref_size + i);
        if (d1 == d2) {
            length  += 1;
        }
        else {
            break;
        }
    }

    return length;
}

#endif /* LZSS_H_ */
