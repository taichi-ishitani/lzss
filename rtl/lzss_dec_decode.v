/**
 *  @file   lzss_dec_decode.v
 *  @brief  デコードモジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/06/26  T. Ishitani     coding start
 */
module lzss_dec_decode #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pDataWidth              = 8,                            //!<    データ幅
    parameter   pCodeWidth              = 9,                            //!<    符号化部サイズ
    parameter   pOffsetWidth            = 6,                            //!<    オフセット幅
    parameter   pLengthWidth            = 3                             //!<    一致長幅
)(
//--type-------+width------------------+name---------------------------+description
    input                               clk,                            //!<    クロック
    input                               rst_x,                          //!<    非同期リセット
    input                               i_valid,                        //!<    入力コードバリッド
    output                              ow_ready,                       //!<    入力コードレディ
    input       [pCodeWidth-1:0]        i_code,                         //!<    入力コード
    input                               i_last,                         //!<    入力コードラスト
    output                              o_valid,                        //!<    出力バリッド
    input                               i_ready,                        //!<    出力レディ
    output                              o_last,                         //!<    出力ラスト
    output                              o_flag,                         //!<    一致フラグ
    output      [pDataWidth-1:0]        o_data_or_offset,               //!<    データ/オフセット
    output      [pLengthWidth-1:0]      o_length                        //!<    一致長
);

//--type-------+width------------------+name---------------------------+description
    wire                                w_ready;
    wire                                w_in_ack;
    reg                                 r_valid;
    wire                                w_out_ack;
    wire                                w_flag;
    wire        [pDataWidth-1:0]        w_data;
    wire        [pDataWidth-1:0]        w_offset;
    wire        [pLengthWidth-1:0]      w_length;
    reg                                 r_last;
    reg                                 r_flag;
    reg         [pDataWidth-1:0]        r_data_or_offset;
    reg         [pLengthWidth-1:0]      r_length;

//----------------------------------------------------------------------
//  入出力ハンドシェイク
//----------------------------------------------------------------------
    assign  ow_ready    = w_ready;
    assign  o_valid     = r_valid;
    assign  o_last      = r_last;

    assign  w_ready     = ((~r_valid) | w_out_ack) & (~r_last);
    assign  w_in_ack    = i_valid & w_ready;
    assign  w_out_ack   = r_valid & i_ready;

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_valid <= 1'b0;
        end
        else if (w_in_ack) begin
            r_valid <= 1'b1;
        end
        else if (w_out_ack) begin
            r_valid <= 1'b0;
        end
    end

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_last  <= 1'b0;
        end
        else if (w_in_ack) begin
            r_last  <= i_last;
        end
        else if (w_out_ack) begin
            r_last  <= 1'b0;
        end
    end

//----------------------------------------------------------------------
//  デコード処理
//----------------------------------------------------------------------
    assign  o_flag              = r_flag;
    assign  o_data_or_offset    = r_data_or_offset;
    assign  o_length            = r_length;

    assign  w_flag      = i_code[pCodeWidth-1];
    assign  w_length    = {1'b0, i_code[pLengthWidth-2:0]} + {{(pLengthWidth-1){1'b0}}, 1'b1};
    generate
        if (pDataWidth > pOffsetWidth) begin : dw_gt_ow
            assign  w_data      = i_code[pDataWidth-1:0];
            assign  w_offset    = {{(pDataWidth-pOffsetWidth){1'b0}}, i_code[pCodeWidth-2:pLengthWidth-1]};
        end
        else if (pDataWidth == pOffsetWidth) begin : dw_eq_ow
            assign  w_data      = i_code[pDataWidth-1:0];
            assign  w_offset    = i_code[pCodeWidth-2:pLengthWidth-1];
        end
        else begin : dw_lt_ow
            assign  w_data      = {{(pOffsetWidth-pDataWidth){1'b0}}, i_code[pDataWidth-1:0]};
            assign  w_offset    = i_code[pCodeWidth-2:pLengthWidth-1];
        end
    endgenerate

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_flag              <= 1'b0;
            r_data_or_offset    <= {pDataWidth{1'b0}};
            r_length            <= {pLengthWidth{1'b0}};
        end
        else if (r_last && w_out_ack) begin
            r_flag              <= 1'b0;
            r_data_or_offset    <= {pDataWidth{1'b0}};
            r_length            <= {pLengthWidth{1'b0}};
        end
        else if (w_in_ack) begin
            if (w_flag) begin
                r_flag              <= 1'b1;
                r_data_or_offset    <= w_offset;
                r_length            <= w_length;
            end
            else begin
                r_flag              <= 1'b0;
                r_data_or_offset    <= w_data;
                r_length            <= {pLengthWidth{1'b0}};
            end
        end
    end

endmodule
