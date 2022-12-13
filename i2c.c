// I2C IP project, i2c.c
// I2C Shell Command (write and read internal registers)
// Ulysses Chaparro 1001718774

// Jason Losh origin code

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// IRQ:
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include "i2c_ip.h"         // GPIO IP library

int main(void)
{
	i2cOpen();
	uint32_t value;
	
	//printf("value to write to data reg: "); //10101010 (170 in decimal)
	//scanf("%u", &value);
	printf("\nset data value: "); 
	scanf("%u", &value); 
	setDataValue(value); 
		
	//printf("value to write to address reg: "); //address 0100000 (0x20) (32 in decimal)
	//scanf("%u", &value);
	setAddressValue(32); 

	//printf("value to write to control reg: "); //00000000000000100000100110000000 ->write, reg_used = 1, reg 0x09, start (133504 in decimal)
						     //00000000000000100000000010000000 ->write, reg_used = 1, reg 0x00, start (131200 in decimal)
	//scanf("%u", &value);
	printf("\nset control value: "); 
	scanf("%u", &value); 
	setControlValue(value); 
	
	value = getControlValue(); 
	printf("\nvalue read from control reg: %u\n", value); 
	
	value = getDataValue(); 
	printf("\nvalue read from control reg: %u\n", value);
}

