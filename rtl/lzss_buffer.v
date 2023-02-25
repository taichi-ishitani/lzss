/**
 *  @file   lzss_buffer.v
 *  @brief  共通バッファモジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/07/06  T. Ishitani     coding start
 */
module lzss_buffer #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pWidth                  = 8,                            //!<    入力幅
    parameter   pDepth                  = 64,                           //!<    深さ
    parameter   pTotalWidth             = pWidth * pDepth               //!<    出力幅
)(
//--type-------+width------------------+name---------------------------+description
    input                               clk,                            //!<    クロック
    input                               rst_x,                          //!<    非同期リセット
    input                               i_clear,                        //!<    クリア
    input                               i_shift,                        //!<    シフトイネーブル
    input       [pWidth-1:0]            i_d,                            //!<    入力データ
    output      [pTotalWidth-1:0]       o_d                             //!<    出力データ(pDepth分)
);

//--type-------+width------------------+name---------------------------+description
    reg         [pWidth-1:0]            r_d[0:pDepth-2];
    wire        [pWidth-1:0]            w_d[0:pDepth-1];
    genvar                              i;

    generate
        for (i = 0;i < pDepth;i = i + 1) begin : buffer_loop
            assign  o_d[i*pWidth+:pWidth]   = w_d[i];

            if (i == (pDepth - 1)) begin : last
                assign  w_d[i]  = i_d;
            end
            else begin : other
                assign  w_d[i]  = r_d[i];

                always @(posedge clk or negedge rst_x) begin
                    if (!rst_x) begin
                        r_d[i]  <= {pWidth{1'b0}};
                    end
                    else if (i_clear) begin
                        r_d[i]  <= {pWidth{1'b0}};
                    end
                    else if (i_shift) begin
                        r_d[i]  <= w_d[i+1];
                    end
                end
            end
        end
    endgenerate

endmodule
