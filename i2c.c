// QE IP Example, qe.c
// QE Shell Command
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

// Hardware configuration:
// QE 0 and 1:
//   GPIO_1[29-28] are used for QE 0 inputs B and A
//   GPIO_1[31-30] are used for QE 1 inputs B and A
// HPS interface:
//   Mapped to offset of 128 in light-weight MM interface aperature

//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include "i2c_ip.h"         // GPIO IP library

int main(int argc, char* argv[])
{
    int pin;
    if (argc == 3)
    {
        gpioOpen();
        pin = atoi(argv[1]);
        if (strcmp(argv[2], "out") == 0)
            selectPinPushPullOutput(pin);
        else if (strcmp(argv[2], "od") == 0)
            selectPinOpenDrainOutput(pin);
        else if (strcmp(argv[2], "in") == 0)
            selectPinDigitalInput(pin);
        else if (strcmp(argv[2], "high") == 0)
            setPinValue(pin, 1);
        else if (strcmp(argv[2], "low") == 0)
            setPinValue(pin, 0);
        else if (strcmp(argv[2], "status") == 0)
            if (getPinValue(pin))
                printf("pin was high (1)\n");
            else
                printf("pin was low (0)\n");
        else
            printf("argument %s not expected\n", argv[2]);
    }
    else if (argc == 4 && strcmp(argv[2], "int") == 0)
    {
        gpioOpen();
        pin = atoi(argv[1]);
        if (strcmp(argv[3], "pos_edge") == 0)
        {
            disablePinInterrupt(pin);
            selectPinInterruptRisingEdge(pin);
            enablePinInterrupt(pin);
        }
        else if (strcmp(argv[3], "neg_edge") == 0)
        {
            disablePinInterrupt(pin);
            selectPinInterruptFallingEdge(pin);
            enablePinInterrupt(pin);
        }
        else if (strcmp(argv[3], "both_edge") == 0)
        {
            disablePinInterrupt(pin);
            selectPinInterruptBothEdges(pin);
            enablePinInterrupt(pin);
        }
        else if (strcmp(argv[3], "high") == 0)
        {
            disablePinInterrupt(pin);
            selectPinInterruptHighLevel(pin);
            enablePinInterrupt(pin);
        }
        else if (strcmp(argv[3], "low") == 0)
        {
            disablePinInterrupt(pin);
            selectPinInterruptLowLevel(pin);
            enablePinInterrupt(pin);
        }
        else if (strcmp(argv[3], "off") == 0)
        {
            disablePinInterrupt(pin);
        }
        else
            printf("argument %s not expected\n", argv[3]);
    }
    else if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
    {
        printf("  usage:\n");
        printf("  qe CHANNEL on            turn on channel\n");
        printf("  qe CHANNEL off           turn off channel\n");
        printf("  \n");
        printf("  qe CHANNEL swap          enable channel input swap\n");
        printf("  qe CHANNEL no_swap       disable channel input swap\n");
        printf("  \n");
        printf("  qe CHANNEL set POSITION  set channel position\n")
        printf("  qe CHANNEL position      get channel position\n")
        printf("  qe CHANNEL velocity      get channel velocity\n")
        printf("  qe timebase PERIOD       set velocity timebase\n")

        printf("  where CHANNEL = [0, 1] and POSITION is [-2^31, 2^31 - 1]\n");
    }
    else
        printf("  command not understood\n");
    return EXIT_SUCCESS;
}

