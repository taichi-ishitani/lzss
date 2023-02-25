/**
 *  @file   lzss_type.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/15  T. Ishitani     coding start
 */

#ifndef LZSS_TYPE_H_
#define LZSS_TYPE_H_

#include "systemc.h"
#include "lzss_utility.h"
#include "lzss_packet.h"

using namespace sc_core;
using namespace sc_dt;

template <int reference_size, int coding_size>
struct LzssConstants {
    static const int    window_size     = reference_size + coding_size;
    static const int    offset_width    = Log2<reference_size>::value;
    static const int    length_width    = Log2<coding_size - 1>::value;
    static const int    data_width      = 8;
    static const int    code_width      = offset_width + length_width + 1;
};

#define LzssTypeDefine(r, c) \
    typedef LzssConstants<r, c>                     constants;\
    typedef sc_uint<constants::data_width>          data_t;\
    typedef sc_uint<constants::code_width>          code_t;\
    typedef LzssPacket<constants::data_width, Data> data_packet_t;\
    typedef LzssPacket<constants::code_width, Code> code_packet_t;\
    typedef sc_fifo_in_if<data_packet_t>            data_in_if_t;\
    typedef sc_fifo_in_if<code_packet_t>            code_in_if_t;\
    typedef sc_fifo_out_if<data_packet_t>           data_out_if_t;\
    typedef sc_fifo_out_if<code_packet_t>           code_out_if_t;\
    typedef sc_uint<1>                              flag_t;\
    typedef sc_uint<constants::offset_width>        offset_t;\
    typedef sc_uint<constants::length_width>        length_t;\


#endif /* LZSS_TYPE_H_ */
