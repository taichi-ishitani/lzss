/**
 *  @file   main.cpp
 *  @brief
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/08  T. Ishitani     coding start
 */

#include <iostream>
#include <cstdlib>
#include "lzss.h"

using namespace std;

#define ReferenceSize   128
#define CodingSize      5

typedef Lzss<ReferenceSize, CodingSize> lzss_t;

int main(int argc, char* argv[]) {
    string          data_in;
    string          code_out;
    string          data_out;
    string          diff_command;
    data_stream_t*  data_in_stream;
    code_stream_t*  code_out_stream;
    data_stream_t*  data_out_stream;
    lzss_t          enc;
    lzss_t          dec;

    for (int i = 1;i < argc;i++) {
        data_in     = string("../sample/") + string(argv[i]);
        code_out    = string("./encode/")  + string(argv[i]) + string(".bin");
        data_out    = string("./decode/")  + string(argv[i]);

        data_in_stream  = read_file<8, data_t>(data_in);
        code_out_stream = enc.encode(data_in_stream);
        data_out_stream = dec.decode(code_out_stream);
        write_file<lzss_t::code_width, code_t>(code_out, code_out_stream);
        write_file<8, data_t>(data_out, data_out_stream);

        diff_command    = string("diff ") + data_in + string(" ") + data_out;
        system(diff_command.c_str());

        cout << endl;

        enc.clear();
        dec.clear();
        delete data_in_stream;
        delete code_out_stream;
        delete data_out_stream;
    }
    return 0;
}
