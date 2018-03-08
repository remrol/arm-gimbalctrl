#include "HMC5883L_1.h"
#include "i2c_master.h"
#include "time.h"
#include <string.h>

StateHMC5883L g_stateHMC5883L;

#define HMC5883L_WRITE 0x3C // write address
#define HMC5883L_READ 0x3D // read address

void HMC5883L_init(void)
{
	i2c_start(HMC5883L_WRITE);
	i2c_write(0x00); // set pointer to CRA
	i2c_write(0b01110100); // CRA7=1,MA1=1,MA0=1: average 8 samples, DO2=1,DO1=0,DO0=1: 30 samples/sec, MS1=0,MS0=0, normal operation
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x01); // set pointer to CRB
	i2c_write(0b00000001); // Gain +/- 1.3 Ga
	//	i2c_stop();
	//	i2c_start(HMC5883L_WRITE);
	//	i2c_write(0x02); // set pointer to measurement mode
	i2c_write(0x00); // continuous measurement
	i2c_stop();

	memset(&g_stateHMC5883L, 0, sizeof(g_stateHMC5883L));
}

void HMC5883L_read(void)
{
	i2c_start(HMC5883L_WRITE);
	i2c_write(0x03); //set pointer to X-axis MSB
	i2c_stop();
	
	g_stateHMC5883L.timeStamp = millis();

	i2c_start(HMC5883L_READ);
	
	g_stateHMC5883L.magx = i2c_read_ack() << 8;
	g_stateHMC5883L.magx |= i2c_read_ack();
	g_stateHMC5883L.magy = i2c_read_ack() << 8;
	g_stateHMC5883L.magy |= i2c_read_ack();
	g_stateHMC5883L.magz = i2c_read_ack() << 8;
	g_stateHMC5883L.magz |= i2c_read_nack();

	i2c_stop();
}

