//-----------------------------------------------------------------------
// airco.ino
//-----------------------------------------------------------------------
// myIOT device to monitor airco and handle air conditioner condensate pump.

#include "airco.h"
#include <myIOTLog.h>
#include <myIOTDataLog.h>
#include <Adafruit_NeoPixel.h>

#define ALIVE_OFF_TIME	980
#define ALIVE_ON_TIME	20

//--------------------------------
// pixels
//--------------------------------

#define DEFAULT_LED_BRIGHTNESS	32
#define INITIAL_LED_BRIGHTNESS 	128

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


//------------------------------
// myIOT setup
//------------------------------


static valueIdType dash_items[] = {
#if WITH_WEBASTO
	ID_AIRCO_STATE,
	ID_AIRCO_ERROR,
#endif
	ID_CLEAR_ERROR,

	ID_DRAIN_STATE,
	ID_DRAIN_MODE,

	ID_STATE_LAST_RUN,
	ID_TIME_LAST_RUN,
	ID_DUR_LAST_RUN,
	ID_SINCE_LAST_RUN,

	ID_DRAIN_HISTORY,
	ID_CHART,
    0
};

static valueIdType config_items[] = {
	ID_ON_THRESH,
	ID_OFF_THRESH,
	ID_EXTRA_TIME,
	ID_MAX_TIME,
	ID_COOLDOWN_TIME,
	ID_IDLE_TIME,
	ID_IDLE_THRESH,

	ID_DRIVE_TIME_US,
    ID_SAMPLE_TIME,
    ID_NUM_SAMPLES,

    ID_LOG_WINDOW,
	ID_LED_BRIGHTNESS,
	ID_UI_INTERVAL,

	0
};


enumValue drainModes[] = { "Off", "ON", "FORCE", 0};


const valDescriptor airco_values[] =
{
    {ID_DEVICE_NAME,	VALUE_TYPE_STRING,	VALUE_STORE_PREF,	VALUE_STYLE_REQUIRED,	NULL,	NULL,	AIRCO_DEVICE },
#if WITH_WEBASTO
 	{ID_AIRCO_STATE,	VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_airco_state_string,	},
  	{ID_AIRCO_ERROR,	VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_airco_error_string,	},
#endif
	{ID_CLEAR_ERROR,    VALUE_TYPE_COMMAND, VALUE_STORE_SUB,    VALUE_STYLE_NONE,       NULL, (void *) aircoDevice::clearAircoError },
 
 	{ID_DRAIN_STATE,	VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_drain_state_string,	},
	{ID_DRAIN_MODE,		VALUE_TYPE_ENUM,	VALUE_STORE_PREF,	VALUE_STYLE_NONE,		(void *) &aircoDevice::_drain_mode,		(void *) aircoDevice::onDrainModeChanged, 	{ .enum_range = { 0, drainModes }} },

	{ID_STATE_LAST_RUN,	VALUE_TYPE_STRING,  VALUE_STORE_PUB, 	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_state_last_run,		},
    {ID_TIME_LAST_RUN,  VALUE_TYPE_TIME,    VALUE_STORE_PUB,    VALUE_STYLE_READONLY,   (void *) &aircoDevice::_time_last_run,		},
    {ID_SINCE_LAST_RUN, VALUE_TYPE_INT,     VALUE_STORE_PUB,    VALUE_STYLE_HIST_TIME,  (void *) &aircoDevice::_since_last_run,NULL,   { .int_range = { 0, DEVICE_MIN_INT, DEVICE_MAX_INT}}  },
    {ID_DUR_LAST_RUN,   VALUE_TYPE_INT,     VALUE_STORE_PUB,    VALUE_STYLE_READONLY,   (void *) &aircoDevice::_dur_last_run,  NULL,   { .int_range = { 0, 0, DEVICE_MAX_INT}}  },

	{ID_ON_THRESH,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_on_thresh,		NULL,	{ .int_range = {500,  1,   4094}}, 	},
 	{ID_OFF_THRESH,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_off_thresh,		NULL,	{ .int_range = {10,   1,   4094}}, 	},
 	{ID_EXTRA_TIME,		VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_extra_time,		NULL,	{ .int_range = {0,    0,   30}}, 	},
 	{ID_MAX_TIME,		VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_max_time,		NULL,	{ .int_range = {120,  30,  360}}, 	},
 	{ID_COOLDOWN_TIME,  VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_cooldown_time,  NULL,	{ .int_range = {60,   10,  900}}, 	},
 	{ID_IDLE_TIME,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_idle_time,		NULL,	{ .int_range = {3600, 0,   86400}},	},
 	{ID_IDLE_THRESH,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_idle_thresh,    NULL,	{ .int_range = {60,   1,   4094}},  },

 	{ID_DRIVE_TIME_US,  VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_drive_time_us,  NULL,	{ .int_range = {1000, 100, 3000}},  },
 	{ID_SAMPLE_TIME,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_sample_time,    NULL,	{ .int_range = {1000, 100, 3000}},  },
 	{ID_NUM_SAMPLES,	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_num_samples,NULL,	{ .int_range = {5,   1,   MAX_SENSOR_SAMPLES}} },

 	{ID_LOG_WINDOW,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_log_window,    	NULL,	{ .int_range = {50,   0,   100}}, },
	{ID_UI_INTERVAL,  	VALUE_TYPE_INT,  	VALUE_STORE_PREF, 	VALUE_STYLE_NONE,		(void *) &aircoDevice::_ui_interval,	NULL,	{ .int_range = {1000,1000,30000}}, 	},
	{ID_LED_BRIGHTNESS, VALUE_TYPE_INT, 	VALUE_STORE_PREF,	VALUE_STYLE_NONE,		(void *) &aircoDevice::_led_brightness,	(void *) aircoDevice::onBrightnessChanged, { .int_range = { DEFAULT_LED_BRIGHTNESS,  10,  255}} },

	{ID_DRAIN_HISTORY,	VALUE_TYPE_STRING,	VALUE_STORE_PUB,	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_drain_history_link, },
	{ID_CHART,			VALUE_TYPE_STRING,	VALUE_STORE_PUB,	VALUE_STYLE_READONLY,	(void *) &aircoDevice::_chart_link,			},
};

#define NUM_AIRCO_VALUES (sizeof(airco_values)/sizeof(valDescriptor))

static const char *plot_legend = "cur,avg,pump";


// vars

#if WITH_WEBASTO
	String 		aircoDevice::_airco_state_string;
	String 		aircoDevice::_airco_error_string;
#endif

String 		aircoDevice::_drain_state_string;
uint32_t	aircoDevice::_drain_mode;

String 		aircoDevice::_state_last_run;
time_t  	aircoDevice::_time_last_run;
int     	aircoDevice::_since_last_run;
int     	aircoDevice::_dur_last_run;

int			aircoDevice::_on_thresh;
int			aircoDevice::_off_thresh;
int			aircoDevice::_extra_time;
int			aircoDevice::_max_time;
int			aircoDevice::_cooldown_time;
int			aircoDevice::_idle_time;
int			aircoDevice::_idle_thresh;

int			aircoDevice::_drive_time_us;
int			aircoDevice::_sample_time;
int			aircoDevice::_num_samples;

int			aircoDevice::_log_window;
int			aircoDevice::_ui_interval;
int			aircoDevice::_led_brightness;

String		aircoDevice::_drain_history_link;
String		aircoDevice::_chart_link;

int			aircoDevice::m_pump_on;
time_t 		aircoDevice::m_run_start;
uint32_t 	aircoDevice::m_run_flags;
int 		aircoDevice::m_sensor_avg;
uint32_t 	aircoDevice::m_flags_last_run;

#if WITH_WEBASTO
	uint8_t		aircoDevice::m_pcb_cond_temp;
	uint8_t		aircoDevice::m_pcb_intake_temp;
	uint8_t		aircoDevice::m_pcb_fan;
	uint8_t		aircoDevice::m_pan_mode;
#endif

aircoDevice *airco;



//--------------------------------
// setup()
//--------------------------------

void setup()
{
	pinMode(PIN_TX_EN,OUTPUT);
	digitalWrite(PIN_TX_EN,0);

	pinMode(PIN_ALIVE_LED,OUTPUT);
	digitalWrite(PIN_ALIVE_LED,1);

	pinMode(PIN_SDRIVE,OUTPUT);
	digitalWrite(PIN_SDRIVE,0);

	pinMode(PIN_SENSOR,INPUT);

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

    LOGU("calling airco->setup()",0);
	setPixel(PIXEL_STATE,MY_LED_GREEN);
	showPixels();

	airco->setup();

    airco->_drain_history_link = "<a href='/custom/getDrainHistory:?uuid=";
    airco->_drain_history_link += airco->getUUID();
    airco->_drain_history_link += "' target='_blank'>Drain History</a>";
	
	airco->_chart_link = "<a href='/spiffs/chart.html?data_name=combinedData&uuid=";
	airco->_chart_link += airco->getUUID();
	airco->_chart_link += "' target='_blank'>Chart</a>";

	airco->setPlotLegend(plot_legend);

#if WITH_WEBASTO
	airco->initAircoMonitor();		// start uart task
#endif

	setPixelsBrightness(airco->_led_brightness);
	setPixel(PIXEL_SYSTEM,MY_LED_BLACK);
	setPixel(PIXEL_STATE,MY_LED_BLACK);
	showPixels();

	digitalWrite(PIN_ALIVE_LED,0);
	LOGU("aircoDevice.ino setup() finished",0);
}



//-------------------------------------------
// implementation
//-------------------------------------------


void aircoDevice::onBrightnessChanged(const myIOTValue *desc, uint32_t val)
{
	LOGU("onBrightnessChanged(%d)",val);
	setPixelsBrightness(val);
	force_pixels = 1;
}



void aircoDevice::handlePixels()
	// system LED:  cyan=booting; green=STA; purple=AP; orange=both; magenta=off/error
	// drainPump LED: green=off; blue=on; flashing green/blue=pump ran too long
{
	#define SYS_FLASH_TIME  250

	uint32_t color_system = MY_LED_MAGENTA;
	iotConnectStatus_t wifi_mode = getConnectStatus();
	if (wifi_mode == IOT_CONNECT_ALL)
		color_system = MY_LED_ORANGE;
	else if (wifi_mode == IOT_CONNECT_AP)
		color_system = MY_LED_MAGENTA;
	else if (wifi_mode == IOT_CONNECT_STA)
		color_system = MY_LED_GREEN;

	bool changed = 0;
	if (last_sys_pixel != color_system)
	{
		last_sys_pixel = color_system;
		setPixel(PIXEL_SYSTEM,color_system);
		changed = 1;
	}

	uint32_t now = millis();
	static int led_error_code = 0;
	static uint32_t state_blink_time = 0;
	static uint32_t state_blink_on = 0;

	// if the pump runs too long, the led will start flashing green
	// when it goes off, and will continue flashing through blue on
	// the next run, until the next run ends.
	
	int error_code = m_flags_last_run & DRAIN_FLAG_TOO_LONG;
	uint32_t color_state = m_pump_on ? MY_LED_BLUE : MY_LED_GREEN;

	if (last_state_pixel != color_state ||
		led_error_code != error_code)
	{
		last_state_pixel = color_state;
		led_error_code = error_code;
		state_blink_time = error_code ? SYS_FLASH_TIME : 0;
		setPixel(PIXEL_STATE,color_state);
		state_blink_time = now;
		changed = 1;
	}
	else if (led_error_code && (now - state_blink_time >= SYS_FLASH_TIME))
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
}


//--------------------------------------
// loop()
//--------------------------------------


void loop()
{
    airco->loop();				// handle myIOT device
#if WITH_WEBASTO
	airco->handleAircoData();	// handle airco data queue
	airco->updateAircoUI();
#endif
	airco->handleDrainPump();	// handle drainPump
	airco->updateDrainPumpUI();
	airco->handlePixels();

	#if PIN_ALIVE_LED
		static bool alive_on = 0;
		static uint32_t last_alive_time = 0;
		uint32_t alive_now = millis();
		uint32_t alive_delay = alive_on ? ALIVE_ON_TIME : ALIVE_OFF_TIME;
		if (alive_now - last_alive_time >= alive_delay)
		{
			alive_on = !alive_on;
			digitalWrite(PIN_ALIVE_LED,alive_on);
			last_alive_time = alive_now;
		}
	#endif
}


// end of airco.ino

