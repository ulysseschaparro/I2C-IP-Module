// I2C IP Status and Data reg Example
// I2C IP Status and Data reg Library 
// Ulysses Chaparro 1001718774

// Jason Losh origin code

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// GPIO Port:
//   GPIO_1[31-0] is used as a general purpose GPIO port
// HPS interface:
//   Mapped to offset of 0 in light-weight MM interface aperature
//   IRQ80 is used as the interrupt interface to the HPS

//-----------------------------------------------------------------------------

#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close
#include "address_map.h"  // address map
#include "i2c_ip.h"         // i2c
#include "i2c_regs.h"       // registers

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

uint32_t *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool i2cOpen()
{
    // Open /dev/mem
    int file = open("/dev/mem", O_RDWR | O_SYNC);
    bool bOK = (file >= 0);
    if (bOK)
    {
        // Create a map from the physical memory location of
        // /dev/mem at an offset to LW avalon interface
        // with an aperature of SPAN_IN_BYTES bytes
        // to any location in the virtual 32-bit memory space of the process
        base = mmap(NULL, SPAN_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED,
                    file, LW_BRIDGE_BASE + I2C_BASE_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    return bOK;
}

void setMode(int32_t direction)
{
	int32_t value = ioread32(base + (OFS_CONTROL));
	value &= ~1; //clearing bit 0
	value |= direction;
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getMode(void)
{
    int32_t value = ioread32(base + (OFS_CONTROL));
	return value & (1 << 0); 
}

void setBytecount(int32_t count)
{
	int32_t value = ioread32(base + (OFS_CONTROL));
	value &= ~126; //clearing bits 6:1
	value |= (count << 1);
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getBytecount(void)
{
    int32_t value = ioread32(base + (OFS_CONTROL));
	return (value & (((1 << 6) - 1) << 1) / 2); //(((1 << # of bits) - 1) << bit starting at) / 2^(bit starting at)
}

void setRegisterValue(int32_t reg_value)
{
	int32_t value = ioread32(base + (OFS_CONTROL));
	value &= ~65280; //clearing bits 15:8
	value |= (reg_value << 8);
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getRegisterValue(void)
{
    int32_t value = ioread32(base + (OFS_CONTROL));
	return (value & (((1 << 8) - 1) << 8) / 256); //(((1 << # of bits) - 1) << bit starting at) / 2^(bit starting at)
}

void setUseRepeatedStart(int32_t urs_value)
{
	int32_t value = ioread32(base + (OFS_CONTROL));
	value &= ~65536; //clearing bit 16
	value |= urs_value;
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getUseRepeatedStart(void)
{
    int32_t value = ioread32(base + (OFS_CONTROL));
	return value & (1 << 16);
}

void setStart(int32_t start_value)
{
	int32_t value = ioread32(base + (OFS_CONTROL));
	value &= ~131072; //clearing bit 17
	value |= start_value;
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getStart(void)
{
    int32_t value = ioread32(base + (OFS_CONTROL));
	return value & (1 << 17);
}
void setAddress(int32_t value)
{
    iowrite32(value, base + (OFS_ADDRESS));
}

int32_t getAddress(void)
{
    return ioread32(base + (OFS_ADDRESS));
}

void setData(int32_t value)
{
    iowrite32(value, base + (OFS_DATA));
}

int32_t getData(void)
{
    return ioread32(base + (OFS_DATA));
}

void setStatus(int32_t value)
{
    iowrite32(value, base + (OFS_STATUS));
}

int32_t getStatus(void)
{
    return ioread32(base + (OFS_STATUS));
}

void setControl(int32_t value)
{
    iowrite32(value, base + (OFS_CONTROL));
}

int32_t getControl(void)
{
    return ioread32(base + (OFS_CONTROL));
}
