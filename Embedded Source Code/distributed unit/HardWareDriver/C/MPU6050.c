/*    
      ____                      _____                  +---+
     / ___\                     / __ \                 | R |
    / /                        / /_/ /                 +---+
   / /   ________  ____  ___  / ____/___  ____  __   __
  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
 / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
 \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
                                                 / /
                                            ____/ /
                                           /_____/
mpu6050.c file
编写者：小马  (Camel)
作者E-mail：375836945@qq.com
编译环境：MDK-Lite  Version: 4.23
初版时间: 2014-01-28
功能：
1.飞机姿态传感器初始化
2.硬件上的数据中断int脚已连接，软件是轮训机制，大家可以在这方面做修改
------------------------------------
*/
#include "MPU6050.h"
#include "IICx.h"
#include "extern_variable.h"
#include "config.h"



uint8_t buffer[14];
int16_t MPU6050_FIFO[6][11];
int16_t Gx_offset=0,Gy_offset=0,Gz_offset=0;

// uint8_t pgm_read_byte(uint16_t add){
// 	return 0;
// }

/**************************实现函数********************************************
*函数原型:		unsigned char MPU6050_is_DRY(void)
*功　　能:	    检查 MPU6050的中断引脚，测试是否完成转换
返回 1  转换完成
0 数据寄存器还没有更新
*******************************************************************************/
unsigned char MPU6050_is_DRY(u8 Channel){///////////////////////
	return 1;
	switch(Channel)
	{
	case 0:
		 if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)==Bit_SET){
			return 1;
		 }
		 else return 0;
		 //break;
	case 1:
		 if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_3)==Bit_SET){
			return 1;
		 }
		 else return 0;
		 //break;
	case 2:
		 if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_11)==Bit_SET){
			return 1;
		 }
		 else return 0;
	case 3:
		 if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)==Bit_SET){
			return 1;
		 }
		 else return 0;
	case 4:
		 if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)==Bit_SET){
			return 1;
		 }
		 else return 0;
	}
  return 0;
	 
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setClockSource(uint8_tsource)
*功　　能:	    设置  MPU6050 的时钟源
 * CLK_SEL | Clock Source
 * --------+--------------------------------------
 * 0       | Internal oscillator
 * 1       | PLL with X Gyro reference
 * 2       | PLL with Y Gyro reference
 * 3       | PLL with Z Gyro reference
 * 4       | PLL with external 32.768kHz reference
 * 5       | PLL with external 19.2MHz reference
 * 6       | Reserved
 * 7       | Stops the clock and keeps the timing generator in reset
*******************************************************************************/
void MPU6050_setClockSource(uint8_t source, u8 Channel){
    IICwriteBits(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, source, Channel);

}

/** Trigger a full device reset.
 * A small delay of ~50ms may be desirable after triggering a reset.
 * @see MPU6050_RA_PWR_MGMT_1
 * @see MPU6050_PWR1_DEVICE_RESET_BIT
 */
void MPU6050_reset(u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET_BIT, 1, Channel);
}

/** Set full-scale gyroscope range.
 * @param range New full-scale gyroscope range value
 * @see getFullScaleRange()
 * @see MPU6050_GYRO_FS_250
 * @see MPU6050_RA_GYRO_CONFIG
 * @see MPU6050_GCONFIG_FS_SEL_BIT
 * @see MPU6050_GCONFIG_FS_SEL_LENGTH
 */
void MPU6050_setFullScaleGyroRange(uint8_t range,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_GYRO_CONFIG, MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, range, Channel);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setFullScaleAccelRange(uint8_trange)
*功　　能:	    设置  MPU6050 加速度计的最大量程
*******************************************************************************/
void MPU6050_setFullScaleAccelRange(uint8_t range,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_ACCEL_CONFIG, MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, range, Channel);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setSleepEnabled(uint8_tenabled)
*功　　能:	    设置  MPU6050 是否进入睡眠模式
				enabled =1   睡觉
			    enabled =0   工作
*******************************************************************************/
void MPU6050_setSleepEnabled(uint8_t enabled, u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_PWR_MGMT_1, MPU6050_PWR1_SLEEP_BIT, enabled, Channel);
}

/**************************实现函数********************************************
*函数原型:		uint8_tMPU6050_getDeviceID(void)
*功　　能:	    读取  MPU6050 WHO_AM_I 标识	 将返回 0x68
*******************************************************************************/
uint8_t MPU6050_getDeviceID(u8 Channel) {

    IICreadBytes(devAddr, MPU6050_RA_WHO_AM_I, 1, buffer, Channel);
    return buffer[0];
}

/**************************实现函数********************************************
*函数原型:		uint8_tMPU6050_testConnection(void)
*功　　能:	    检测MPU6050 是否已经连接
*******************************************************************************/
uint8_t MPU6050_testConnection(u8 Channel) {
   if(MPU6050_getDeviceID( Channel) == 0x68)  //0b01101000;
   return 1;
   else return 0;
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CMasterModeEnabled(uint8_tenabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CMasterModeEnabled(uint8_t enabled,u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled, Channel);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_setI2CBypassEnabled(uint8_tenabled)
*功　　能:	    设置 MPU6050 是否为AUX I2C线的主机
*******************************************************************************/
void MPU6050_setI2CBypassEnabled(uint8_t enabled,u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_I2C_BYPASS_EN_BIT, enabled, Channel);
}

/**************************实现函数********************************************
*函数原型:		void MPU6050_Check()
*功　　能:	  检测IIC总线上的MPU6050是否存在
*******************************************************************************/
void MPU6050_Check(u8 Channel) 
{ 
  switch(MPU6050_testConnection(Channel))
  {
    case 0:printf("未检测到MPU6050...\r\n");
      break;
    case 1:printf("已检测到MPU6050...\r\n");
      break;
  }
} 
/**************************实现函数********************************************
*函数原型:		void MPU6050_initialize(void)
*功　　能:	    初始化 	MPU6050 以进入可用状态。
*******************************************************************************/
void MPU6050_initialize(u8 Channel) {
	//int16_t temp[6];
//	unsigned char i;
    MPU6050_setClockSource(MPU6050_CLOCK_PLL_XGYRO, Channel); //设置时钟
    MPU6050_setFullScaleGyroRange(MPU6050_GYRO_FS_1000, Channel);//陀螺仪最大量程 +-1000度每秒
    MPU6050_setFullScaleAccelRange(MPU6050_ACCEL_FS_2, Channel);	//加速度度最大量程 +-2G
    MPU6050_setSleepEnabled(0, Channel);          //进入工作状态
	  MPU6050_setI2CMasterModeEnabled(0, Channel);	 //不让MPU6050 控制AUXI2C
	  MPU6050_setI2CBypassEnabled(1, Channel);	     //主控制器的I2C与	MPU6050的AUXI2C	直通。控制器可以直接访问HMC5883L
	

	//配置MPU6050 的中断模式 和中断电平模式
	IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_LEVEL_BIT, 0, Channel);
	IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_OPEN_BIT, 0, Channel);
	IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_LATCH_INT_EN_BIT, 1, Channel);
	IICwriteBit(devAddr, MPU6050_RA_INT_PIN_CFG, MPU6050_INTCFG_INT_RD_CLEAR_BIT, 1, Channel);
	//开数据转换完成中断
  //IICwriteBit(devAddr, MPU6050_RA_INT_ENABLE, MPU6050_INTERRUPT_DATA_RDY_BIT, 1);
  MPU6050_Check( Channel);//打印设备检测信息
}

// BANK_SEL register
void MPU6050_setMemoryBank(uint8_t bank, uint8_t prefetchEnabled, uint8_t userBank,u8 Channel) {
    bank &= 0x1F;
    if (userBank) bank |= 0x20;
    if (prefetchEnabled) bank |= 0x40;
    IICwriteByte(devAddr, MPU6050_RA_BANK_SEL, bank, Channel);
}

// MEM_START_ADDR register
void MPU6050_setMemoryStartAddress(uint8_t address,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_MEM_START_ADDR, address, Channel);
}

// MEM_R_W register
uint8_t MPU6050_readMemoryByte(u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_MEM_R_W, 1 , buffer, Channel);
    return buffer[0];
}

// XG_OFFS_USR* registers
int16_t MPU6050_getXGyroOffset(u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_XG_OFFS_USRH, 2, buffer, Channel);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t MPU6050_getYGyroOffset(u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_YG_OFFS_USRH, 2, buffer, Channel);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
int16_t MPU6050_getZGyroOffset(u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_ZG_OFFS_USRH, 2, buffer, Channel);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}

uint8_t verifyBuffer[MPU6050_DMP_MEMORY_CHUNK_SIZE];
uint8_t progBuffer[MPU6050_DMP_MEMORY_CHUNK_SIZE];
uint8_t MPU6050_writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify, uint8_t useProgMem,u8 Channel) {
    uint8_t chunkSize;
    //uint8_t *verifyBuffer;
    uint8_t *tprogBuffer;
    uint16_t i;
    uint8_t j;
	MPU6050_setMemoryBank(bank,0,0, Channel);
    MPU6050_setMemoryStartAddress(address, Channel);
    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;
        
        // write the chunk of data as specified
        tprogBuffer = (uint8_t*)data + i;
     

        IICwriteBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, tprogBuffer, Channel);

        // verify data if needed
        if (verify) {
            MPU6050_setMemoryBank(bank,0,0, Channel);
            MPU6050_setMemoryStartAddress(address, Channel);
            IICreadBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer, Channel);

			for(j=0;j<chunkSize;j++){
			if(tprogBuffer[j] != verifyBuffer[j]) 
				return 0; // uh oh.
			}
        }

        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_tautomatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
            MPU6050_setMemoryBank(bank,0,0, Channel);
            MPU6050_setMemoryStartAddress(address, Channel);
        }
    }
    return 1;
}

uint8_t MPU6050_writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, uint8_t useProgMem,u8 Channel) {
    uint8_t *progBuffer, success, special;
    uint16_t i;

    // config set data is a long string of blocks with the following structure:
    // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
    uint8_t bank, offset, length;
    for (i = 0; i < dataSize;) {
            bank = data[i++];
            offset = data[i++];
            length = data[i++];

        // write data or perform special action
        if (length > 0) {
            // regular block of data to write
            progBuffer = (uint8_t*)data + i;
           
            success = MPU6050_writeMemoryBlock(progBuffer, length, bank, offset, 1, 0, Channel);
            i += length;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.
         
            special = data[i++];
            if (special == 0x01) {
                // enable DMP-related interrupts
                
                IICwriteByte(devAddr, MPU6050_RA_INT_ENABLE, 0x32, Channel);  // single operation

                success = 1;
            } else {
                // unknown special command
                success = 0;
            }
        }
        
        if (!success) {
            return 0; // uh oh
        }
    }
    return 1;
}

uint8_t MPU6050_writeProgMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify,u8 Channel) {
	return MPU6050_writeMemoryBlock(data, dataSize, bank, address, verify, 0, Channel);
}

uint8_t MPU6050_writeProgDMPConfigurationSet(const uint8_t *data, uint16_t dataSize,u8 Channel) {
    return MPU6050_writeDMPConfigurationSet(data, dataSize, 0, Channel);
}

/** Set full interrupt enabled status.
 * Full register byte for all interrupts, for quick reading. Each bit should be
 * set 0 for disabled, 1 for enabled.
 * @param enabled New interrupt enabled status
 * @see getIntFreefallEnabled()
 * @see MPU6050_RA_INT_ENABLE
 * @see MPU6050_INTERRUPT_FF_BIT
 **/
void MPU6050_setIntEnabled(uint8_t enabled,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_INT_ENABLE, enabled, Channel);
}

/** Set gyroscope sample rate divider.
 * @param rate New sample rate divider
 * @see getRate()
 * @see MPU6050_RA_SMPLRT_DIV
 */
void MPU6050_setRate(uint8_t rate,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_SMPLRT_DIV, rate, Channel);
}

/** Set digital low-pass filter configuration.
 * @param mode New DLFP configuration setting
 * @see getDLPFBandwidth()
 * @see MPU6050_DLPF_BW_256
 * @see MPU6050_RA_CONFIG
 * @see MPU6050_CFG_DLPF_CFG_BIT
 * @see MPU6050_CFG_DLPF_CFG_LENGTH
 */
void MPU6050_setDLPFMode(uint8_t mode,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, mode, Channel);
}

/** Set external FSYNC configuration.
 * @see getExternalFrameSync()
 * @see MPU6050_RA_CONFIG
 * @param sync New FSYNC configuration value
 */
void MPU6050_setExternalFrameSync(uint8_t sync,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, sync, Channel);
}

void MPU6050_setDMPConfig1(uint8_t config,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_DMP_CFG_1, config, Channel);
}

void MPU6050_setDMPConfig2(uint8_t config,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_DMP_CFG_2, config, Channel);
}

void MPU6050_setOTPBankValid(uint8_t enabled,u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, enabled, Channel);
}


void MPU6050_setXGyroOffset(int16_t offset,u8 Channel) {
	buffer[0] = offset>>8;
	buffer[1] = offset&0x00ff;
    IICwriteBytes(devAddr, MPU6050_RA_XG_OFFS_USRH, 2, buffer, Channel);
}
void MPU6050_setYGyroOffset(int16_t offset,u8 Channel) {
	buffer[0] = offset>>8;
	buffer[1] = offset&0x00ff;
    IICwriteBytes(devAddr, MPU6050_RA_YG_OFFS_USRH, 2, buffer, Channel);
}

void MPU6050_setZGyroOffset(int16_t offset,u8 Channel) {
    buffer[0] = offset>>8;
	buffer[1] = offset&0x00ff;
    IICwriteBytes(devAddr, MPU6050_RA_ZG_OFFS_USRH, 2, buffer, Channel);
}

/** Reset the FIFO.
 * This bit resets the FIFO buffer when set to 1 while FIFO_EN equals 0. This
 * bit automatically clears to 0 after the reset has been triggered.
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_FIFO_RESET_BIT
 */
void MPU6050_resetFIFO(u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, 1, Channel);
}

/** Get current FIFO buffer size.
 * This value indicates the number of bytes stored in the FIFO buffer. This
 * number is in turn the number of bytes that can be read from the FIFO buffer
 * and it is directly proportional to the number of samples available given the
 * set of sensor data bound to be stored in the FIFO (register 35 and 36).
 * @return Current FIFO buffer size
 */
uint16_t MPU6050_getFIFOCount(u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_FIFO_COUNTH, 2, buffer, Channel);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

/** Set free-fall event acceleration threshold.
 * @param threshold New motion detection acceleration threshold value (LSB = 2mg)
 * @see getMotionDetectionThreshold()
 * @see MPU6050_RA_MOT_THR
 */
void MPU6050_setMotionDetectionThreshold(uint8_t threshold,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_MOT_THR, threshold, Channel);
}

/** Set zero motion detection event acceleration threshold.
 * @param threshold New zero motion detection acceleration threshold value (LSB = 2mg)
 * @see getZeroMotionDetectionThreshold()
 * @see MPU6050_RA_ZRMOT_THR
 */
void MPU6050_setZeroMotionDetectionThreshold(uint8_t threshold,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_ZRMOT_THR, threshold, Channel);
}

/** Set motion detection event duration threshold.
 * @param duration New motion detection duration threshold value (LSB = 1ms)
 * @see getMotionDetectionDuration()
 * @see MPU6050_RA_MOT_DUR
 */
void MPU6050_setMotionDetectionDuration(uint8_t duration,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_MOT_DUR, duration, Channel);
}

/** Set zero motion detection event duration threshold.
 * @param duration New zero motion detection duration threshold value (LSB = 1ms)
 * @see getZeroMotionDetectionDuration()
 * @see MPU6050_RA_ZRMOT_DUR
 */
void MPU6050_setZeroMotionDetectionDuration(uint8_t duration,u8 Channel) {
    IICwriteByte(devAddr, MPU6050_RA_ZRMOT_DUR, duration, Channel);
}

void MPU6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address,u8 Channel) {
    uint8_t chunkSize;
	uint16_t i;
	MPU6050_setMemoryBank(bank,0,0, Channel);
    MPU6050_setMemoryStartAddress(address, Channel);
    
    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        // read the chunk of data as specified
        IICwriteBytes(devAddr, MPU6050_RA_MEM_R_W, chunkSize, data + i, Channel);
        
        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_tautomatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
            MPU6050_setMemoryBank(bank,0,0, Channel);
            MPU6050_setMemoryStartAddress(address, Channel);
        }
    }
}

void MPU6050_getFIFOBytes(uint8_t *data, uint8_t length,u8 Channel) {
    IICreadBytes(devAddr, MPU6050_RA_FIFO_R_W, length, data, Channel);
}

/** Get full set of interrupt status bits.
 * These bits clear to 0 after the register has been read. Very useful
 * for getting multiple INT statuses, since each single bit read clears
 * all of them because it has to read the whole byte.
 * @return Current interrupt status
 * @see MPU6050_RA_INT_STATUS
 */
uint8_t MPU6050_getIntStatus(u8 Channel) {
    return I2C_ReadOneByte(devAddr, MPU6050_RA_INT_STATUS, Channel);
}

void MPU6050_setDMPEnabled(uint8_t enabled,u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled, Channel);
}

uint8_t MPU6050_getOTPBankValid(u8 Channel) {
	uint8_t  temp = I2C_ReadOneByte(devAddr, MPU6050_RA_XG_OFFS_TC, Channel);
    return temp&(1<<MPU6050_TC_OTP_BNK_VLD_BIT);
}

int8_t MPU6050_getXGyroOffsetTC(u8 Channel) {
	uint8_t  temp = I2C_ReadOneByte(devAddr, MPU6050_RA_XG_OFFS_TC, Channel);
	temp &= 0x3F;
    return temp;
}
void MPU6050_setXGyroOffsetTC(int8_t offset,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset, Channel);
}

// YG_OFFS_TC register
int8_t MPU6050_getYGyroOffsetTC(u8 Channel) {
	uint8_t  temp = I2C_ReadOneByte(devAddr, MPU6050_RA_YG_OFFS_TC, Channel);
	temp &= 0x3F;
    return temp;
}
void MPU6050_setYGyroOffsetTC(int8_t offset,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset, Channel);
}

// ZG_OFFS_TC register
int8_t MPU6050_getZGyroOffsetTC(u8 Channel) {
	uint8_t  temp = I2C_ReadOneByte(devAddr, MPU6050_RA_ZG_OFFS_TC, Channel);
	temp &= 0x3F;
    return temp;
}
void MPU6050_setZGyroOffsetTC(int8_t offset,u8 Channel) {
    IICwriteBits(devAddr, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset, Channel);
}

/** Set the I2C address of the specified slave (0-3).
 * @param num Slave number (0-3)
 * @param address New address for specified slave
 * @see getSlaveAddress()
 * @see MPU6050_RA_I2C_SLV0_ADDR
 */
void MPU6050_setSlaveAddress(uint8_t num, uint8_t address,u8 Channel) {
    if (num > 3) return;
    IICwriteByte(devAddr, MPU6050_RA_I2C_SLV0_ADDR + num*3, address, Channel);
}

/** Reset the I2C Master.
 * This bit resets the I2C Master when set to 1 while I2C_MST_EN equals 0.
 * This bit automatically clears to 0 after the reset has been triggered.
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_I2C_MST_RESET_BIT
 */
void MPU6050_resetI2CMaster(u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT, 1, Channel);
}

/** Set FIFO enabled status.
 * @param enabled New FIFO enabled status
 * @see getFIFOEnabled()
 * @see MPU6050_RA_USER_CTRL
 * @see MPU6050_USERCTRL_FIFO_EN_BIT
 */
void MPU6050_setFIFOEnabled(uint8_t enabled,u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, enabled, Channel);
}

void MPU6050_resetDMP(u8 Channel) {
    IICwriteBit(devAddr, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, 1, Channel);
}




//------------------End of File----------------------------
