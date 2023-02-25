`ifndef dDataWidth
`define dDataWidth      8
`endif

`ifndef dReferenceSize
`define dReferenceSize  64
`endif

`ifndef dCodingSize
`define dCodingSize     5
`endif

module dut_dec #(
    parameter   pDataWidth      = `dDataWidth,
    parameter   pReferenceSize  = `dReferenceSize,
    parameter   pCodingSize     = `dCodingSize,
    parameter   pCodeWidth      = get_code_width(
                                            pDataWidth,
                                            pReferenceSize,
                                            pCodingSize
                                          )
)(
    input                           clk,
    input                           rst_x,
    input                           i_valid,
    output                          ow_ready,
    input       [pCodeWidth-1:0]    i_code,
    input                           i_last,
    output                          o_valid,
    input                           i_ready,
    output      [pDataWidth-1:0]    o_data,
    output                          o_last
);

    `include "lzss_function.vh"

    lzss_dec_top #(
        .pDataWidth     (pDataWidth     ),
        .pReferenceSize (pReferenceSize ),
        .pCodingSize    (pCodingSize    )
    ) u_dut_dec (
        .clk        (clk        ),
        .rst_x      (rst_x      ),
        .i_valid    (i_valid    ),
        .ow_ready   (ow_ready   ),
        .i_code     (i_code     ),
        .i_last     (i_last     ),
        .o_valid    (o_valid    ),
        .i_ready    (i_ready    ),
        .o_data     (o_data     ),
        .o_last     (o_last     )
    );

endmodule
