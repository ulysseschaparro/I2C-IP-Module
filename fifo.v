//fifo for i2c ip

module fifo(					 //W = 8 (data width) //N = 63 (depth) //M = 6 (2^6 = 64 = N)
	input clk, 							
	input reset, 						
	input [7:0] wr_data,			 		
	input wr_request, 					
	output [7:0] rd_data, 
	input rd_request, 					
	output empty, 
	output full, 
	output reg overflow, 
	input clear_overflow_request, 	
	output reg [5:0] wr_index,			
	output reg [5:0] rd_index);		
	
	reg [7:0] fifo_buffer [63:0];    //2D array fifo buffer 
	parameter N = 64;
	
	assign full = ((wr_index + 1) % N == rd_index);
	assign empty = (wr_index == rd_index);
	assign rd_data = fifo_buffer[rd_index];
	
	always @ (posedge(clk))
	begin
		if(reset)
		begin
			wr_index <= 0; 
			rd_index <= 0;
			overflow <= 1'b0;
		end
		
		if(clear_overflow_request)
			overflow <= 1'b0;
		
		if(wr_request)
		begin 
			if(!full & !overflow)	//if not full & no overflow
			begin
				fifo_buffer[wr_index] <= wr_data;	
				wr_index <= (wr_index + 1) % N;
			end
			
			else	//fifo is full or overflow is set
				overflow <= 1'b1; 
		end
		
		if(rd_request)
		begin 
			if(!empty)	//if not empty
			begin
				rd_index <= (rd_index + 1) % N; 
			end
		end
	end
endmodule

module edge_detect(
	input clk, 
	input in, 
	output pulse); 

	reg prev;
	reg pre_out; 
	wire detected = !prev & in; 
	
	assign pulse = pre_out; 
	always @ (posedge(clk))
	begin
	    prev <= in; 
		pre_out <= detected;
	end	
endmodule
