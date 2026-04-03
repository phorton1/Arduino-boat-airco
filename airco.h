//----------------------------------------------
// Constants for airco.ino
//----------------------------------------------

#pragma once

#include <myIOTDevice.h>

#define WITH_WEBASTO	1
	// set to 0 to make this "just a drain pump controller"
	//allows for use without the RS485/Webasto monitor



//=========================================================
// pins
//=========================================================

#define PIN_ALIVE_LED	2
#define PIN_TX_EN		4
#define PIN_RX2			16
#define PIN_TX2			17
#define PIN_PUMP		33
#define PIN_SENSOR		36
#define PIN_SDRIVE		25
#define PIN_LED			32


// SD CARD
// PIN_MOSI		23
// PIN_MISO		19
// PIN_SCLK		18
// PIN_SDCS		5


//------------------------
// myIOT definition
//------------------------

#define AIRCO_DEVICE			"airco_device"
#define AIRCO_DEVICE_VERSION	"ac2.0"
#define AIRCO_DEVICE_URL		"https://github.com/phorton1/Arduino-boat-airco"

// AIRCO

#if WITH_WEBASTO
	#define ID_AIRCO_STATE			"AIRCO_STATE"		// readonly string containing airco state
	#define ID_AIRCO_ERROR			"AIRCO_ERROR"		// readonly string containing cumulative airco errors
#endif

// DRAIN PUMP

#define ID_DRAIN_STATE			"DRAIN_STATE"		// readonly string containing drain pump state
#define ID_DRAIN_MODE			"DRAIN_MODE"		// enum select

#define ID_STATE_LAST_RUN		"STATE_LAST_RUN"	// most recent drain pump error/notice, if any
#define ID_TIME_LAST_RUN    	"TIME_LAST_RUN"		// statistics regarding the last completed run
#define ID_DUR_LAST_RUN     	"DUR_LAST_RUN"
#define ID_SINCE_LAST_RUN   	"SINCE_LAST_RUN"

#define ID_ON_THRESH			"ON_THRESH"			// int; default=500;   min=1; max=4094;
#define ID_OFF_THRESH			"OFF_THRESH"		// int; default=10;	   min=1; max=4094;
#define ID_EXTRA_TIME			"EXTRA_TIME"		// int seconds; default=0; min=0; max=30;
#define ID_MAX_TIME				"MAX_TIME"			// int seconds; default=150;  min=0 (off); max=360	 (6 minutes)
#define ID_COOLDOWN_TIME		"COOLDOWN_TIME"		// int seconds; default=60 min=10; max=900 (15 minutes)
#define ID_IDLE_TIME			"IDLE_TIME"			// int seconds; default=3600; min=0 (off); max=84600 (1 day)
#define ID_IDLE_THRESH			"IDLE_THRESH"		// int; default=60;    min=1; max=4094;

#define ID_DRIVE_TIME_US		"DRIVE_TIME_US"		// int microseconds; default=1000; min=100; max=3000;
	// time in microseconds between turning the DRIVE on and reading the SENSOR.
	// The default is 1000 us where higher settings lead directly to higher readings.
#define ID_SAMPLE_TIME			"SAMPLE_TIME_MS"	// in ms, default=1000, min=100, max=3000
	// how often, in ms, to take a sample and add it to the circular buffer
	// This is also the maximum granularity of the pump being turned on and off
#define ID_NUM_SAMPLES			"NUM_SAMPLES"		// int; default=5; min=1; max=20
	// Number of samples in circular buffer
	// THIS SETS A "SLOP" EXTRA PUMP RUN-DRY TIME !!!

#define ID_LOG_WINDOW			"LOG_WINDOW"		// int; default=50; min=0 (off); max=100;
	// Sets a window for how far the sensor_avg must move before a log entry is created.
	// Note that pump on, extra, cooldown, and pump off already generate log entries,
	// so this is mostly used to guage the rise time, and is set to avoid slosh effects.

// USER INTERFACE

#define ID_CLEAR_ERROR			"CLEAR_ERROR"
#define ID_UI_INTERVAL			"UI_INTERVAL"		// int milliseconds; default=1000; min=1000; max=30000 (5 minutes)
#define ID_LED_BRIGHTNESS		"LED_BRIGHTNESS"

#define ID_DRAIN_HISTORY     	"DRAIN_HISTORY"
#define ID_CHART				"CHART"


// enums

#define MAX_SENSOR_SAMPLES		20

#define DRAIN_MODE_OFF			0
#define DRAIN_MODE_ON			1
#define DRAIN_MODE_FORCE		2

#define DRAIN_FLAG_FORCE		0x0001
#define DRAIN_FLAG_IDLE_RUN		0x0002
#define DRAIN_FLAG_INITIAL_RUN	0x0004
#define DRAIN_FLAG_TOO_LONG		0x0010



class aircoDevice : public myIOTDevice
{
public:

    aircoDevice() {}
    ~aircoDevice() {}

	virtual bool hasPlot() override    { return true; }

#if WITH_WEBASTO
	static String 	_airco_state_string;		// built string
	static String 	_airco_error_string;		// built string
#endif

	static String 	_drain_state_string;		// built string
	static uint32_t	_drain_mode;				// enum

	static String 	_state_last_run;			// built string
    static time_t   _time_last_run;
    static int      _since_last_run;
    static int      _dur_last_run;

	static int		_on_thresh;			// vs average of analog reads
	static int		_off_thresh;		// vs average of analog reads
	static int		_extra_time;		// seconds
	static int		_max_time;			// seconds
	static int		_cooldown_time;		// seconds

	static int		_idle_time;			// seconds
	static int		_idle_thresh;		// low threhold vs average of analog reads

	static int		_drive_time_us;		// microseconds;
	static int		_sample_time;		// milliseconds;
	static int		_num_samples;

	static int 		_log_window;		// sensor reading
	static int		_ui_interval;		// milliseconds
	static int		_led_brightness;

	static String	_drain_history_link;
	static String	_chart_link;

	static int		m_pump_on;
	static time_t 	m_run_start;
	static uint32_t m_run_flags;
	static int 		m_sensor_avg;
	static uint32_t m_flags_last_run;

#if WITH_WEBASTO
	static uint8_t	m_pcb_cond_temp;
	static uint8_t	m_pcb_intake_temp;
	static uint8_t	m_pcb_fan;
	static uint8_t	m_pan_mode;
#endif

	// implementation
	// in airco.ino

	void handlePixels();
	static void onBrightnessChanged(const myIOTValue *desc, uint32_t val);

	// in aircoData.cpp

#if WITH_WEBASTO
	void initAircoMonitor();
	void handleAircoData();
	void updateAircoUI();
	void reportAircoError(const char *msg);
#endif
	static void clearAircoError();

	// in aircoDrain.cpp

	void handleDrainPump();
	void updateDrainPumpUI();
	void pumpOn(bool on);
	void startCooldown();
	int readDrainSensor(int pin, int *avg);

	static void onDrainModeChanged(const myIOTValue *value, uint32_t val);

	// in aircoHistory.cpp

	void endDrainRun();
	void addLogRecord();
	String onCustomLink(const String &path,  const char **mime_type) override;
};



extern aircoDevice *airco;

extern bool force_pixels;
	// public to mitigate LEDs going off (?!?!) on pump/compressor power surges(?!?!)
