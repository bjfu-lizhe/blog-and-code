#include "driver.h"
/***************************************************/
#define ICM20948_ADDR 0x68
#define AK09916_ADDR 0x0c
/***************************************************/
#define INE_RW(status, reg, data) \
			((status == 'r') ? i2c_read_byte_addr(ICM20948_ADDR, reg) : \
			i2c_write_byte_addr(ICM20948_ADDR, reg, data))
#define EXT_RW(status, reg, data) \
			((status == 'r') ? i2c_read_byte_addr(AK09916_ADDR, reg) : \
			i2c_write_byte_addr(AK09916_ADDR, reg, data))
#define null 0
/***************************************************/
#define USE_BANK0 0x00
#define USE_BANK1 0x10
#define USE_BANK2 0x20
#define USE_BANK3 0x30
#define SWITCH_BANK(x) {i2c_write_byte_addr(ICM20948_ADDR, 0x7f, x);}//choose bank 0-3, reg_bank_sel 0x7f
/**********************bank0**********************/
#define WHO_AM_I 0x00 //id
#define USER_CTRL 0x03
#define PWR_MGMT 0x06 //Range : 0x06 - 0x07
#define INT_PIN_CFG 0x0f
#define INT_ENABLE 0x10
#define AGT_BASE 0x2d //Range : 0X2d-0X3a , agt
#define FIFO_EN 0x66//Range : 0x66-0x67
/**********************bank2**********************/
#define GYRO_SMPLRT_DIV 0x00
#define GYRO_CONFIG 0x01//Range : 0x01-0x02
#define ACCEL_SMPLRT_DIV 0x10//Range : 0x10-0x11
#define ACCEL_CONFIG 0x20
#define TEMP_CONFIG 0x83
/**********************bank2**********************/
#define MAG_BASE 0x11//Range : 0x11 - 0x16
#define CNTL2 0x31
#define CNTL3 0x32
/***************************************************/
#define DELAY delay_us(1)
/*************************iic************************/
#define SDA_ADDR (*((unsigned int *)0x40010c03))
#define SDA_IN()  {SDA_ADDR &= 0x0f; SDA_ADDR |= 0x80;}
#define SDA_OUT() {SDA_ADDR &= 0x0f; SDA_ADDR |= 0x30;}
#define I2C_SCL   PBout(6)
#define I2C_SDA   PBout(7)
#define READ_SDA  PBin(7)


static void i2c_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_Initure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Initure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB, &GPIO_Initure);
	
	I2C_SDA=1;
  I2C_SCL=1; 
}
static void i2c_start(void)
{
	SDA_OUT();
	I2C_SDA=1;	  	  
	I2C_SCL=1;
	delay_us(4);
 	I2C_SDA=0;
	delay_us(4);
	I2C_SCL=0;
}
static void i2c_stop(void)
{
	SDA_OUT();
	I2C_SCL=0;
	I2C_SDA=0;
 	delay_us(4);
	I2C_SCL=1; 
	delay_us(4);			
	I2C_SDA=1;   	
}
static u8 i2c_wait_ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();
	I2C_SDA=1;delay_us(1);	   
	I2C_SCL=1;delay_us(1);	 
	while(READ_SDA){
		ucErrTime++;
		if(ucErrTime>250){
			i2c_stop();
			return 1;
		}
	}
	I2C_SCL=0;
	return 0;  
} 
static void i2c_ack(void)
{
	I2C_SCL=0;
	SDA_OUT();
	I2C_SDA=0;
	delay_us(2);
	I2C_SCL=1;
	delay_us(2);
	I2C_SCL=0;
}	    
static void i2c_nack(void)
{
	I2C_SCL=0;
	SDA_OUT();
	I2C_SDA=1;
	delay_us(2);
	I2C_SCL=1;
	delay_us(2);
	I2C_SCL=0;
}					 				     		  
static void i2c_send_byte(u8 txd)
{                        
   u8 t;   
	 SDA_OUT(); 	    
   I2C_SCL=0;
   for(t=0;t<8;t++){              
     I2C_SDA=(txd&0x80)>>7;
     txd<<=1; 	  
		 delay_us(2);
		 I2C_SCL=1;
		 delay_us(2); 
		 I2C_SCL=0;	
		 delay_us(2);
   }	 
} 	    
static u8 i2c_read_byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();
  for(i=0;i<8;i++ ){
     I2C_SCL=0; 
     delay_us(2);
		 I2C_SCL=1;
     receive<<=1;
     if(READ_SDA)receive++;   
		 delay_us(1); 
  }					 
  if (!ack)i2c_nack();
  else{i2c_ack();}
  return receive;
}
static u8 i2c_write_byte_addr(u8 addr,u8 reg,u8 data) 				 
{ 
  i2c_start(); 
	i2c_send_byte((addr<<1)|0);
	if(i2c_wait_ack()){
		i2c_stop();		 
		return 1;		
	}
  i2c_send_byte(reg);
  i2c_wait_ack();
	i2c_send_byte(data);
	if(i2c_wait_ack()){
		i2c_stop();	 
		return 1;		 
	}		 
    	i2c_stop();	 
	return 0;
}
static u8 i2c_read_byte_addr(u8 addr,u8 reg)
{
	u8 res;
  i2c_start(); 
	i2c_send_byte((addr<<1)|0);
	i2c_wait_ack();	
  i2c_send_byte(reg);
  i2c_wait_ack();
  i2c_start();
	i2c_send_byte((addr<<1)|1);
  i2c_wait_ack();
	res=i2c_read_byte(0);
  i2c_stop();
	return res;		
}
unsigned char icm20948_init(void)
{	
	/*Debug Mode*/
	u8 who_am_i;
	
	i2c_init();
	
	SWITCH_BANK(USE_BANK0); DELAY;
	
	/*Debug Mode*/
	who_am_i = INE_RW('r', WHO_AM_I, null); DELAY;
	
	/* ###Set power management and basic function, bank0###*/
	
	/*Reset the whole device*/
	INE_RW('w', PWR_MGMT, 0x80); DELAY;
	/*Enable the I2C Master I/F module*/
	INE_RW('w', USER_CTRL, 0x20); DELAY;
	/*Auto selects the best available clock source*/
	INE_RW('w', PWR_MGMT, 0x01); DELAY;
	/*Restart gyroscope and accelerometer*/
	INE_RW('w', PWR_MGMT + 1, 0xff); DELAY;
	INE_RW('w', PWR_MGMT + 1, 0x00); DELAY;
	
	SWITCH_BANK(USE_BANK2); DELAY;
	
	/* ###Set gyroscope and accelerometer, bank2### */
	
	/*Range : +-2000dps, low pass filter 17.8hz*/
	INE_RW('w', GYRO_CONFIG, 0x2f); DELAY;
	/*Gyroscope sample rate : 100hz*/
	INE_RW('w', GYRO_SMPLRT_DIV, 0x0a); DELAY;
	/*Range : +-2g, low pass filter 17hz*/
	INE_RW('w', ACCEL_CONFIG, 0x29); DELAY;
	/*accelerometer sample rate : 100hz*/
	INE_RW('w', ACCEL_SMPLRT_DIV, 0x00); DELAY;
	INE_RW('w', ACCEL_SMPLRT_DIV + 1, 0x0a); DELAY;
	
	/* ###Set temperature sensor, bank2### */
	
	/*Temperature sensor set*/
	INE_RW('w', TEMP_CONFIG, 0x05); DELAY;
	
	SWITCH_BANK(USE_BANK0); DELAY;

	/* ###Set bypass mode i2c to support magn### */

	/*INT Pin / Bypass Enable Configuration*/
	INE_RW('w', INT_PIN_CFG, 0x02); DELAY;
	INE_RW('w', USER_CTRL, 0x00); DELAY;
	INE_RW('w', FIFO_EN, 0x00); DELAY;
	INE_RW('w', FIFO_EN + 1, 0x00); DELAY;
	INE_RW('w', INT_ENABLE, 0x00); DELAY;
	
	/*###Initialize magnetometer###*/

	EXT_RW('w', CNTL2, 0x01);
	
	SWITCH_BANK(USE_BANK0); DELAY;
	
	return who_am_i;
}

unsigned char icm20948_get(short *buf, const int length)
{	
	if((unsigned char)length != 10)return 0;
	
	/*get acce*/
	buf[0] = INE_RW('r', AGT_BASE + 0, null) << 8 | INE_RW('r', AGT_BASE + 1, null);//get ax
	buf[1] = INE_RW('r', AGT_BASE + 2, null) << 8 | INE_RW('r', AGT_BASE + 3, null);//get ay
	buf[2] = INE_RW('r', AGT_BASE + 4, null) << 8 | INE_RW('r', AGT_BASE + 5, null);//get az
	
	/*get gyro*/
	buf[3] = INE_RW('r', AGT_BASE + 6, null) << 8 | INE_RW('r', AGT_BASE + 7, null);//get gx
	buf[4] = INE_RW('r', AGT_BASE + 8, null) << 8 | INE_RW('r', AGT_BASE + 9, null);//get gy
	buf[5] = INE_RW('r', AGT_BASE + 10, null) << 8 | INE_RW('r', AGT_BASE + 11, null);//get gz
	
	/*get temperature*/
	buf[9] = INE_RW('r', AGT_BASE + 12, null) << 8 | INE_RW('r', AGT_BASE + 13, null);//get temp
	
	/*get magn*/
	buf[6] = EXT_RW('r', MAG_BASE + 1, null) << 8 | EXT_RW('r', MAG_BASE + 0, null);//get mx
	buf[7] = EXT_RW('r', MAG_BASE + 3, null) << 8 | EXT_RW('r', MAG_BASE + 2, null);//get my
	buf[8] = EXT_RW('r', MAG_BASE + 5, null) << 8 | EXT_RW('r', MAG_BASE + 4, null);//get mz

	EXT_RW('w', CNTL3, 0x01);
	EXT_RW('w', CNTL2, 0x01);
	return 1;
}





