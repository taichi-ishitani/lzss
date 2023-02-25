/**
 *  @file   lzss_enc_match.v
 *  @brief  一致比較モジュール
 *
 *  @par    Copyright
 *  (C) 2012 Taichi Ishitani All Rights Reserved.
 *
 *  @author Taichi Ishitani
 *
 *  @date   0.0.00  2012/07/05  T. Ishitani     coding start
 *  @date   0.0.01  2012/07/18  T. Ishitani     オフセットをlzss_enc_matchから出力するように変更
 */
module lzss_enc_match #(
//--type-------+name-------------------+value--------------------------+description
    parameter   pOffset                 = 0,                            //!<    オフセット
    parameter   pDataWidth              = 8,                            //!<    データ幅
    parameter   pCodingSize             = 5,                            //!<    符号化部サイズ
    parameter   pOffsetWidth            = 6,                            //!<    オフセット幅
    parameter   pLengthWidth            = 3,                            //!<    一致長幅
    parameter   pTotalData              = pDataWidth * pCodingSize      //!<    入力総データ幅
)(
//--type-------+width------------------+name---------------------------+description
    input                               clk,                            //!<    クロック
    input                               rst_x,                          //!<    非同期リセット
    input                               i_update,                       //!<    更新
    input                               i_clear,                        //!<    クリア
    input       [pCodingSize-1:0]       i_valid,                        //!<    入力データバリッド
    input       [pTotalData-1:0]        i_data,                         //!<    入力データ
    input       [pCodingSize-1:0]       i_last,                         //!<    入力データラスト
    input       [pCodingSize-1:0]       i_ref_valid,                    //!<    参照データバリッド
    input       [pTotalData-1:0]        i_ref_data,                     //!<    参照データ
    output      [pOffsetWidth-1:0]      o_offset,                       //!<    オフセット
    output      [pLengthWidth-1:0]      o_length,                       //!<    一致長
    output                              o_last                          //!<    最終コード
);

//--type-------+width------------------+name---------------------------+description
    wire        [pCodingSize-1:0]       w_each_match;
    wire        [pCodingSize-1:0]       w_match;
    reg         [pCodingSize-1:0]       r_match;
    wire        [pCodingSize-1:0]       w_sel;
    wire        [pLengthWidth-1:0]      w_length;
    wire                                w_last;
    reg         [pLengthWidth-1:0]      r_length;
    reg         [1:0]                   r_last;
    genvar                              i;
    genvar                              j;

//----------------------------------------------------------------------
//  一致比較
//----------------------------------------------------------------------
    generate
        for (i = 0;i < pCodingSize;i = i + 1) begin : each_matching_loop
            assign  w_each_match[i] = (
                i_valid[i] &&
                i_ref_valid[i] &&
                (i_data[i*pDataWidth+:pDataWidth] == i_ref_data[i*pDataWidth+:pDataWidth])
            ) ? 1'b1 : 1'b0;
        end

        for (i = 0;i < pCodingSize;i = i + 1) begin : matching_loop
            if (i == 0) begin : first
                assign  w_match[i]  = w_each_match[i];
            end
            else begin : other
                assign  w_match[i]  = &w_each_match[i:0];
            end
        end
    endgenerate

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_match <= {pCodingSize{1'b0}};
        end
        else if (i_clear) begin
            r_match <= {pCodingSize{1'b0}};
        end
        else if (i_update) begin
            r_match <= w_match;
        end
    end

//----------------------------------------------------------------------
//  出力生成
//----------------------------------------------------------------------
    assign  o_offset    = pOffset[pOffsetWidth-1:0];
    assign  o_length    = r_length;
    assign  o_last      = r_last[1];

    //  一致長
    generate
        for (i = 0;i < pCodingSize;i = i + 1) begin : sel_loop
            if (i == (pCodingSize - 1)) begin : last
                assign  w_sel[i]    = r_match[i];
            end
            else begin : other
                assign  w_sel[i]    = r_match[i] & (~r_match[i+1]);
            end
        end

        for (i = 0;i < pLengthWidth;i = i + 1) begin : length_loop1
            wire    [pCodingSize-1:0]   w_length_temp;

            assign  w_length[i] = |(w_sel & w_length_temp);
            for (j = 0;j < pCodingSize;j = j + 1) begin : length_loop2
                assign  w_length_temp[j]    = j[i];
            end
        end
    endgenerate

    //  ラスト信号
    assign  w_last  = (|(i_last & w_match)) | i_last[0];

    always @(posedge clk or negedge rst_x) begin
        if (!rst_x) begin
            r_length    <= {pLengthWidth{1'b0}};
            r_last      <= 2'b00;
        end
        else if (i_clear) begin
            r_length    <= {pLengthWidth{1'b0}};
            r_last      <= 2'b00;
        end
        else if (i_update) begin
            r_length    <= w_length;
            r_last      <= {r_last[0], w_last};
        end
    end

endmodule
