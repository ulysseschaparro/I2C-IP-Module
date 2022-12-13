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

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool i2cOpen();

void setMode(int32_t direction);
int32_t getMode(void);
void setBytecount(int32_t count);
int32_t getBytecount(void);
void setRegisterValue(int32_t reg_value);
int32_t getRegisterValue(void);
void setUseRepeatedStart(int32_t urs_value);;
void setStart(int32_t start_value);
int32_t getStart(void);
void setAddress(int32_t value);
int32_t getAddress(void);
void setData(int32_t value);
int32_t getData(void);
void setStatus(int32_t value);
int32_t getStatus(void);
void setControl(int32_t value);
int32_t getControl(void);

#endif
