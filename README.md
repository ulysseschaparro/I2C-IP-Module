# CSE4356 System on Chip Design Final Project- Design of a I2C IP Core and Linux Device Drivers

# Overview
This project implements an I2C IP module on a Cyclone V FPGA that is capable of being controlled by the hard processor subsystem (HPS) over an Avalon memory-mapped interface. Additionally, Linux kernel modules are implemented to create a virtual file system (VFS) that controls the operation of the I2C communication protocol. 

The goal is to operate as the bus master via the I2C IP module to communicate with an I2C device, specifically the MCP23008 chip. To do this, there is an Avalon memory-mapped interface in which the I2C IP module interfaces with the HPS. There are 4 internal registers- address, data, status, and control, that are offset relative to the lightweight Avalon interface aperture base address +0x8000. There is an internal 200kHz reference clock divider that essentially drives the state transitions. The I2C_SDA 
and I2C_SCL signals can be set accordingly on the current state in the state machine. As for the state machine, both a high level and a low level state machine are utilized. The high level machine is used solely for transitioning from state to state. The low level machine is used for configuring the I2C_SDA and I2C_SCL lines based on the state. A transmit FIFO (TX FIFO) is written to by writing to the internal data register. A receiver FIFO (RX FIFO) is written to after a data byte arrives on the I2C_SDA line. When the software reads the internal data register, the Avalon read signal is set, which results in data being retrieved from the RX FIFO. The two main courses of action are to transmit bytes to the device and to receive bytes from the device.
