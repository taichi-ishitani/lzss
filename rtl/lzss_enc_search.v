/**
 *  @file   lzss_enc_search.v
 *  @brief  最長一致系列検索モジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/07/06  T. Ishitani     coding start
 *  @date   0.0.01  2012/07/18  T. Ishitani     オフセットをlzss_enc_matchから出力するように変更
 */
module lzss_enc_search #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pReferenceSize          = 64,                           //!<    参照部サイズ
    parameter   pOffsetWidth            = 6,                            //!<    オフセット幅
    parameter   pLengthWidth            = 3,                            //!<    一致長幅
    parameter   pTotalOffset            = pOffsetWidth                  //!<    入力総オフセット幅
                                        * pReferenceSize,
    parameter   pTotalLength            = pLengthWidth                  //!<    入力総一致長幅
                                        * pReferenceSize
)(
//--type-------+width------------------+name---------------------------+description
    input                               clk,                            //!<    クロック
    input                               rst_x,                          //!<    非同期リセット
    input                               i_update,                       //!<    更新
    input                               i_clear,                        //!<    クリア
    input       [pTotalOffset-1:0]      i_offset,                       //!<    入力オフセット
    input       [pTotalLength-1:0]      i_length,                       //!<    入力一致長
    input       [pReferenceSize-1:0]    i_last,                         //!<    入力最終コード
    output      [pOffsetWidth-1:0]      o_offset,                       //!<    オフセット
    output      [pLengthWidth-1:0]      o_length,                       //!<    一致長
    output                              o_last                          //!<    最終コード
);

//--type-------+name-------------------+value--------------------------+description
    localparam  lpSelNum1               = (1 << (pOffsetWidth + 1)) - 1;
    localparam  lpSelNum2               = (1 << (pOffsetWidth + 0)) - 1;

//--type-------+width------------------+name---------------------------+description
    wire        [pOffsetWidth-1:0]      w_offset[0:lpSelNum1-1];
    wire        [pLengthWidth-1:0]      w_length[0:lpSelNum1-1];
    wire                                w_last[0:lpSelNum1-1];
    reg         [pOffsetWidth-1:0]      r_offset[0:lpSelNum2-1];
    reg         [pLengthWidth-1:0]      r_length[0:lpSelNum2-1];
    reg                                 r_last[0:lpSelNum2-1];
    genvar                              i;
    genvar                              j;

//----------------------------------------------------------------------
//  最長一致長検索
//----------------------------------------------------------------------
    assign  o_offset    = w_offset[0];
    assign  o_length    = w_length[0];
    assign  o_last      = w_last[0];

    generate
        for (i = 0;i < pReferenceSize;i = i + 1) begin : input_loop
            assign  w_offset[lpSelNum2+i]  = i_offset[i*pOffsetWidth+:pOffsetWidth];
            assign  w_length[lpSelNum2+i]  = i_length[i*pLengthWidth+:pLengthWidth];
            assign    w_last[lpSelNum2+i]  = i_last[i];
        end

        for (i = 0;i < pOffsetWidth;i = i + 1) begin : search_loop1
            for (j = 0;j < (1 << i);j = j + 1) begin : search_loop2
                assign  w_offset[((1<<i)-1)+j]  = r_offset[((1<<i)-1)+j];
                assign  w_length[((1<<i)-1)+j]  = r_length[((1<<i)-1)+j];
                assign    w_last[((1<<i)-1)+j]  =   r_last[((1<<i)-1)+j];

                always @(posedge clk or negedge rst_x) begin
                    if (!rst_x) begin
                        r_offset[((1<<i)-1)+j]  <= {pOffsetWidth{1'b0}};
                        r_length[((1<<i)-1)+j]  <= {pLengthWidth{1'b0}};
                          r_last[((1<<i)-1)+j]  <= 1'b0;
                    end
                    else if (i_clear) begin
                        r_offset[((1<<i)-1)+j]  <= {pOffsetWidth{1'b0}};
                        r_length[((1<<i)-1)+j]  <= {pLengthWidth{1'b0}};
                          r_last[((1<<i)-1)+j]  <= 1'b0;
                    end
                    else if (i_update) begin
                        if (w_length[((1<<(i+1))-1)+(2*j+1)] >= w_length[((1<<(i+1))-1)+(2*j+0)]) begin
                            r_offset[((1<<i)-1)+j]  <= w_offset[((1<<(i+1))-1)+(2*j+1)];
                            r_length[((1<<i)-1)+j]  <= w_length[((1<<(i+1))-1)+(2*j+1)];
                              r_last[((1<<i)-1)+j]  <=   w_last[((1<<(i+1))-1)+(2*j+1)];
                        end
                        else begin
                            r_offset[((1<<i)-1)+j]  <= w_offset[((1<<(i+1))-1)+(2*j+0)];
                            r_length[((1<<i)-1)+j]  <= w_length[((1<<(i+1))-1)+(2*j+0)];
                              r_last[((1<<i)-1)+j]  <=   w_last[((1<<(i+1))-1)+(2*j+0)];
                        end
                    end
                end
            end
        end
    endgenerate

endmodule
