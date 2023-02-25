/**
 *  @file   utility.h
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/04  T. Ishitani     coding start
 */

#ifndef UTILITY_H_
#define UTILITY_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

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

template <int W, typename T>
void write_file(string& file, vector<T>* output_stream) {
    ofstream    ofs(file.c_str(), ios::binary | ios::out);
    char        byte;
    T           data;
    int         byte_pos;
    int         data_pos;
    int         byte_count;

    byte_pos    = 8;
    byte        = 0;
    byte_count  = 0;
    for (typename vector<T>::iterator i = output_stream->begin();i != output_stream->end();++i) {
        data    = *i;

        for (data_pos = W;data_pos > 0;data_pos--) {
            byte    |= ((data >> (data_pos - 1)) & 0x1) << (byte_pos - 1);
            if (byte_pos == 1) {
                ofs.put(byte);
                byte_pos    = 8;
                byte        = 0;
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

    cout << "Write       : " << file                  << endl;
    cout << "Output Size : " << byte_count << "bytes" << endl;
}

template <int W, typename T>
vector<T>* read_file(string& file) {
    vector<T>*  output  = new vector<T>;
    ifstream    ifs(file.c_str(), ios::binary);
    T           data;
    char        byte;
    int         data_pos;
    int         byte_pos;

    data_pos    = W;
    data        = 0;
    while (1) {
        ifs.get(byte);
        if (ifs.eof()) {
            break;
        }

        for (byte_pos = 8;byte_pos > 0;byte_pos--) {
            data    |= ((byte >> (byte_pos - 1)) & 0x1) << (data_pos - 1);
            if (data_pos == 1) {
                output->push_back(data);
                data_pos    = W;
                data        = 0;
            }
            else {
                data_pos    -= 1;
            }
        }
    }

    cout << "Read        : " << file << endl;
    cout << "Input Size  : " << output->size() << "bytes" << endl;

    return output;
}

#endif /* UTILITY_H_ */
