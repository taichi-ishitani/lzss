/**
 *  @file   lzss_utility.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/15  T. Ishitani     coding start
 */

#ifndef LZSS_UTILITY_H_
#define LZSS_UTILITY_H_

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "systemc.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

template <int n>
struct Log2 {
    enum {
        value   = Log2<n / 2>::value + 1
    };
};

template <>
struct Log2<1> {
    enum {
        value   = 0
    };
};

template <typename T, int W>
void read_file(string& file, vector<T>& stream, const char* id) {
    ifstream        ifs(file.c_str(), ios::in | ios::binary);
    stringstream    message;
    char            byte;
    unsigned int    data;
    int             byte_pos;
    int             data_pos;
    unsigned int    byte_count;

    if (!ifs.good()) {
        message << sc_time_stamp() << "\nCould not be opened : " << file;
        SC_REPORT_FATAL(id, message.str().c_str());
        return;
    }

    data        = 0;
    data_pos    = W;
    byte_count  = 0;
    stream.clear();
    while (true) {
        ifs.get(byte);
        if (ifs.eof()) {
            break;
        }

        byte_count  += 1;
        for (byte_pos = 8;byte_pos > 0;byte_pos--) {
            data    |= (byte >> (byte_pos - 1) & 0x1) << (data_pos - 1);
            if (data_pos == 1) {
                stream.push_back((T)data);
                data        = 0;
                data_pos    = W;
            }
            else {
                data_pos    -= 1;
            }
        }
    }
    ifs.close();

    message << sc_time_stamp() << "\nRead : " << file << " (" << byte_count << " bytes)";
    SC_REPORT_INFO(id, message.str().c_str());
}

template <typename T, int W>
void write_file(string& file, vector<T>& stream, const char* id) {
    ofstream        ofs(file.c_str(), ios::out | ios::binary);
    stringstream    message;
    char            byte;
    unsigned int    data;
    int             byte_pos;
    int             data_pos;
    unsigned int    byte_count;

    if (!ofs.good()) {
        message << sc_time_stamp() << "\nCould not be opened : " << file;
        SC_REPORT_FATAL(id, message.str().c_str());
        return;
    }

    byte        = 0;
    byte_pos    = 8;
    byte_count  = 0;
    for (size_t i = 0;i < stream.size();i++) {
        data    = (unsigned int)stream[i];

        for (data_pos = W;data_pos > 0;data_pos--) {
            byte    |= ((data >> (data_pos - 1)) & 0x1) << (byte_pos - 1);
            if (byte_pos == 1) {
                ofs.put(byte);
                byte        = 0;
                byte_pos    = 8;
                byte_count  += 1;
            }
            else {
                byte_pos    -= 1;
            }
        }
    }
    if (byte_pos != 8) {
        ofs.put(byte);
        byte_count  += 1;
    }
    ofs.close();
    stream.clear();

    message << sc_time_stamp() << "\nWrite : " << file << " (" << byte_count << " bytes)";
    SC_REPORT_INFO(id, message.str().c_str());
}

struct Stimulus {
    vector<string>  list;
    string          data_in_dir;
    string          code_out_dir;
    string          data_out_dir;
    string          dump_dir;

    typedef vector<string>::iterator    iterator;

    void add_file(const char* file) {
        list.push_back(file);
    }

    size_t list_size() {
        return list.size();
    }

    iterator begin() {
        return list.begin();
    }

    iterator end() {
        return list.end();
    }
};

struct TimeConstraint {
    TimeConstraint(double cycle_time_value, double time_out_value, sc_time_unit unit) :
        cycle_time  (cycle_time_value, unit),
        time_out    (time_out_value  , unit)
    {};

    sc_time cycle_time;
    sc_time time_out;
};

#endif /* LZSS_UTILITY_H_ */
