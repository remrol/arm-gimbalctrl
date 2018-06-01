#include "storm32.h"
extern "C"
{
	#include "uart.h"
};
#include "time.h"
#include "config.h"
#include "debug.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

uint32_t        g_storm32LiveDataTimeStamp;
Storm32LiveData g_storm32LiveData;

//please use LIVEDATA_STATUS_V2, may deprecate in future
#define ST32_LIVEDATA_STATUS_V1			0x0001
#define ST32_LIVEDATA_TIMES				0x0002
#define ST32_LIVEDATA_IMU1GYRO			0x0004
#define ST32_LIVEDATA_IMU1ACC			0x0008
#define ST32_LIVEDATA_IMU1R				0x0010
#define ST32_LIVEDATA_IMU1ANGLES		0x0020
#define ST32_LIVEDATA_PIDCNTRL			0x0040
#define ST32_LIVEDATA_INPUTS			0x0080
#define ST32_LIVEDATA_IMU2ANGLES		0x0100
#define ST32_LIVEDATA_MAGANGLES			0x0200
#define ST32_LIVEDATA_STORM32LINK		0x0400
#define ST32_LIVEDATA_IMUACCCONFIDENCE	0x0800
#define ST32_LIVEDATA_ATTITUDE_RELATIVE	0x1000
#define ST32_LIVEDATA_STATUS_V2			0x2000
#define ST32_LIVEDATA_ENCODERANGLES		0x4000
#define ST32_LIVEDATA_IMUACCABS			0x8000


static inline void crc_accumulate(uint8_t data, uint16_t *crcAccum)
{
	/*Accumulate one byte of data into the CRC*/
    uint8_t tmp;
 
    tmp=data ^ (uint8_t)(*crcAccum &0xff);
    tmp^= (tmp<<4);
    *crcAccum = (*crcAccum>>8) ^ (tmp<<8) ^ (tmp <<3) ^ (tmp>>4);
}

static inline void crc_init(uint16_t* crcAccum)
{
#define X25_INIT_CRC 0xffff
    *crcAccum = X25_INIT_CRC;
}

void storm32Init()
{
	g_storm32LiveDataTimeStamp = 0;
	memset(&g_storm32LiveData, 0, sizeof(Storm32LiveData));
}

uint16_t uart1_recv_until( uint8_t* _buffer, uint8_t _sizeLimit, uint32_t _timeout )
{
	for( uint8_t i = 0; i < _sizeLimit; ++i)
	{
		while( true)
		{
			uint16_t c = uart1_getc();
			
			// No data, check for timeout
			if( c & UART_NO_DATA)
			{
				// Detect timeout
				if( millis() > _timeout )
					return 0x0100 + i;	// Signal timeout error
				
				continue;
			}

			// Error, return with error
			if( c & 0xff00 )
				return 0x0200 + i;	// Signal general error

			_buffer[i] = (uint8_t) c;
			break;
		}
	}
	
	return _sizeLimit;
}

Storm32Status storm32UpdateStatus()
{
	uint32_t timeStamp = millis();
	uint32_t timeout = timeStamp + 100;
	uint8_t stormData[sizeof(Storm32LiveData)];
	uint16_t crc;
	
	crc_init(&crc);
	
	uart1_putc('d');
	
	for( uint8_t i = 0; i < sizeof(Storm32LiveData); ++i )
	{
		uint16_t c;
		
		while( true)
		{
			c = uart1_getc();
			
			// No data, check for timeout
			if( c & UART_NO_DATA)
			{
				// Detect timeout
				if( millis() > timeout )
					return ST32_UPDATE_TIMEOUT;
				
				continue;
			}

			// Error, return with error
			if( c & 0xff00 )
				return ST32_UPDATE_UARTERROR;

			// Accumulate crc for all bytes except of last three (crc and 'o' character)
			if( i < sizeof(Storm32LiveData) - 3 )
				crc_accumulate((uint8_t) c, &crc);
				
			stormData[i] = (uint8_t) c;
			break;
		}
	}
	
	Storm32LiveData* st32Data = (Storm32LiveData*) stormData;
	
	if(st32Data->crc != crc)
		return ST32_UPDATE_CRCERROR;
	
	if( st32Data->endChar != 'o' )
		return ST32_UPDATE_DATAERROR;
	
	g_storm32LiveDataTimeStamp = timeStamp;
	memcpy(&g_storm32LiveData, st32Data, sizeof(Storm32LiveData));
	
	return ST32_UPDATE_OK;	
}

#define LOW_BYTE(x)   ((x) & 0xFF)
#define HIGH_BYTE(x)   (((x)>>8) & 0xFF)

// 4..6 ms
Storm32Status storm32UpdateAngles()
{
	uint8_t cmdBuffer[32];
	uint16_t crc;

	uint32_t timeNow = millis();
	
	// Prepare input buffer.
	crc_init(&crc);
	cmdBuffer[0] = 0xFA;
	cmdBuffer[1] = 0x02;
	crc_accumulate(cmdBuffer[1], &crc);
	cmdBuffer[2] = 0x06;
	crc_accumulate(cmdBuffer[2], &crc);
	cmdBuffer[3] = LOW_BYTE(ST32_LIVEDATA_IMU1ANGLES);
	crc_accumulate(cmdBuffer[3], &crc);
	cmdBuffer[4] = HIGH_BYTE(ST32_LIVEDATA_IMU1ANGLES);
	crc_accumulate(cmdBuffer[4], &crc);
	cmdBuffer[5] = LOW_BYTE(crc);
	cmdBuffer[6] = HIGH_BYTE(crc);
	
	
	// Send data.
	for( uint8_t i = 0; i < 7; ++i )
		uart1_putc(cmdBuffer[i]);
	
	// Receive data back
	uint16_t status = uart1_recv_until( cmdBuffer, 0x0d, timeNow + 100 );
	
	// Check receive error
	if( status & 0x0100 )
		return ST32_UPDATE_TIMEOUT;
	if( status & 0x200)
		return ST32_UPDATE_UARTERROR;
	
	// Check syntax
	// 0xFB LEN 0x06 datamask16 roll16 pitch16 yaw16 crc-low-byte crc-high-byte
	if(cmdBuffer[0] != 0xfb || cmdBuffer[1] != 0x08 )
		return ST32_UPDATE_DATAERROR;

	g_state.storm32YawAngle = -*( ( int16_t* ) ( cmdBuffer + 9 ) );
	g_state.storm32YawTimeStamp = timeNow;

	return ST32_UPDATE_OK;
}

//	g_state.storm32YawTimeStamp = 0;