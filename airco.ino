//-----------------------------------------------------------------------
// airco.ino
//-----------------------------------------------------------------------
// myIOT device to monitor airco and handle air conditioner condensate pump.

#define WITH_PIXELS 0

#include "airco.h"
#include <myIOTLog.h>
#include <myIOTDataLog.h>

#if WITH_PIXELS
	#include <Adafruit_NeoPixel.h>
#endif


//---------------------------------------
// uart queue
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


HardwareSerial aircoSerial(2);
QueueHandle_t packetQueue;

void uartTask(void *unused_params);
	// forward


//---------------------------------
// static state machine variables
//---------------------------------

static int num_packets = 1;
static int dropped_bytes = 0;


static uint8_t pcb_mode;
static uint8_t pcb_cond_temp;
static uint8_t pcb_intake_temp;
static uint8_t pcb_fan;

static uint8_t pan_on;
static uint8_t pan_mode;
static uint8_t pan_setpoint;
static uint8_t pan_intake_temp;
static uint8_t pan_fan;

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


// my %STATE_FIELDS = (
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


//--------------------------------
// pixels
//--------------------------------

#define DEFAULT_LED_BRIGHTNESS	32
#define INITIAL_LED_BRIGHTNESS 	128

#if WITH_PIXELS

	#define PIXEL_SYSTEM    0
	#define PIXEL_STATE     1
	#define NUM_PIXELS		2

	#define MY_LED_BLACK    0x000000
	#define MY_LED_RED      0x440000
	#define MY_LED_GREEN    0x003300
	#define MY_LED_BLUE     0x000044
	#define MY_LED_CYAN     0x003322
	#define MY_LED_YELLOW   0x333300
	#define MY_LED_MAGENTA  0x440044
	#define MY_LED_WHITE    0x444444
	#define MY_LED_ORANGE   0x402200
	#define MY_LED_REDMAG   0x400022
	#define MY_LED_PURPLE   0x220022
	#define MY_LED_BLUECYAN 0x002240

	static Adafruit_NeoPixel pixels(NUM_PIXELS,PIN_LED);
	static uint32_t last_sys_pixel = MY_LED_BLACK;
	static uint32_t last_state_pixel = MY_LED_BLACK;
	static bool force_pixels = 0;

	static void showPixels()						{pixels.show();}
	static void clearPixels()						{pixels.clear();}
	static void setPixelsBrightness(int val)		{pixels.setBrightness(val);}
	static void setPixel(int num, uint32_t color)	{pixels.setPixelColor(num,color);}
#endif


//------------------------------
// myIOT setup
//------------------------------

static valueIdType dash_items[] = {
	ID_STATE_STRING,
	ID_ERROR_STRING,
	ID_CLEAR_ERROR,
	ID_UI_INTERVAL,
	ID_PLOT_INTERVAL,
	ID_LED_BRIGHTNESS,
	ID_CHART_LINK,
    0
};

static valueIdType config_items[] = {
	0
};


const valDescriptor airco_values[] =
{
    {ID_DEVICE_NAME,	VALUE_TYPE_STRING,	VALUE_STORE_PREF,	VALUE_STYLE_REQUIRED,	NULL,	NULL,	AIRCO_DEVICE },
 	{ID_STATE_STRING,   VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_state_string,    	},
  	{ID_ERROR_STRING,   VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_error_string,		},
	{ID_CLEAR_ERROR,    VALUE_TYPE_COMMAND, VALUE_STORE_SUB,    VALUE_STYLE_NONE,       NULL, (void *) aircoDevice::clearError },
 	{ID_UI_INTERVAL,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_ui_interval,		NULL,	{ .int_range = {1000,1000,30000}}, 	},
 	{ID_PLOT_INTERVAL,  VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_plot_interval,		NULL,	{ .int_range = {3000,1000,30000}}, 	},
	{ID_LED_BRIGHTNESS, VALUE_TYPE_INT, 	VALUE_STORE_PREF,	VALUE_STYLE_NONE,		(void *) &aircoDevice::_led_brightness,	(void *) aircoDevice::onBrightnessChanged, { .int_range = { DEFAULT_LED_BRIGHTNESS,  10,  255}} },
	{ID_CHART_LINK,		VALUE_TYPE_STRING,	VALUE_STORE_PUB,	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_chart_link, },
};


#define NUM_AIRCO_VALUES (sizeof(airco_values)/sizeof(valDescriptor))

// chart stuff

extern void addHistoryRecord(uint8_t mode, uint8_t fan, uint8_t intake, uint8_t cond);
	// defined in aircoHistory.h

logColumn_t  airco_cols[] = {
	{"intake",		LOG_COL_TYPE_TEMPERATURE,	10,	},
	{"condensor",	LOG_COL_TYPE_TEMPERATURE,	10,	},
	{"fan",			LOG_COL_TYPE_UINT32,		100,},
	{"compressor",	LOG_COL_TYPE_UINT32,		10,	},
	{"mode",		LOG_COL_TYPE_UINT32,		100,},
};

myIOTDataLog data_log("aircoData",5,airco_cols,0);
	// extern'd in aircoHistory.cpp
	// 0 = debug_send_data level


// vars

String		aircoDevice::_state_string;
String		aircoDevice::_error_string;
int   		aircoDevice::_ui_interval;
int   		aircoDevice::_plot_interval;
int			aircoDevice::_led_brightness;
String		aircoDevice::_chart_link;

aircoDevice *airco;



//--------------------------------
// main
//--------------------------------

void setup()
{

#if WITH_PIXELS
	setPixelsBrightness(INITIAL_LED_BRIGHTNESS);
	setPixel(PIXEL_SYSTEM,MY_LED_CYAN);
	setPixel(PIXEL_STATE,MY_LED_CYAN);
	showPixels();
#endif

    Serial.begin(MY_IOT_ESP32_CORE == 3 ? 115200 : 921600);
    delay(1000);

    LOGU("aircoDevice.ino setup() started on core(%d)",xPortGetCoreID());
    airco = new aircoDevice();
    airco->addValues(airco_values,NUM_AIRCO_VALUES);
    airco->setTabLayouts(dash_items,config_items);

#if WITH_PIXELS
    LOGU("calling airco->setup()",0);
	setPixel(PIXEL_STATE,MY_LED_GREEN);
	showPixels();
#endif
	airco->setup();
	
	// note that airco_chart.html must be uploaded to SPIFFS by hand
	airco->_chart_link = "<a href='/spiffs/chart.html?uuid=";
	airco->_chart_link += airco->getUUID();
	airco->_chart_link += "' target='_blank'>Chart</a>";

    LOGU("starting uartTask",0);
	aircoSerial.begin(9600, SERIAL_8N1, PIN_RX2, PIN_TX2);
	packetQueue = xQueueCreate(MAX_QUEUE_PACKETS, sizeof(packet_t));
    xTaskCreatePinnedToCore(
        uartTask,
        "uartTask",
        4096,
        NULL,
        12,
        NULL,
        ESP32_CORE_OTHER);   // Core 0

#if WITH_PIXELS
	setPixelsBrightness(airco->_led_brightness);
	setPixel(PIXEL_SYSTEM,MY_LED_BLACK);
	setPixel(PIXEL_STATE,MY_LED_BLACK);
	showPixels();
#endif

	LOGU("aircoDevice.ino setup() finished",0);
}



void loop()
{
    airco->loop();			// handle myIOT device
	airco->handleData();	// handle device specific behavior
	airco->updateUI();		// update webUI
#if WITH_PIXELS
	airco->handlePixels();
#endif
}


//-------------------------------------------
// implementation
//-------------------------------------------


void aircoDevice::onBrightnessChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onBrightnessChanged(%d)",val);
#if WITH_PIXELS
	setPixelsBrightness(val);
	force_pixels = 1;
#endif
}

void aircoDevice::clearError()
{
	LOGU("clearError()");
	airco->setString(ID_ERROR_STRING,"");
	dropped_bytes = 0;
	num_packets = 1;
}


void aircoDevice::reportError(const char *msg)
{
	if (_error_string.indexOf(msg) == -1)
	{
		String err = _error_string;
		if (err != "")
			err += "; ";
		err += msg;
		setString(ID_ERROR_STRING,err);
	}
}



void aircoDevice::handlePixels()
	// system LED:  cyan=booting; green=STA; purple=AP; orange=both; magenta=off/error
	// state LED: green=off; red=on; flashing green=error
{
#if WITH_PIXELS
	#define SYS_FLASH_TIME  250

	uint32_t color_system = MY_LED_MAGENTA;
	if (getBool(ID_WIFI))
	{
		iotConnectStatus_t wifi_mode = getConnectStatus();
		if (wifi_mode == IOT_CONNECT_ALL)
			color_system = MY_LED_ORANGE;
		else if (wifi_mode == IOT_CONNECT_AP)
			color_system = MY_LED_MAGENTA;
		else if (wifi_mode == IOT_CONNECT_STA)
			color_system = MY_LED_GREEN;
	}

	bool changed = 0;
	if (last_sys_pixel != color_system)
	{
		last_sys_pixel = color_system;
		setPixel(PIXEL_SYSTEM,color_system);
		changed = 1;
	}

	uint32_t now = millis();
	static bool last_error_existed = 0;
	static uint32_t state_blink_time = 0;
	static uint32_t state_blink_on = 0;
	uint32_t color_state = MY_LED_GREEN;

	bool error_exists = _error_string != "";

	if (last_state_pixel != color_state ||
		last_error_existed != error_exists)
	{
		last_state_pixel = color_state;
		last_error_existed = error_exists;
		state_blink_time = error_exists ? SYS_FLASH_TIME : 0;
		setPixel(PIXEL_STATE,color_state);
		state_blink_time = now;
		changed = 1;
	}
	else if (last_error_existed && (now - state_blink_time >= SYS_FLASH_TIME))
	{
		state_blink_time = now;
		state_blink_on = !state_blink_on;
		setPixel(PIXEL_STATE,state_blink_on?color_state:MY_LED_BLACK);
		changed = 1;
	}

	if (changed || force_pixels)
	{
		force_pixels = 0;
		showPixels();
	}
#endif
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
	airco->reportError(buf);
	return buf;
}



void aircoDevice::updateUI()
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
			reportError("NO PACKETS!");

		// skip 0th bogus packet
		
		if (num_packets > 1)
		{
			// PCB line

			state += "PCB mode(";
			state += modeStr("PCB",pcb_mode);
			state += ") cond(";
			state += tempToStr(pcb_cond_temp,CENTIGRADE_BIAS);
			state += ") intake(";
			state += tempToStr(pcb_intake_temp,CENTIGRADE_BIAS);
			state += ") fan(";
			state += hexByte(pcb_fan);
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

			state += (pcb_fan & 0x80) ? "user_" : "auto_";

			uint8_t fan_nibble = (pcb_fan & 0x70);
			state +=
				fan_nibble == 0x10 ? "high" :
				fan_nibble == 0x20 ? "med" :
				fan_nibble == 0x40 ? "low" : "off";

			static char tbuf[60];
			uint8_t comp_on   = pcb_fan & 0x0f;
			if (comp_on == 0x00)
				strcpy(tbuf," comp_off");
			else if (comp_on = 0x80)
				strcpy(tbuf," comp_on");
			else
			{
				sprintf(tbuf," UNKNOWN FAN_COMP(%d)",comp_on);
				reportError(tbuf);
			}
			state += tbuf;
			state += "<br>";

			// PANEL line

			state += "PANEL ";
			state += pan_on ? "ON" : "OFF";
			if (pan_on != 0x00 && pan_on != 0xff)
			{
				sprintf(tbuf,"UKNOWN PAN_ON 0x%02x)",pan_on);
				reportError(tbuf);
			}
			state += "  mode(";
			state += modeStr("PAN",pan_mode);
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
				reportError(tbuf);
			}
			state += tbuf;
			
		}	// if num_packets > 1
		
		if (_state_string != state)
			setString(ID_STATE_STRING,state);
	}
}




//-----------------------------------------------------------------------
// handleData()
//-----------------------------------------------------------------------


void aircoDevice::handleData()
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
	LOGV(tstring.c_str());

	if (!ok)
	{
		reportError("CRC_ERROR");
		return;
	}
	
	// check the signature

	String err = _error_string;
	if (data[0] != 0x7e || data[1] != 0x7e)
	{
		sprintf(tbuf,"ILLEGAL_SIG(%02x%02x)",data[0],data[1]);
		reportError(tbuf);
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
		reportError(tbuf);
		return;
	}

	// based on the type, pull out the state variables

	if (packet_type == PACKET_TYPE_PCB)
	{
		pcb_mode 		= data[PCB_MODE_OFFSET];
		pcb_cond_temp 	= data[PCB_COND_OFFSET];
		pcb_intake_temp = data[PCB_INTAKE_OFFSET];
		pcb_fan 		= data[PCB_FAN_OFFSET];
	}
	else if (packet_type == PACKET_TYPE_PANEL)
	{
		pan_on 			= data[PAN_ON_OFFSET];
		pan_mode 		= data[PAN_MODE_OFFSET];
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

		if (last_mode 	!= pan_mode ||
			last_fan  	!= pcb_fan ||
			last_intake	!= pcb_intake_temp ||
			last_cond   != pcb_cond_temp)
		{
			last_mode 	= pan_mode;
			last_fan  	= pcb_fan;
			last_intake	= pcb_intake_temp;
			last_cond   = pcb_cond_temp;
			addHistoryRecord(pan_mode, pcb_fan, pcb_intake_temp, pcb_cond_temp);
		}
	}

}	// handleData()



//------------------------------------------------------------------
// uartTask
//------------------------------------------------------------------
// #include <esp_task_wdt.h>

#define EXPECTED_LEN	19	// 0
	// When using the 3.3V RS485 module it was necessary to set this to 19
	//	as we were getting garbage bytes.
	// After switching to the 5V MAX485 module with a voltage divider (recv only)
	//	I did not as oftenget the garbage bytes but added this back so we don't
	//	get any CRC errors, but now get dropped_bytes


void uartTask(void *unused_params)
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
        while (aircoSerial.available())
        {
            uint8_t b = (uint8_t)aircoSerial.read();
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
