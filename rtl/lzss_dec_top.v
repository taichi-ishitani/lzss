/**
 *  @file   lzss_dec_top.v
 *  @brief  LZSSデコーダトップモジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/26  T. Ishitani     coding start
 */
module lzss_dec_top #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pDataWidth              = 8,                            //<!    データ幅
    parameter   pReferenceSize          = 64,                           //<!    参照部サイズ
    parameter   pCodingSize             = 5,                            //<!    符号化部サイズ
    parameter   pCodeWidth              = get_code_width(               //<!    コード幅
                                            pDataWidth,
                                            pReferenceSize,
                                            pCodingSize
                                          )
)(
//--type-------+width------------------+name---------------------------+description
    //  システム
    input                               clk,                            //<!    クロック
    input                               rst_x,                          //<!    非同期リセット
    //  コード入力
    input                               i_valid,                        //<!    入力コードバリッド
    output                              ow_ready,                       //<!    入力コードレディ
    input       [pCodeWidth-1:0]        i_code,                         //<!    入力コード
    input                               i_last,                         //<!    入力コードラスト
    //  データ出力
    output                              o_valid,                        //<!    出力データバリッド
    input                               i_ready,                        //<!    出力データレディ
    output      [pDataWidth-1:0]        o_data,                         //<!    出力データ
    output                              o_last                          //<!    出力データラスト
);

    `include "lzss_function.vh"

//--type-------+name-------------------+value--------------------------+description
    localparam  lpOffsetWidth           = log2(pReferenceSize);
    localparam  lpLengthWidth           = log2(pCodingSize) + 1;
    localparam  lpDataBufferSize        = pDataWidth * pReferenceSize;
    localparam  lpSelSize               = pReferenceSize + 1;

//--type-------+width------------------+name---------------------------+description
    wire                                w_valid;
    wire                                w_ready;
    reg                                 r_valid;
    wire                                w_out_ack;
    wire                                w_no_data;
    wire                                w_last;
    wire                                w_flag;
    wire                                w_matched;
    wire                                w_unmatched;
    wire        [pDataWidth-1:0]        w_data_or_offset;
    wire        [lpLengthWidth-1:0]     w_length;
    wire                                w_shift;
    wire                                w_new_code;
    wire                                w_last_data_done;
    reg         [lpLengthWidth-1:0]     r_shift_count;
    wire                                w_shift_count_ne_0;
    wire                                w_shift_count_eq_0;
    wire                                w_shift_count_eq_1;
    wire        [lpDataBufferSize-1:0]  w_data_buffer;
    reg         [pDataWidth-1:0]        r_data;
    reg                                 r_last;
    wire        [pDataWidth-1:0]        w_data;
    wire        [pDataWidth-1:0]        w_sel_data[0:lpSelSize-1];
    wire        [lpSelSize-1:0]         w_sel;
    genvar                              i;
    genvar                              j;

//----------------------------------------------------------------------
//  デコード
//----------------------------------------------------------------------
    lzss_dec_decode #(
        .pDataWidth     (pDataWidth     ),
        .pCodeWidth     (pCodeWidth     ),
        .pOffsetWidth   (lpOffsetWidth  ),
        .pLengthWidth   (lpLengthWidth  )
    ) u_c_decode (
        .clk                (clk                ),
        .rst_x              (rst_x              ),
        .i_valid            (i_valid            ),
        .ow_ready           (ow_ready           ),
        .i_code             (i_code             ),
        .i_last             (i_last             ),
        .o_valid            (w_valid            ),
        .i_ready            (w_ready            ),
        .o_last             (w_last             ),
        .o_flag             (w_flag             ),
        .o_data_or_offset   (w_data_or_offset   ),
        .o_length           (w_length           )
    );

    assign  w_matched   =  w_flag;
    assign  w_unmatched = ~w_flag;

//----------------------------------------------------------------------
//  入出力ハンドシェイク
//----------------------------------------------------------------------
    assign  o_valid = r_valid;

    assign  w_ready     = ((w_out_ack & (w_shift_count_eq_1 | w_unmatched)) | (w_unmatched & w_no_data)) & (~r_last);
    assign  w_out_ack   = r_valid & i_ready;
    assign  w_no_data   = ~r_valid;

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_valid <= 1'b0;
        end
        else if (w_new_code) begin
            r_valid <= 1'b1;
        end
        else if (w_out_ack && w_shift_count_eq_0) begin
            r_valid <= 1'b0;
        end
    end

//----------------------------------------------------------------------
//  バッファ更新/出力タイミング制御
//----------------------------------------------------------------------
    assign  w_new_code          = (w_no_data | (w_shift_count_eq_0 & w_out_ack)) & w_valid;
    assign  w_shift             = w_new_code | (w_out_ack & w_shift_count_ne_0);
    assign  w_last_data_done    = w_out_ack & r_last;

    assign  w_shift_count_ne_0  = |r_shift_count;
    assign  w_shift_count_eq_0  = ~w_shift_count_ne_0;
    assign  w_shift_count_eq_1  = (~|r_shift_count[lpLengthWidth-1:1]) & r_shift_count[0];
    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_shift_count   <= {lpLengthWidth{1'b0}};
        end
        else if (w_new_code) begin
            r_shift_count   <= w_length;
        end
        else if (w_out_ack && w_shift_count_ne_0) begin
            r_shift_count   <= r_shift_count + {lpLengthWidth{1'b1}};
        end
    end

//----------------------------------------------------------------------
//  参照データバッファ
//----------------------------------------------------------------------
    lzss_buffer #(
        .pWidth (pDataWidth     ),
        .pDepth (pReferenceSize )
    ) u_data_buffer (
        .clk        (clk                ),
        .rst_x      (rst_x              ),
        .i_clear    (w_last_data_done   ),
        .i_shift    (w_shift            ),
        .i_d        (r_data             ),
        .o_d        (w_data_buffer      )
    );

//----------------------------------------------------------------------
//  データ出力
//----------------------------------------------------------------------
    assign  o_data  = r_data;
    assign  o_last  = r_last;

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_data  <= {pDataWidth{1'b0}};
        end
        else if (w_last_data_done) begin
            r_data  <= {pDataWidth{1'b0}};
        end
        else if (w_shift) begin
            r_data  <= w_data;
        end
    end

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_last  <= 1'b0;
        end
        else if (w_last_data_done) begin
            r_last  <= 1'b0;
        end
        else if (w_shift && w_last) begin
            if (w_unmatched || w_shift_count_eq_1) begin
                r_last  <= 1'b1;
            end
        end
    end

    generate
        for (i = 0;i < lpSelSize;i = i + 1) begin : data_sel_loop
            if (i < (lpSelSize - 1)) begin : matched
                assign  w_sel[i]        = ((w_data_or_offset[lpOffsetWidth-1:0] == i[lpOffsetWidth-1:0]) && w_matched) ? 1'b1 : 1'b0;
                assign  w_sel_data[i]   = w_data_buffer[i*pDataWidth+:pDataWidth];
            end
            else begin : unmatced
                assign  w_sel[i]        = w_unmatched;
                assign  w_sel_data[i]   = w_data_or_offset[pDataWidth-1:0];
            end
        end

        for (i = 0;i < pDataWidth;i = i + 1) begin : data_loop1
            wire    [lpSelSize-1:0] w_data_temp;

            assign  w_data[i]   = |(w_sel & w_data_temp);
            for (j = 0;j < lpSelSize;j = j + 1) begin : data_loop2
                assign  w_data_temp[j]  = w_sel_data[j][i];
            end
        end
    endgenerate

endmodule
