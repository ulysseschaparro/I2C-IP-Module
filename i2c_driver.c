// I2C Driver
//Ulysses Chaparro 1001718774

// Jason Losh origin code

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board

//-----------------------------------------------------------------------------

#include <linux/kernel.h>     // kstrtouint
#include <linux/module.h>     // MODULE_ macros
#include <linux/init.h>       // __init
#include <linux/kobject.h>    // kobject, kobject_atribute,
                              // kobject_create_and_add, kobject_put
#include <asm/io.h>           // iowrite, ioread, ioremap_nocache (platform specific)
#include "address_map.h"      // overall memory map
#include "i2c_regs.h"          // register offsets in QE IP

//-----------------------------------------------------------------------------
// Kernel module information
//-----------------------------------------------------------------------------

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Angel & Uly");
MODULE_DESCRIPTION("I2C IP Driver");

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

static unsigned int *base = NULL; 

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// Kernel Objects
//-----------------------------------------------------------------------------

// MODE
static int mode = 0;
module_param(mode, int, S_IRUGO);
MODULE_PARM_DESC(mode, "Mode");

static ssize_t modeStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &mode);
    if (result == 0)
        setMode(mode);
    return count;
}

static ssize_t modeShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    mode = getMode();
    return sprintf(buffer, "%d\n", mode);
}

static struct kobj_attribute modeAttr = __ATTR(mode, 0664, modeShow, modeStore);

// BYTECOUNT
static int bytecount = 0;
module_param(bytecount, int, S_IRUGO);
MODULE_PARM_DESC(bytecount, "Bytecount");

static ssize_t bytecountStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &bytecount);
    if (result == 0)
        setBytecount(bytecount);
    return count;
}

static ssize_t bytecountShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    bytecount = getBytecount();
    return sprintf(buffer, "%d\n", bytecount);
}

static struct kobj_attribute bytecountAttr = __ATTR(bytecount, 0664, bytecountShow, bytecountStore);

// REGISTER
static int registervalue = 0;
module_param(registervalue, int, S_IRUGO);
MODULE_PARM_DESC(registervalue, "Register value");

static ssize_t registervalueStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &registervalue);
    if (result == 0)
        setRegisterValue(registervalue);
    return count;
}

static ssize_t registervalueShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    registervalue = getRegisterValue();
    return sprintf(buffer, "%d\n", registervalue);
}

static struct kobj_attribute registervalueAttr = __ATTR(registervalue, 0664, registervalueShow, registervalueStore);

// USE REPEATED START
static int use_repeated_start = 0;
module_param(use_repeated_start, int, S_IRUGO);
MODULE_PARM_DESC(use_repeated_start, "Use repeated start value");

static ssize_t use_repeated_startStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &use_repeated_start);
    if (result == 0)
        setUseRepeatedStart(use_repeated_start);
    return count;
}

static ssize_t use_repeated_startShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    use_repeated_start = getUseRepeatedStart();
    return sprintf(buffer, "%d\n", use_repeated_start);
}

static struct kobj_attribute use_repeated_startAttr = __ATTR(use_repeated_start, 0664, use_repeated_startShow, use_repeated_startStore);

// START
static int start = 0;
module_param(start, int, S_IRUGO);
MODULE_PARM_DESC(start, "Start value");

static ssize_t startStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &start);
    if (result == 0)
        setStart(start);
    return count;
}

static ssize_t startShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    start = getStart();
    return sprintf(buffer, "%d\n", start);
}

static struct kobj_attribute startAttr = __ATTR(start, 0664, startShow, startStore);

// ADDRESS
static int address = 0;
module_param(address, int, S_IRUGO);
MODULE_PARM_DESC(address, "Address");

static ssize_t addressStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &address);
    if (result == 0)
        setAddress(address);
    return count;
}

static ssize_t addressShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    address = getAddress();
    return sprintf(buffer, "%d\n", address);
}

static struct kobj_attribute addressAttr = __ATTR(address, 0664, addressShow, addressStore);

// DATA
static int data = 0;
module_param(data, int, S_IRUGO);
MODULE_PARM_DESC(data, "Data");

static ssize_t dataStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &data);
    if (result == 0)
        setData(data);
    return count;
}

static ssize_t dataShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    data = getData();
    return sprintf(buffer, "%d\n", data);
}

static struct kobj_attribute dataAttr = __ATTR(data, 0664, dataShow, dataStore);

// STATUS
static int status = 0;
module_param(status, int, S_IRUGO);
MODULE_PARM_DESC(status, " Status");

static ssize_t statusStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &status);
    if (result == 0)
        setStatus(status);
    return count;
}

static ssize_t statusShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    status = getStatus();
    return sprintf(buffer, "%d\n", status);
}

static struct kobj_attribute statusAttr = __ATTR(status, 0664, statusShow, statusStore);

// CONTROL
static int control = 0;
module_param(control, int, S_IRUGO);
MODULE_PARM_DESC(control, " Control");

static ssize_t controlStore(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int result = kstrtouint(buffer, 0, &control);
    if (result == 0)
        setControl(control);
    return count;
}

static ssize_t controlShow(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    control = getControl();
    return sprintf(buffer, "%d\n", control);
}

static struct kobj_attribute controlAttr = __ATTR(control, 0664, controlShow, controlStore);


// Attributes

static struct attribute *attrs0[] = {&modeAttr.attr, &bytecountAttr.attr, &registervalueAttr.attr, &use_repeated_startAttr.attr, &startAttr.attr, &addressAttr.attr, &dataAttr.attr, &statusAttr.attr, &controlAttr.attr, NULL};

static struct attribute_group group0 =
{
    .name = "i2c0",
    .attrs = attrs0
};


static struct kobject *kobj;

//-----------------------------------------------------------------------------
// Initialization and Exit
//-----------------------------------------------------------------------------

static int __init initialize_module(void)
{
    int result;

    printk(KERN_INFO "I2C driver: starting\n");

    // Create i2c directory under /sys/kernel
    kobj = kobject_create_and_add("i2c", kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "I2C driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    // Create i2c group
    result = sysfs_create_group(kobj, &group0);
    if (result !=0)
        return result;

    // Physical to virtual memory map to access gpio registers
    base = (unsigned int*)ioremap_nocache(LW_BRIDGE_BASE + I2C_BASE_OFFSET,
                                          SPAN_IN_BYTES);
    if (base == NULL)
        return -ENODEV;

    printk(KERN_INFO "I2C driver: initialized\n");

    return 0;
}

static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "I2C driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);

