#include "HMC5883L_1.h"
#include "i2c_master.h"

void HMC5883L_init(void)
{
	#define HMC5883L_WRITE 0x3C // write address
	#define HMC5883L_READ 0x3D // read address
	
	//	char buf[16];
	
	i2c_start(HMC5883L_WRITE);
	i2c_write(0x00); // set pointer to CRA
	i2c_write(0b01110100); //
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x01); // set pointer to CRB
	i2c_write(0b00000000);
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x02); // set pointer to measurement mode
	i2c_write(0x00); // continuous measurement
	i2c_stop();
}

int16_t magx = 0, magy = 0, magz = 0;

void HMC5883L_read(void)
{
	#define HMC5883L_WRITE 0x3C // write address
	#define HMC5883L_READ 0x3D // read address
	
	//	char buf[16];
	

	i2c_start(HMC5883L_WRITE);
	i2c_write(0x03); //set pointer to X-axis MSB
	i2c_stop();
	
	i2c_start(HMC5883L_READ);
	
	magx = i2c_read_ack() << 8;
	magx |= i2c_read_ack();
	magy = i2c_read_ack() << 8;
	magy |= i2c_read_ack();
	magz = i2c_read_ack() << 8;
	magz |= i2c_read_nack();

	i2c_stop();
}

