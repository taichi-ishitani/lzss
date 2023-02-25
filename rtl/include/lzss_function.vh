/**
 *  @file   lzss_function.vh
 *  @brief  共通関数
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/26  T. Ishitani     coding start
 */

function integer log2 (
    input   integer n
);
    integer val;
    for (val = 0;n > 0;val = val + 1) begin
        n       = n / 2;
        log2    = val;
    end
endfunction

function integer get_code_width (
    input   integer data_width,
    input   integer reference_size,
    input   integer coding_size
);
    if (data_width <= (log2(reference_size) + log2(coding_size - 1))) begin
        get_code_width  = 1
                        + log2(reference_size )
                        + log2(coding_size - 1);
    end
    else begin
        get_code_width  = 0;
    end
endfunction
