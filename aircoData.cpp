//-----------------------------------------------------------------------
// aircoData.cpp
//-----------------------------------------------------------------------
// handle the RS485 Webasto air conditioner monitoring

#include "airco.h"
#include <myIOTLog.h>



#if WITH_WEBASTO

	//---------------------------------------
	// airco uart queue
	//---------------------------------------

	#define PACKET_TIMEOUT		15000
		// should get a packet every second!

	#define PACKET_TYPE_BUS		0
	#define PACKET_TYPE_PCB		1
	#define PACKET_TYPE_PANEL	2

	#define MAX_PACKET_SIZE   32
	#define MAX_QUEUE_PACKETS 10

	typedef struct
	{
		uint8_t data[MAX_PACKET_SIZE];
		int len;
	} packet_t;


	HardwareSerial AIRCO_SERIAL(2);
	QueueHandle_t packetQueue;

	//-------------------------------------
	// packet descriptors
	//-------------------------------------
	// full packet offsets

	#define PCB_MODE_OFFSET			9
	#define PCB_COND_OFFSET			10
	#define PCB_INTAKE_OFFSET		11
	#define PCB_FAN_OFFSET			12

	#define PAN_ON_OFFSET			9
	#define PAN_MODE_OFFSET			10
	#define PAN_SETPOINT_OFFSET		11
	#define PAN_INTAKE_OFFSET		12
	#define PAN_FAN_OFFSET			13

	#define CENTIGRADE_BIAS			40	// bias for intake and other C temperatures

	// from perl: my %STATE_FIELDS = (
	// 	PCB => {
	// 		5 => {name=>'on_off', x=>0, y=>1 },
	// 		7 => {name=>'intake', x=>0, y=>3, bias=>40, },
	// 		8 => {name=>'fan_actual', x=>0, y=>4, },
	// 	},
	// 	PAN => {
	// 		5 => {name=>'on_off', x=>0, y=>1 },
	// 		7 => {name=>"setp",   x=>0, y=>2},
	// 		8 => {name=>'intake', x=>0, y=>3, bias=>40, },
	// 		9 => {name=>'fan',	  x=>0, y=>4, discrete=>$PANEL_FAN_SPEEDS, },
	// 	},
	// );

	//---------------------------------
	// vars
	//---------------------------------

	static int num_packets = 1;
	static int dropped_bytes = 0;

	static uint8_t pcb_mode;
	// static uint8_t m_pcb_cond_temp;
	// static uint8_t m_pcb_intake_temp;
	// static uint8_t m_pcb_fan;

	static uint8_t pan_on;
	// static uint8_t m_pan_mode;
	static uint8_t pan_setpoint;
	static uint8_t pan_intake_temp;
	static uint8_t pan_fan;

#endif


//******************************************************
// clearError() is not only for Webasto!
//******************************************************

void aircoDevice::clearAircoError()
{
	LOGU("clearAircoError()");
	#if WITH_WEBASTO
		airco->setString(ID_AIRCO_ERROR,"");
		dropped_bytes = 0;
		num_packets = 1;
	#endif
	// clear flashing drainpump LED if any
	m_flags_last_run = 0;
}



#if WITH_WEBASTO

	//-------------------------------------------
	// implementation
	//-------------------------------------------

	void aircoDevice::reportAircoError(const char *msg)
	{
		if (_airco_error_string.indexOf(msg) == -1)
		{
			String err = _airco_error_string;
			if (err != "")
				err += "; ";
			err += msg;
			LOGE(err.c_str());
			setString(ID_AIRCO_ERROR,err);
		}
	}



	//-----------------------------------------------------------------------
	// updateUI()
	//-----------------------------------------------------------------------

	static const char *tempToStr(int centigrade, int bias)
	{
		float cent = centigrade - bias;
		float faren = centigradeToFarenheit(cent);
		static char buf[30];
		sprintf(buf,"%.0fC=%.0fF",cent,faren);
		return buf;
	}

	static char *hexByte(uint8_t byte)
	{
		static char buf[12];
		sprintf(buf,"%02x",byte);
		return buf;
	}

	static const char *modeStr(const char *type, uint8_t byte)
	{
		if (byte == 0x3f) return "cool";
		if (byte == 0x7e) return "dehum";
		if (byte == 0xbd) return "fan";
		if (byte == 0xce) return "heat";
		if (byte == 0xdd) return "auto";
		static char buf[64];
		sprintf(buf,"UNKNOWN %s MODE 0x%02x",type,byte);
		airco->reportAircoError(buf);
		return buf;
	}



	void aircoDevice::updateAircoUI()
	{
		static uint32_t last_ui;
		uint32_t now = millis();
		if (now - last_ui > _ui_interval)
		{
			last_ui = now;

			String state;
			if (!num_packets)
				state += "NO PACKETS!<br>";
			if (dropped_bytes)
				state += "dropped_bytes(" + String(dropped_bytes) + ")<br>";

			// optional separate error message if no packets
			// would generally need to be cleared if not connected

			if (!num_packets)
				reportAircoError("NO PACKETS!");

			// skip 0th bogus packet

			if (num_packets > 1)
			{
				// PCB line

				state += "PCB mode(";
				state += modeStr("PCB",pcb_mode);
				state += ") cond(";
				state += tempToStr(m_pcb_cond_temp,CENTIGRADE_BIAS);
				state += ") intake(";
				state += tempToStr(m_pcb_intake_temp,CENTIGRADE_BIAS);
				state += ") fan(";
				state += hexByte(m_pcb_fan);
				state += ")=";

				// PCB fan values
				// high bit 0x80 = 'user' setting; otherwise it's "auto" controlled by the unit
				// nigh_nibble & 0x70 is the fan 'speed':
				// 		0x10 = high
				// 		0x20 = medium
				// 		0x40 = low
				// 		0x00 = off
				// the low nibble is whether the compressor is on
				//		where 0x8=on, 0x0=off, and 0x1,0x2 are currently considered illegal
				// I have not identified a separate value for the water pump on,
				// 		nor figured out how the two separate pressure switches map to
				// 		andy of these fields.

				state += (m_pcb_fan & 0x80) ? "user_" : "auto_";

				uint8_t fan_nibble = (m_pcb_fan & 0x70);
				state +=
					fan_nibble == 0x10 ? "high" :
					fan_nibble == 0x20 ? "med" :
					fan_nibble == 0x40 ? "low" : "off";

				static char tbuf[60];
				uint8_t comp_on   = m_pcb_fan & 0x0f;
				if (comp_on == 0x00)
					strcpy(tbuf," comp_off");
				else if (comp_on == 0x08)
					strcpy(tbuf," comp_on");
				else
				{
					sprintf(tbuf," UNKNOWN FAN_COMP(%d)",comp_on);
					reportAircoError(tbuf);
				}
				state += tbuf;
				state += "<br>";

				// PANEL line

				state += "PANEL ";
				state += pan_on ? "ON" : "OFF";
				if (pan_on != 0x00 && pan_on != 0xff)
				{
					sprintf(tbuf,"UNKNOWN PAN_ON 0x%02x)",pan_on);
					reportAircoError(tbuf);
				}
				state += "  mode(";
				state += modeStr("PAN",m_pan_mode);
				state += ") setpoint(";
				state += tempToStr(pan_setpoint,0);
				state += ") intake(";
				state += tempToStr(pan_intake_temp,CENTIGRADE_BIAS);
				state += ") fan(";
				state += hexByte(pan_fan);
				state += ")=";

				if (pan_fan == 0xce)
					strcpy(tbuf,"auto");
				else if (pan_fan == 0xbd)
					strcpy(tbuf,"high");
				else if (pan_fan == 0x7e)
					strcpy(tbuf,"med");
				else if (pan_fan == 0x3f)
					strcpy(tbuf,"low");
				else
				{
					sprintf(tbuf,"UNKNOWN PAN_FAN(%02x)",pan_fan);
					reportAircoError(tbuf);
				}
				state += tbuf;

			}	// if num_packets > 1

			if (_airco_state_string != state)
			{
				setString(ID_AIRCO_STATE,state);
				force_pixels = true;
					// mitigate LEDs going off (?!?!) on pump/compressor power surges(?!?!)
			}
		}
	}




	//-----------------------------------------------------------------------
	// handleData()
	//-----------------------------------------------------------------------

	void aircoDevice::handleAircoData()
	{
		packet_t packet;
		if (!xQueueReceive(packetQueue, &packet, 0))
			return;
		num_packets++;
		// skip first bogus packet
		if (num_packets == 1)
			return;

		int len = packet.len;
		const uint8_t *data = packet.data;

		// show debug of full packet while doing crc check

		uint8_t sum = 0;
		char tbuf[128];
		String tstring = "packet(" + String(num_packets) + "," + String(len) + ") ";
		for (int i=0; i<len; i++)
		{
			uint8_t byte = data[i];
			if (i>1 && i<len-1) sum ^= byte;
			sprintf(tbuf,"%02x ",byte);
			tstring += tbuf;
		}
		uint8_t crc = data[len-1];
		bool ok = sum == crc;
		tstring += ok ? "" : "CRC ERROR";
		LOGD(tstring.c_str());

		if (!ok)
		{
			reportAircoError("CRC_ERROR");
			return;
		}

		// check the signature

		String err = _airco_error_string;
		if (data[0] != 0x7e || data[1] != 0x7e)
		{
			sprintf(tbuf,"ILLEGAL_SIG(%02x%02x)",data[0],data[1]);
			reportAircoError(tbuf);
			return;
		}

		// check the type

		int packet_type = -1;
		if (data[2] == 0x00 && data[3] == 0xff)
			packet_type = PACKET_TYPE_PCB;
		else if (data[2] == 0x88 && data[3] == 0xff)
			packet_type = PACKET_TYPE_BUS;
		else if (data[2] == 0xff && data[3] == 0x00)
			packet_type = PACKET_TYPE_PANEL;
		else
		{
			sprintf(tbuf,"BAD_TYPE(%02x%02x)",data[2],data[3]);
			reportAircoError(tbuf);
			return;
		}

		// based on the type, pull out the state variables

		if (packet_type == PACKET_TYPE_PCB)
		{
			pcb_mode 			= data[PCB_MODE_OFFSET];
			m_pcb_cond_temp 	= data[PCB_COND_OFFSET];
			m_pcb_intake_temp 	= data[PCB_INTAKE_OFFSET];
			m_pcb_fan 			= data[PCB_FAN_OFFSET];
		}
		else if (packet_type == PACKET_TYPE_PANEL)
		{
			pan_on 			= data[PAN_ON_OFFSET];
			m_pan_mode 		= data[PAN_MODE_OFFSET];
			pan_setpoint 	= data[PAN_SETPOINT_OFFSET];
			pan_intake_temp = data[PAN_INTAKE_OFFSET];
			pan_fan 		= data[PAN_FAN_OFFSET];
		}

		// we don't chart till we are more or less guaranteed to
		// have gotten both sides at least once.  This helps with
		// scaling the chart to avoid initial zeros.

		static int all_packets;
		all_packets++;
		if (all_packets >= 3)
		{
			static uint8_t last_mode;
			static uint8_t last_fan;
			static uint8_t last_intake;
			static uint8_t last_cond;

			if (last_mode 	!= m_pan_mode ||
				last_fan  	!= m_pcb_fan ||
				last_intake	!= m_pcb_intake_temp ||
				last_cond   != m_pcb_cond_temp)
			{
				last_mode 	= m_pan_mode;
				last_fan  	= m_pcb_fan;
				last_intake	= m_pcb_intake_temp;
				last_cond   = m_pcb_cond_temp;
				addLogRecord();
			}
		}

	}	// handleData()



	//------------------------------------------------------------------
	// aircoTask
	//------------------------------------------------------------------
	// #include <esp_task_wdt.h>

	#define EXPECTED_LEN	19	// 0
		// When using the 3.3V RS485 module it was necessary to set this to 19
		//	as we were getting garbage bytes.
		// After switching to the 5V MAX485 module with a voltage divider (recv only)
		//	I did not as oftenget the garbage bytes but added this back so we don't
		//	get any CRC errors, but now get dropped_bytes


	static void aircoTask(void *unused_params)
	{
		// esp_task_wdt_init(0x0FFFFFFF, false);

		delay(100);
		LOGI("starting uartTask on core(%d)", xPortGetCoreID());
		delay(100);

		static uint8_t buffer[MAX_PACKET_SIZE];
		int index = 0;
		unsigned long last_rcv = 0;

		enum { WAIT_SYNC, IN_PACKET } state = WAIT_SYNC;

		while (1)
		{
			while (AIRCO_SERIAL.available())
			{
				uint8_t b = (uint8_t)AIRCO_SERIAL.read();
				last_rcv = millis();

				if (state == WAIT_SYNC)
				{
					// look for sync signature 0x7e 0x7e
					if (index == 0 && b == 0x7e)
					{
						buffer[index++] = b;
					}
					else if (index == 1)
					{
						if (buffer[0] == 0x7e && b == 0x7e)
						{
							buffer[index++] = b;
							state = IN_PACKET;
						}
						else
						{
							dropped_bytes++;
							index = 0; // reset until sync found
						}
					}
					else
					{
						dropped_bytes++;
						index = 0; // reset until sync found
					}
				}
				else if (state == IN_PACKET)
				{
					buffer[index++] = b;

					// guard against overflow
					if (index >= MAX_PACKET_SIZE)
					{
						index = 0;
						state = WAIT_SYNC;
					}

					// heuristic: minimum valid packet size is 8
					if (index >= 8)
					{
						// check for end condition: checksum or next sync
						if (b == 0x7e && buffer[index - 2] == 0x7e)
						{
							// next sync detected, cut packet before it
							packet_t packet;
							packet.len = index - 2;
							memcpy(packet.data, buffer, packet.len);
							xQueueSend(packetQueue, &packet, portMAX_DELAY);

							// start new packet with sync
							buffer[0] = 0x7e;
							buffer[1] = 0x7e;
							index = 2;
							state = IN_PACKET;
						}
					#if EXPECTED_LEN
						else if (index >= EXPECTED_LEN) // optional: known length
						{
							packet_t packet;
							packet.len = index;
							memcpy(packet.data, buffer, packet.len);

							xQueueSend(packetQueue, &packet, portMAX_DELAY);

							index = 0;
							state = WAIT_SYNC;
						}
					#endif
					}
				}
			}

			// timeout handling
			uint32_t now = millis();
			if (now - last_rcv > PACKET_TIMEOUT)
			{
				dropped_bytes += index;
				last_rcv = now;
				num_packets = 0; // flag ui that a timeout has occurred
				index = 0;
				state = WAIT_SYNC;
			}

			vTaskDelay(pdMS_TO_TICKS(1));
		}
	}


	//--------------------------------------------
	// initAircoMonitor()
	//--------------------------------------------

	void aircoDevice::initAircoMonitor()
	{
		LOGD("initAircoMonitor() starting aircoTask",0);
		AIRCO_SERIAL.begin(9600, SERIAL_8N1, PIN_RX2, PIN_TX2);
		packetQueue = xQueueCreate(MAX_QUEUE_PACKETS, sizeof(packet_t));
		xTaskCreatePinnedToCore(
			aircoTask,
			"aircoTask",
			4096,
			NULL,
			12,
			NULL,
			ESP32_CORE_OTHER);   // Core 0
	}


#endif 	// WITH_WEBASTO

// end of aircoData.cpp
