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
	#define MAX_QUEUE_PACKETS 100

	typedef struct
	{
		uint8_t data[MAX_PACKET_SIZE];
		int len;
	} packet_t;


	HardwareSerial AIRCO_SERIAL(2);
	QueueHandle_t packetQueue;

	// aircoTask() never blocks on the queue (that would stop it reading the UART);
	// if the queue is full the packet is dropped and counted here.

	static volatile uint32_t queue_overflows;

	//-------------------------------------
	// bus up/down events
	//-------------------------------------
	// "UP" means bytes are arriving from the airco RS485, "DOWN" means none
	// have arrived for PACKET_TIMEOUT (the airco has lost power, i.e. the
	// breaker has probably tripped), regardless of framing, crcs or content.
	// aircoTask() is the ONLY writer; it posts events to this ring and
	// publishBusEvents() in the loop reads and publishes them.

	#define BUS_EVENT_UP		1
	#define BUS_EVENT_DOWN		2
	#define MAX_BUS_EVENTS		8

	typedef struct
	{
		int			type;			// BUS_EVENT_xxx
		uint32_t	last_heard_ms;	// millis() of the last byte before the event (0=never)
		uint32_t	event_ms;		// millis() of the event
	} busEvent_t;

	static busEvent_t 			bus_events[MAX_BUS_EVENTS];
	static volatile uint32_t	bus_event_head;		// number of events posted (aircoTask)
	static uint32_t				bus_event_tail;		// number of events published (loop)

	static void postBusEvent(int type, uint32_t last_heard_ms, uint32_t event_ms)
	{
		busEvent_t *e = &bus_events[bus_event_head % MAX_BUS_EVENTS];
		e->type = type;
		e->last_heard_ms = last_heard_ms;
		e->event_ms = event_ms;
		bus_event_head++;
	}

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

	#if WITH_DBG_RAW_PACKETS

		// one-shot capture of the raw RS485 bytes as aircoTask() reads them,
		// armed by the DBG_RAW_PACKETS command.  The state has exactly one
		// owner for each transition, so no locking is needed:
		//
		//		IDLE -> CAPTURING	dbgRawPackets() command
		//		CAPTURING -> DONE	aircoTask(), when the buffer is full
		//		DONE -> IDLE		dbgPacketsPrint(), from the loop, after printing
		//
		// The timestamps are micros() when aircoTask() READ the byte, which
		// can lag the actual arrival by about a millisecond (the task polls).

		#define DBG_PACKET_MAX			256
		#define DBG_PACKET_GAP_US		3000
			// bytes further apart than this start a new line in the output
		#define DBG_PACKET_LINE_MAX		32
			// maximum bytes printed on one line

		#define DBG_PACKET_IDLE			0
		#define DBG_PACKET_CAPTURING	1
		#define DBG_PACKET_DONE			2

		static volatile int	dbg_packet_state = DBG_PACKET_IDLE;
		static volatile int	dbg_packet_count = 0;
		static uint32_t		dbg_packet_us[DBG_PACKET_MAX];
		static uint8_t		dbg_packet_bytes[DBG_PACKET_MAX];

	#endif	// WITH_DBG_RAW_PACKETS

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


	#if WITH_DBG_RAW_PACKETS

		void aircoDevice::dbgRawPackets()
			// command: arm a one-shot capture of the next DBG_PACKET_MAX raw bytes
		{
			if (dbg_packet_state != DBG_PACKET_IDLE)
			{
				LOGW("dbgRawPackets() ignored; a capture is already in progress");
				return;
			}
			dbg_packet_count = 0;
			dbg_packet_state = DBG_PACKET_CAPTURING;
			LOGU("dbgRawPackets() capturing %d bytes ...",DBG_PACKET_MAX);
		}


		static void dbgPacketsPrint()
			// called from handleAircoData() in the loop; prints a finished
			// capture, one line per "burst" of bytes, and re-arms the command
		{
			if (dbg_packet_state != DBG_PACKET_DONE)
				return;

			static char line[DBG_PACKET_LINE_MAX * 3 + 1];

			int count = dbg_packet_count;
			uint32_t t0 = dbg_packet_us[0];
			uint32_t span = dbg_packet_us[count-1] - t0;
			LOGU("dbgRawPackets() captured %d bytes over %u.%03u ms",
				count,
				(unsigned) (span / 1000),
				(unsigned) (span % 1000));

			int i = 0;
			while (i < count)
			{
				int start = i;
				char *p = line;
				while (i < count &&
					   i - start < DBG_PACKET_LINE_MAX &&
					   (i == start || dbg_packet_us[i] - dbg_packet_us[i-1] <= DBG_PACKET_GAP_US))
				{
					p += sprintf(p,"%02x ",dbg_packet_bytes[i]);
					i++;
				}

				uint32_t at  = dbg_packet_us[start] - t0;
				uint32_t gap = start ? dbg_packet_us[start] - dbg_packet_us[start-1] : 0;
				LOGU("dbgRaw t(%u.%03u ms) gap(%u us) len(%d): %s",
					(unsigned) (at / 1000),
					(unsigned) (at % 1000),
					(unsigned) gap,
					i - start,
					line);
			}

			dbg_packet_state = DBG_PACKET_IDLE;
		}

	#endif	// WITH_DBG_RAW_PACKETS



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

	static void busTimeText(time_t t, char *buf, int len)
		// local time text, in the device's TIMEZONE, for the log lines
	{
		struct tm tm_local;
		localtime_r(&t,&tm_local);
		strftime(buf,len,"%Y-%m-%d %H:%M:%S",&tm_local);
	}


	void aircoDevice::publishBusEvents()
		// called from handleAircoData() in the loop.  Publishes the up/down
		// events posted by aircoTask() as one log line each, and as the
		// AIRCO_BUS value, which only changes on an edge so it can be polled.
	{
		static int 		bus_state = -1;		// -1=unknown, 0=down, 1=up
		static time_t	bus_since = 0;		// epoch of the start of the current state
		static uint32_t	bus_outages = 0;	// times the bus went from up to down
		static time_t	bus_last_down = 0;	// epoch of the last byte heard before the last outage
		static uint32_t	bus_down_secs = 0;	// length of the last outage

		// TEST_NO_PACKETS only changes what is displayed; the real state above is
		// always tracked from the real events

		static uint32_t	test_outages = 0;	// fake outages added to the outage count
		static bool		test_on = false;	// the fake DOWN is currently being displayed
		static time_t	test_since = 0;		// epoch the current fake outage began
		static time_t	test_last_down = 0;	// epoch of the last completed fake outage
		static uint32_t	test_down_secs = 0;	// and its length

		bool changed = false;

		static uint32_t last_overflows = 0;
		if (queue_overflows != last_overflows)
		{
			LOGW("RS485 packet queue overflow; %u packets dropped",(unsigned) (queue_overflows - last_overflows));
			last_overflows = queue_overflows;
		}

		uint32_t head = bus_event_head;
		if (head - bus_event_tail > MAX_BUS_EVENTS)
			bus_event_tail = head - MAX_BUS_EVENTS;
				// the loop stalled long enough to lose the oldest events

		while (bus_event_tail != head)
		{
			busEvent_t e = bus_events[bus_event_tail % MAX_BUS_EVENTS];
			bus_event_tail++;

			time_t time_now = time(NULL);
			uint32_t ms_now = millis();
			time_t at = time_now - (time_t) ((ms_now - e.event_ms) / 1000);
			time_t heard = e.last_heard_ms ?
				time_now - (time_t) ((ms_now - e.last_heard_ms) / 1000) : 0;

			char at_text[32];
			char heard_text[32];
			busTimeText(at,at_text,sizeof(at_text));
			busTimeText(heard,heard_text,sizeof(heard_text));

			if (e.type == BUS_EVENT_UP)
			{
				if (bus_state == 0 && e.last_heard_ms)
				{
					bus_down_secs = (e.event_ms - e.last_heard_ms) / 1000;
					LOGU("bus UP at %u (%s); was down %us; last heard %u (%s)",
						(unsigned) at, at_text,
						(unsigned) bus_down_secs,
						(unsigned) heard, heard_text);
				}
				else
				{
					LOGU("bus UP at %u (%s); first bytes since boot",
						(unsigned) at, at_text);
				}
				bus_state = 1;
				bus_since = at;
			}
			else	// BUS_EVENT_DOWN
			{
				if (bus_state == 1)
				{
					bus_outages++;
					bus_last_down = heard;
					LOGE("bus DOWN; last heard %u (%s); was up %us",
						(unsigned) heard, heard_text,
						(unsigned) (heard - bus_since));
					bus_since = heard;
				}
				else
				{
					LOGE("bus DOWN at %u (%s); no bytes since boot",
						(unsigned) at, at_text);
					bus_since = at;
				}
				bus_state = 0;
			}

			changed = true;
		}

		// the TEST_NO_PACKETS switch fakes a DOWN in the AIRCO_BUS value and
		// the log, without touching the airco data or the real tracking

		if (_test_no_packets != test_on)
		{
			time_t time_now = time(NULL);
			char now_text[32];
			busTimeText(time_now,now_text,sizeof(now_text));
			test_on = _test_no_packets;
			if (test_on)
			{
				test_outages++;
				test_since = time_now;
				LOGE("bus DOWN (TEST) at %u (%s)",
					(unsigned) time_now, now_text);
			}
			else
			{
				test_last_down = test_since;
				test_down_secs = (uint32_t) (time_now - test_since);
				LOGU("bus UP (TEST) at %u (%s); was down %us",
					(unsigned) time_now, now_text,
					(unsigned) test_down_secs);
			}
			changed = true;
		}

		if (changed)
		{
			const char *state_text =
				test_on ? "DOWN" :
				bus_state == 1 ? "UP" :
				bus_state == 0 ? "DOWN" : "UNKNOWN";
			time_t since = test_on ? test_since : bus_since;
			time_t last_down = test_on ? test_since : bus_last_down;
			uint32_t down_secs = bus_down_secs;
			if (!test_on && test_last_down > bus_last_down)
			{
				last_down = test_last_down;
				down_secs = test_down_secs;
			}

			// line 1 is for the poller (epoch seconds); line 2 is for people

			char buf[224];
			sprintf(buf,"%s since=%u outages=%u last_down=%u down_secs=%u%s",
				state_text,
				(unsigned) since,
				(unsigned) (bus_outages + test_outages),
				(unsigned) last_down,
				(unsigned) down_secs,
				test_on ? " test=1" : "");
			if (since)
			{
				char since_text[32];
				busTimeText(since,since_text,sizeof(since_text));
				sprintf(buf + strlen(buf),"<br>%s since %s",state_text,since_text);
				if (last_down)
				{
					char down_text[32];
					busTimeText(last_down,down_text,sizeof(down_text));
					sprintf(buf + strlen(buf),"  last_down %s",down_text);
				}
			}
			setString(ID_AIRCO_BUS,buf);
		}
	}


	void aircoDevice::handleAircoData()
	{
		#if WITH_DBG_RAW_PACKETS
			dbgPacketsPrint();
		#endif

		publishBusEvents();

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
		LOGV(tstring.c_str());

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
			m_have_pcb			= true;
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

		uint32_t last_byte_ms = 0;	// for the bus edges; last_rcv is reset by the timeout handling
		int bus_up = -1;			// -1=no bytes yet, 0=down, 1=up

		enum { WAIT_SYNC, IN_PACKET } state = WAIT_SYNC;

		while (1)
		{
			while (AIRCO_SERIAL.available())
			{
				uint8_t b = (uint8_t)AIRCO_SERIAL.read();
				last_rcv = millis();

				uint32_t prev_byte_ms = last_byte_ms;
				last_byte_ms = last_rcv;
				if (bus_up != 1)
				{
					bus_up = 1;
					postBusEvent(BUS_EVENT_UP,prev_byte_ms,last_byte_ms);
				}

				#if WITH_DBG_RAW_PACKETS
					if (dbg_packet_state == DBG_PACKET_CAPTURING)
					{
						dbg_packet_us[dbg_packet_count] = micros();
						dbg_packet_bytes[dbg_packet_count] = b;
						if (++dbg_packet_count >= DBG_PACKET_MAX)
							dbg_packet_state = DBG_PACKET_DONE;
					}
				#endif

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
							if (xQueueSend(packetQueue, &packet, 0) != pdTRUE)
								queue_overflows++;

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

							if (xQueueSend(packetQueue, &packet, 0) != pdTRUE)
								queue_overflows++;

							index = 0;
							state = WAIT_SYNC;
						}
					#endif
					}
				}
			}

			// timeout handling
			uint32_t now = millis();

			// no bytes at all for PACKET_TIMEOUT means the bus is down

			if (bus_up != 0 && now - last_byte_ms > PACKET_TIMEOUT)
			{
				bus_up = 0;
				postBusEvent(BUS_EVENT_DOWN,last_byte_ms,now);
			}

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
		setString(ID_AIRCO_BUS,"UNKNOWN");
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
