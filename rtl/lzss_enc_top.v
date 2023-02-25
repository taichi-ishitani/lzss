/**
 *  @file   lzss_enc_top.v
 *  @brief  LZSSエンコーダトップモジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/07/05  T. Ishitani     coding start
 *  @date   0.0.01  2012/07/18  T. Ishitani     オフセットをlzss_enc_matchから出力するように変更
 */
module lzss_enc_top #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pDataWidth              = 8,                            //!<    データ幅
    parameter   pReferenceSize          = 64,                           //!<    参照部サイズ
    parameter   pCodingSize             = 5,                            //!<    符号化部サイズ
    parameter   pCodeWidth              = get_code_width(               //!<    コード幅
                                            pDataWidth,
                                            pReferenceSize,
                                            pCodingSize
                                          )
)(
//--type-------+width------------------+name---------------------------+description
    //  システム
    input                               clk,                            //!<    クロック
    input                               rst_x,                          //!<    非同期リセット
    //  データ入力
    input                               i_valid,                        //!<    入力データバリッド
    output                              ow_ready,                       //!<    入力データレディ
    input       [pDataWidth-1:0]        i_data,                         //!<    入力データ
    input                               i_last,                         //!<    入力データラスト
    //  コード出力
    output                              o_valid,                        //!<    出力コードバリッド
    input                               i_ready,                        //!<    出力コードレディ
    output      [pCodeWidth-1:0]        o_code,                         //!<    出力コード
    output                              o_last                          //!<    出力コードラスト
);

    `include "lzss_function.vh"

//--type-------+name-------------------+value--------------------------+description
    localparam  lpWindowSize            = pReferenceSize + pCodingSize;
    localparam  lpOffsetWidth           = log2(pReferenceSize);
    localparam  lpLengthWidth           = log2(pCodingSize) + 1;
    localparam  lpMatchingData          = pCodingSize    * pDataWidth;
    localparam  lpTotalData             = lpWindowSize   * pDataWidth;
    localparam  lpTotalOffset           = pReferenceSize * lpOffsetWidth;
    localparam  lpTotalLength           = pReferenceSize * lpLengthWidth;
    localparam  lpOutputIndex           = pReferenceSize
                                        - lpOffsetWidth
                                        - 2;

//--type-------+width------------------+name---------------------------+description
    wire                                w_ready;
    reg                                 r_valid;
    wire                                w_in_ack;
    wire                                w_out_ack;
    wire                                w_new_code;
    wire                                w_shift_enable;
    wire                                w_shift;
    reg         [lpLengthWidth-1:0]     r_shift_count;
    wire                                w_shift_count_ne_0;
    wire                                w_shift_count_eq_0;
    reg                                 r_last_data_done;
    wire                                w_last_code_done;
    wire                                w_in_valid;
    wire        [lpWindowSize-1:0]      w_valid_buffer;
    wire        [lpTotalData-1:0]       w_data_buffer;
    wire        [pCodingSize-1:0]       w_last_buffer;
    wire        [lpTotalOffset-1:0]     w_each_offset;
    wire        [lpTotalLength-1:0]     w_each_length;
    wire        [pReferenceSize-1:0]    w_each_last;
    wire        [lpOffsetWidth-1:0]     w_offset;
    wire        [lpLengthWidth-1:0]     w_length;
    wire                                w_last;
    wire                                w_match;
    wire        [lpLengthWidth-2:0]     w_out_length;
    wire        [pDataWidth-1:0]        w_out_data;
    wire        [pCodeWidth-1:0]        w_match_code;
    wire        [pCodeWidth-1:0]        w_unmatch_code;
    wire        [pCodeWidth-1:0]        w_code;
    reg         [pCodeWidth-1:0]        r_code;
    reg                                 r_last;
    genvar                              i;

//----------------------------------------------------------------------
//  入出力ハンドシェイク
//----------------------------------------------------------------------
    assign  ow_ready    = w_ready;
    assign  o_valid     = r_valid;

    assign  w_ready     = w_shift_enable & (~r_last_data_done);
    assign  w_in_ack    = i_valid & w_ready;
    assign  w_out_ack   = r_valid & i_ready;

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_valid <= 1'b0;
        end
        else if (w_new_code) begin
            r_valid <= 1'b1;
        end
        else if (w_out_ack) begin
            r_valid <= 1'b0;
        end
    end

//----------------------------------------------------------------------
//  バッファ更新タイミング/出力タイミング制御
//----------------------------------------------------------------------
    assign  w_shift_enable      = w_shift_count_ne_0 | (w_shift_count_eq_0 & (w_out_ack | (~r_valid)));
    assign  w_shift             = w_shift_enable & (w_in_ack | r_last_data_done);
    assign  w_new_code          = w_shift_count_eq_0 & w_shift & w_valid_buffer[lpOutputIndex];
    assign  w_last_code_done    = w_out_ack & r_last;

    assign  w_shift_count_ne_0  = |r_shift_count;
    assign  w_shift_count_eq_0  = ~w_shift_count_ne_0;
    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_shift_count   <= {lpLengthWidth{1'b0}};
        end
        else if (w_last_code_done) begin
            r_shift_count   <= {lpLengthWidth{1'b0}};
        end
        else if (w_new_code) begin
            r_shift_count   <= w_length;
        end
        else if (w_shift && w_shift_count_ne_0) begin
            r_shift_count   <= r_shift_count + {lpLengthWidth{1'b1}};
        end
    end

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_last_data_done    <= 1'b0;
        end
        else if (w_last_code_done) begin
            r_last_data_done    <= 1'b0;
        end
        else if (w_in_ack && i_last) begin
            r_last_data_done    <= 1'b1;
        end
    end

//----------------------------------------------------------------------
//  バッファ
//----------------------------------------------------------------------
    //  バリッド
    assign  w_in_valid  = i_valid & (~r_last_data_done);
    lzss_buffer #(
        .pWidth (1              ),
        .pDepth (lpWindowSize   )
    ) u_valid_buffer (
        .clk        (clk                ),
        .rst_x      (rst_x              ),
        .i_clear    (w_last_code_done   ),
        .i_shift    (w_shift            ),
        .i_d        (w_in_valid         ),
        .o_d        (w_valid_buffer     )
    );
    //  データ
    lzss_buffer #(
        .pWidth (pDataWidth     ),
        .pDepth (lpWindowSize   )
    ) u_data_buffer (
        .clk        (clk                ),
        .rst_x      (rst_x              ),
        .i_clear    (w_last_code_done   ),
        .i_shift    (w_shift            ),
        .i_d        (i_data             ),
        .o_d        (w_data_buffer      )
    );
    //  ラスト
    lzss_buffer #(
        .pWidth (1              ),
        .pDepth (pCodingSize    )
    ) u_last_buffer (
        .clk        (clk                ),
        .rst_x      (rst_x              ),
        .i_clear    (w_last_code_done   ),
        .i_shift    (w_shift            ),
        .i_d        (i_last             ),
        .o_d        (w_last_buffer      )
    );

//----------------------------------------------------------------------
//  最長一致系列検索
//----------------------------------------------------------------------
    //  一致比較
    generate
        for (i = 0;i < pReferenceSize;i = i + 1) begin : matching_loop
            lzss_enc_match #(
                .pOffset        (i              ),
                .pDataWidth     (pDataWidth     ),
                .pCodingSize    (pCodingSize    ),
                .pOffsetWidth   (lpOffsetWidth  ),
                .pLengthWidth   (lpLengthWidth  )
            ) u_match (
                .clk            (clk                                            ),
                .rst_x          (rst_x                                          ),
                .i_update       (w_shift                                        ),
                .i_clear        (w_last_code_done                               ),
                .i_valid        (w_valid_buffer[lpWindowSize-1-:pCodingSize]    ),
                .i_data         (w_data_buffer[lpTotalData-1-:lpMatchingData]   ),
                .i_last         (w_last_buffer[pCodingSize-1:0]                 ),
                .i_ref_valid    (w_valid_buffer[i+:pCodingSize]                 ),
                .i_ref_data     (w_data_buffer[i*pDataWidth+:lpMatchingData]    ),
                .o_offset       (w_each_offset[i*lpOffsetWidth+:lpOffsetWidth]  ),
                .o_length       (w_each_length[i*lpLengthWidth+:lpLengthWidth]  ),
                .o_last         (w_each_last[i]                                 )
            );
        end
    endgenerate

    //  検索
    lzss_enc_search #(
        .pReferenceSize (pReferenceSize ),
        .pOffsetWidth   (lpOffsetWidth  ),
        .pLengthWidth   (lpLengthWidth  )
    ) u_search (
        .clk        (clk                ),
        .rst_x      (rst_x              ),
        .i_update   (w_shift            ),
        .i_clear    (w_last_code_done   ),
        .i_offset   (w_each_offset      ),
        .i_length   (w_each_length      ),
        .i_last     (w_each_last        ),
        .o_offset   (w_offset           ),
        .o_length   (w_length           ),
        .o_last     (w_last             )
    );

//----------------------------------------------------------------------
//  エンコード
//----------------------------------------------------------------------
    assign  o_code  = r_code;
    assign  o_last  = r_last;

    assign  w_match         = |w_length;
    assign  w_out_length    = w_length[lpLengthWidth-2:0] + {(lpLengthWidth-1){1'b1}};
    assign  w_out_data      = w_data_buffer[lpOutputIndex*pDataWidth+:pDataWidth];
    assign  w_match_code    = {1'b1, w_offset, w_out_length};
    assign  w_unmatch_code  = {{(pCodeWidth-pDataWidth){1'b0}}, w_out_data};
    assign  w_code          = (w_match) ? w_match_code : w_unmatch_code;

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_code  <= {pCodeWidth{1'b0}};
            r_last  <= 1'b0;
        end
        else if (w_last_code_done) begin
            r_code  <= {pCodeWidth{1'b0}};
            r_last  <= 1'b0;
        end
        else if (w_new_code) begin
            r_code  <= w_code;
            r_last  <= w_last;
        end
    end

endmodule
