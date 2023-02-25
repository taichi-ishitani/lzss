/**
 *  @file   lzss_packet.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/14  T. Ishitani     coding start
 */

#ifndef LZSS_PACKET_H_
#define LZSS_PACKET_H_

#include <iostream>
#include <iomanip>
#include <string>
#include "systemc.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

enum LzssPacketType {
    Data,
    Code
};

template <int w, LzssPacketType type>
struct LzssPacket {
    LzssPacket() :
        value   (0      ),
        last    (false  )
    {}

    LzssPacket(const LzssPacket<w, type>& other) :
        value   (other.value    ),
        last    (other.last     )
    {}

    LzssPacket<w, type>& operator =(const LzssPacket<w, type>& rhs) {
        value   = rhs.value;
        last    = rhs.last;
        return *this;
    }

    bool operator == (const LzssPacket<w, type>& rhs) const {
        return ((value == rhs.value) && (last == rhs.last)) ? true : false;
    }

    bool operator != (const LzssPacket<w, type>& rhs) const {
        return ((value != rhs.value) || (last != rhs.last)) ? true : false;
    }

    LzssPacket<w, type>& set(sc_uint<w>& other_value, bool other_last) {
        value   = other_value;
        last    = other_last;
        return *this;
    }

    sc_uint<w>  value;
    sc_uint<1>  last;
};

template <int w, LzssPacketType type>
void sc_trace(sc_trace_file* tr, const LzssPacket<w, type>& val, const string& name) {
    const string    value_name  = (type == Data) ? ".data" : ".code";
    sc_trace(tr, val.value, name + value_name);
    sc_trace(tr, val.last , name + ".last"   );
}

template <int w, LzssPacketType type>
ostream& operator <<(ostream& os, const LzssPacket<w, type>& val) {
    const string    value_name  = (type == Data) ? "data : 0x" : "code : 0x";
    const int       width       = (w + 3) / 4;
    os << value_name << hex << setw(width) << internal << setfill('0') << val.value << " "
       << "last : "  << val.last;
    return os;
}

#endif /* LZSS_PACKET_H_ */
