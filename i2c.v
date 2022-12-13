// I2C IP Module
// I2C Verilog Implementation (i2c.v)
// Ulysses Chaparro (1001718774) & Angel Montelongo (1001665238)

//Jason Losh origin code

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

//-----------------------------------------------------------------------------

module i2c (clk, reset, address, byteenable, chipselect, writedata, 
			readdata, write, read, i2c_scl, i2c_sda, test_out,
			debug_address, debug_chipselect, debug_read, debug_readdata, 
			debug_write, debug_writedata, rd_index, wr_index);

    // Clock, reset
    input	clk, reset;

    // Avalon MM interface (8 word aperature) 
    input              read, write, chipselect, byteenable;
    input [1:0]        address;
    input [31:0]       writedata;
    output reg [31:0]  readdata;
    
    // i2c bus/test signals conduit 
    output reg 		 i2c_scl;
    inout reg		 i2c_sda;
    output	    	 test_out;

    //debug conduit 
    output 	       debug_read, debug_write, debug_chipselect; 		 
    output [1:0]       debug_address; 
    output [4:0]       debug_writedata; 
    output [4:0]       debug_readdata; 
    output [4:0]       rd_index; 
    output [4:0]       wr_index;

    assign debug_read = read;
    assign debug_write = write; 
    assign debug_chipselect = chipselect;
    assign debug_address = address; 
    assign debug_writedata[3:0] = d;
    assign debug_writedata[4] = repeatbit;
    assign debug_readdata = state [4:0];
    assign rd_index = rd_index_tx [4:0]; 
    assign wr_index = state [4:0]; //wr_index_tx [4:0];

    // internal registers 
    reg [6:0]  address_reg;
    reg [7:0]  data;
    reg [31:0] status;			//actual status reg that you read from
    reg [31:0] control;
	
    reg [31:0] clear_status; 	//use this to write to status reg to clear
    
    // register map 
    // ofs  fn
    //   0  address_reg (r/w)
    //   4  data (r/w)
    //   8  status (r/w1c)
    //  12  control (r/w)
    
    // register numbers
    parameter ADDRESS_REG           = 2'b00;
    parameter DATA_REG              = 2'b01;
    parameter STATUS_REG            = 2'b10;
    parameter CONTROL_REG   	    = 2'b11;

    //edge detect for write & read
    wire write_ed;
    edge_detect wr(clk, write, write_ed);
    wire read_ed;
    edge_detect rd(clk, sm_rd_request, read_ed);
    wire write_RX;
    edge_detect RXwr(clk, sm_wr_requestRX, write_RX);
    wire read_RX;
    edge_detect RXrd(clk, read, read_RX);

    reg sm_rd_request; 
    reg sm_wr_requestRX;
    wire [7:0] sm_data;

    //tx fifo instantiation
    wire [7:0] wr_data_tx = data;
    wire [7:0] rd_data_tx;
    wire [5:0] wr_index_tx;
    wire [5:0] rd_index_tx;
    wire empty_tx, full_tx, overflow_tx;
    wire clear_overflow_tx = clear_status [3];

    fifo tx_fifo(
    .clk(clk), 
    .reset(reset), 
    .clear_overflow_request(clear_overflow_tx),
    .wr_data(wr_data_tx), 
    .wr_request(write_ed & (address == DATA_REG)), 
    .rd_request(read_ed),
    .rd_data(sm_data), //?
    .empty(empty_tx),
    .full(full_tx),
    .overflow(overflow_tx),
    .wr_index(wr_index_tx),
    .rd_index(rd_index_tx)); 			

    //rx fifo instantiation
    reg [7:0] wr_data_rx; 
    wire [7:0] rd_data_rx;  
    wire [5:0] wr_index_rx;
    wire [5:0] rd_index_rx; 
    wire empty_rx, full_rx, overflow_rx;
    wire clear_overflow_rx = clear_status [3];

    fifo rx_fifo(
    .clk(clk), 
    .reset(reset), 
    .clear_overflow_request(clear_overflow_rx),
    .wr_data(wr_data_rx), 
    .wr_request(write_RX), 
    .rd_request(0), //read_RX & (address == DATA_REG)),
    .rd_data(rd_data_rx),
    .empty(empty_rx),
    .full(full_rx),
    .overflow(overflow_rx),
    .wr_index(wr_index_rx),
    .rd_index(rd_index_rx)); 	

    always @ (posedge clk)
    begin
	status [0] <= overflow_rx; 
	status [1] <= full_rx; 
	status [2] <= empty_rx;
	status [3] <= overflow_tx; 
	status [4] <= full_tx;
	status [5] <= empty_tx; 
	status [13:8] <= wr_index_tx;		
	status [19:14] <= rd_index_tx; 
	status [31:20] <= 12'b000000000000;  
    end
        
    // read register
    always @ (*)
    begin
        if (read && chipselect)
            case (address)
                ADDRESS_REG: 
                    readdata = address_reg;
                DATA_REG:
		    readdata = rd_data_rx;
                STATUS_REG: 
                    readdata = status;
                CONTROL_REG: 
                    readdata = control;
            endcase
        else
            readdata = 32'b0;
    end        

    // write register
    always @ (posedge clk)
    begin
	if (busy)
	    control [17] <= 1'b0; 
        if (reset)
        begin
            address_reg [6:0] <= 7'b0;
            data [7:0] <= 8'b0;
            clear_status [31:0] <= 32'b0;
            control [31:0] <= 32'b0;
        end
        else
        begin
            if (write && chipselect)
            begin
                case (address)
                    ADDRESS_REG: 
                        address_reg <= writedata;
                    DATA_REG: 
		        data <= writedata;
                    STATUS_REG: 
                        clear_status <= writedata;
                    CONTROL_REG: 
                        control <= writedata;
                endcase
            end
	    else
	        clear_status <= 32'b0; 
        end 
    end
	
    //200 kHz internal reference for state change
    reg [7:0] count; 
    always @ (posedge(clk))
    begin
	direction_reg <= control [0];
			
	if (count == 250) //50,000,000 / 200,000
	begin
		state <= next_state;
		count <= 0;
			
		if (state == START2_STATE)
			n <= 6; 
				
		if (state == ADDRESS_WRITE2_STATE)
			n <= n - 1;
				
		if (state == ACK_ADDRESS2_STATE)
			r <= 7;
				
                if (state == REG_WRITE2_STATE)
                        r <= r - 1;
				
                if (state == ACK_REG2_STATE)
                        d <= 7; 
				
                if (state == DATA_READ2_STATE)
                        wr_data_rx[d2] <= i2c_sda;

                if (state == DATA_WRITE2_STATE)
                        d <= d - 1;

                if(state == DATA_READ1_STATE)
                        d2 <= d2 - 1;
	end
	else
		count <= count + 1; 
	end
	
	parameter IDLE_STATE            = 0;
	parameter START1_STATE        	= 1;
	parameter START2_STATE         	= 2;
	parameter ADDRESS_WRITE1_STATE 	= 3;
	parameter ADDRESS_WRITE2_STATE 	= 4;
	parameter RW1_STATE		= 5; 
	parameter RW2_STATE		= 6; 
	parameter ACK_ADDRESS1_STATE	= 7;
	parameter ACK_ADDRESS2_STATE	= 8;
	parameter REG_WRITE1_STATE      = 9;
	parameter REG_WRITE2_STATE 	= 10; 
	parameter ACK_REG1_STATE	= 11;
	parameter ACK_REG2_STATE	= 12;
	parameter DATA_WRITE1_STATE	= 13; 
	parameter DATA_WRITE2_STATE	= 14; 
	parameter DATA_READ1_STATE 	= 15; 
	parameter DATA_READ2_STATE 	= 16; 
	parameter ACK_DATA1_STATE	= 17;
	parameter ACK_DATA2_STATE	= 18;	
	parameter PRESTOP_STATE         = 19;
	parameter STOP1_STATE           = 20;
	parameter STOP2_STATE		= 21;
	parameter REPEATED_START1_STATE	= 22;
	parameter REPEATED_START2_STATE	= 23;
	
	reg [4:0] state;
	reg [4:0] next_state;
	reg busy;
	reg [7:0] bytecount;
	reg [3:0] n, r, d;
	reg [3:0] d2 = 7;
	wire repeated_start;
	assign repeated_start = control [16];
	reg repeatbit = 1'b0;
	reg direction_reg;
	wire reg_used;
	assign reg_used = control [7]; 
	wire [7:0] register_addr; 
	assign register_addr = control [15:8];
	wire start_bit;
	assign start_bit = control [17]; 
	
	//high-level state machine
	always @ (*)
	begin
		next_state = state;
		case (state)
			IDLE_STATE: 
			begin
				if (start_bit & bytecount != 0)
				begin 
					next_state = START1_STATE; 
					repeatbit = 1'b0;
				end
				else 
				begin
					busy = 0; 
					repeatbit = 1'b0;
					next_state = IDLE_STATE;
				end
			end
			
			START1_STATE: 
			begin 
				next_state = START2_STATE; 
			end
			
			START2_STATE:
			begin  
				next_state = ADDRESS_WRITE1_STATE;
			end
			
 			ADDRESS_WRITE1_STATE: 
			begin
				busy = 1; 
				next_state = ADDRESS_WRITE2_STATE; 
			end
			
			ADDRESS_WRITE2_STATE: 
			begin
				if (n == 0)
					next_state = RW1_STATE; 
				else 
					next_state = ADDRESS_WRITE1_STATE; 	
			end

			RW1_STATE:
			begin 	
				 next_state = RW2_STATE; 
			end
			
			RW2_STATE:
			begin 	
				 next_state = ACK_ADDRESS1_STATE; 
			end
			
			ACK_ADDRESS1_STATE:
			begin 
				next_state = ACK_ADDRESS2_STATE;
			end
			
			ACK_ADDRESS2_STATE:
			begin
				if(repeatbit == 1'b1)
				begin
					next_state = DATA_READ1_STATE;
					end
				else if(reg_used)
					next_state = REG_WRITE1_STATE;
			end
			
			REG_WRITE1_STATE:
			begin 
				next_state = REG_WRITE2_STATE; 
			end
			
			REG_WRITE2_STATE:
			begin 
				if (r == 0)
				begin
					sm_rd_request = 1;
					next_state = ACK_REG1_STATE; 
				end
				else 
					next_state = REG_WRITE1_STATE;				
			end
			
			ACK_REG1_STATE:
			begin
				next_state = ACK_REG2_STATE;
				sm_rd_request = 0; 
			end
			
			ACK_REG2_STATE:
			begin
				if (control [0] == 1'b0)
					next_state = DATA_WRITE1_STATE;
				else
				begin
					next_state = REPEATED_START1_STATE;
					repeatbit = 1'b1;
				end
			end
			
			DATA_WRITE1_STATE:
			begin
				next_state = DATA_WRITE2_STATE;
			end	 
			
			DATA_WRITE2_STATE:
			begin
				if (d == 0)
					next_state = ACK_DATA1_STATE; 
				else 
					next_state = DATA_WRITE1_STATE; 	
			end	 
			
			DATA_READ1_STATE:
			begin 
				next_state = DATA_READ2_STATE; 
			end
			
			DATA_READ2_STATE: //write to RX FIFO
			begin 
				if (d2 == 0)
				begin
					next_state = ACK_DATA1_STATE; 
					sm_wr_requestRX = 1;
				end
				else 
					next_state = DATA_READ1_STATE; 	
			end
			
			ACK_DATA1_STATE:
			begin
				sm_wr_requestRX = 0;
				next_state = ACK_DATA2_STATE;
			end
			
			ACK_DATA2_STATE:
			begin
				if (control [0] == 1'b0)
					  next_state = PRESTOP_STATE; 	
				if (control [0] == 1'b1)
					  next_state = PRESTOP_STATE; 
			end
			
			PRESTOP_STATE:
			begin
				next_state = STOP1_STATE;
			end

			STOP1_STATE:
			begin
				next_state = STOP2_STATE;
			end
			
			STOP2_STATE:
			begin
				next_state = IDLE_STATE; 
			end
			
			REPEATED_START1_STATE:
			begin 
				next_state = REPEATED_START2_STATE;
			end
			
			REPEATED_START2_STATE:
			begin
				next_state = START1_STATE;
			end
		endcase
	end
	
	//low-level state machine
	always @ (*)
	begin
		i2c_sda = 1'bz; 
		i2c_scl = 1'bz;
		
		case (state)
			IDLE_STATE: 
			begin
				i2c_sda = 1'bz; 
				i2c_scl = 1'bz;
			end
			
			START1_STATE: 
			begin 
				i2c_sda = 1'b0;
				i2c_scl = 1'bz; 
			end
			
			START2_STATE: 
			begin
				i2c_scl = 1'b0;
				i2c_sda = 1'b0;		
			end
			
			ADDRESS_WRITE1_STATE: 
			begin
				i2c_scl = 1'b0;
				i2c_sda = address_reg [n] ? 1'bz : 1'b0; 	
			end
			
			ADDRESS_WRITE2_STATE: 
			begin
				i2c_scl = 1'bz;
				i2c_sda = address_reg [n] ? 1'bz : 1'b0; 
			end
			
			RW1_STATE:
			begin 
				i2c_scl = 1'b0;
				if(repeatbit == 1'b1)
					i2c_sda = 1'bz;
				else
					i2c_sda = 1'b0; //write
			end
			
			RW2_STATE:
			begin 
				i2c_scl = 1'bz;
				if(repeatbit == 1'b1)
					i2c_sda = 1'bz;
				else
					i2c_sda = 1'b0; //write
			end
			
			ACK_ADDRESS1_STATE:
			begin 
				i2c_sda = 1'bz;
				i2c_scl = 1'b0;
			end
			
			ACK_ADDRESS2_STATE:
			begin 
				i2c_scl = 1'bz;
				i2c_sda = 1'bz;
			end
			
			REG_WRITE1_STATE:
			begin 
				i2c_scl = 1'b0;
				i2c_sda = register_addr [r] ? 1'bz : 1'b0 ;				
			end 
			
			REG_WRITE2_STATE:
			begin 
				i2c_scl = 1'bz;
				i2c_sda = register_addr [r] ? 1'bz : 1'b0; 
			end 
			
			ACK_REG1_STATE: 
			begin
				i2c_sda = 1'bz;
				i2c_scl = 1'b0;			
			end
			
			ACK_REG2_STATE: 
			begin
				i2c_sda = 1'bz;
				i2c_scl = 1'bz;				
			end
			
			DATA_WRITE1_STATE:
			begin
				i2c_scl = 1'b0;
				i2c_sda = sm_data [d] ? 1'bz : 1'b0 ; 				
			end	
			
			DATA_WRITE2_STATE:
			begin
				i2c_scl = 1'bz;
				i2c_sda = sm_data [d] ? 1'bz : 1'b0; 
			end	
					
			DATA_READ1_STATE:
			begin 
				i2c_scl = 1'b0; 
				i2c_sda = 1'bz;
			end
			
			DATA_READ2_STATE: //write to RX FIFO
			begin
				i2c_scl = 1'bz;
				i2c_sda = 1'bz;
			end
			
			ACK_DATA1_STATE: 
			begin
				i2c_sda = 1'bz; 
				i2c_scl = 1'b0;
			end
			
			ACK_DATA2_STATE: 
			begin
				i2c_sda = 1'bz;
				i2c_scl = 1'bz;	
			end
			
			PRESTOP_STATE:
			begin
				i2c_sda = 1'b0; 
				i2c_scl = 1'b0;
			end

			STOP1_STATE:
			begin
				i2c_scl = 1'bz;
				i2c_sda = 1'b0;
			end
			
			STOP2_STATE:
			begin
				i2c_sda = 1'bz;
				i2c_scl = 1'bz;
			end
			
			REPEATED_START1_STATE:
			begin 
				i2c_scl = 1'b0;
				i2c_sda = 1'bz;
			end
			
			REPEATED_START2_STATE:
			begin
				i2c_scl = 1'bz;
				i2c_sda = 1'bz;
			end
		endcase
	end
endmodule
