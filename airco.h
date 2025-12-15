//----------------------------------------------
// constants for airco.ino
//----------------------------------------------

#pragma once

#include <myIOTDevice.h>


#define	WITH_PLOT			0
#define WITH_MEM_HISTORY  	1


//=========================================================
// pins
//=========================================================

#define PIN_RX2			16
#define PIN_TX2			17
#define PIN_LED			27


//------------------------
// myIOT definition
//------------------------
// In the worst case of both sensors constantly reading WET:
// 	in HIGH mode the pump will have a duty cycle of HIGH_TIME on, and COOLDOWN_TIME off;
// 	in LOW mode the pump will have a duty cycle of MAX_TIME on and COOLDOWN_TIME off.
// There is no provision for sensing that the air conditioner is running and the HIGH
//	SENSOR_HIGH never registers as wet.

#define AIRCO_DEVICE				"airco_device"
#define AIRCO_DEVICE_VERSION		"ac1.0"
#define AIRCO_DEVICE_URL			"https://github.com/phorton1/Arduino-boat-airco"


#define ID_STATE_STRING			"STATE_STRING"
#define ID_ERROR_STRING			"ERROR_STRING"
#define ID_CLEAR_ERROR			"CLEAR_ERROR"
#define ID_UI_INTERVAL			"UI_INTERVAL"		// int milliseconds; default=1000; min=1000; max=30000 (5 minutes)
#define ID_PLOT_INTERVAL		"PLOT_INTERVAL"		// int milliseconds; default=3000; min=1000; max=30000 (5 minutes)
#define ID_LED_BRIGHTNESS		"LED_BRIGHTNESS"
#if WITH_MEM_HISTORY
	#define ID_CHART_LINK				"CHART"
#endif


class aircoDevice : public myIOTDevice
{
public:

    aircoDevice() {}
    ~aircoDevice() {}

	static String 	_state_string;		// built string
	static String 	_error_string;		// built string
	static int		_ui_interval;		// milliseconds
	static int		_plot_interval;		// milliseconds
	static int		_led_brightness;
	
	static void clearError();
	static void onBrightnessChanged(const myIOTValue *desc, uint32_t val);

	void updateUI();
	void handleData();
	void handlePixels();
	void reportError(const char *msg);

	#if WITH_PLOT
		virtual bool hasPlot() override    { return true; }
	#endif
	#if WITH_MEM_HISTORY
		String onCustomLink(const String &path,  const char **mime_type) override;
		static String _chart_link;
			// note that extra_data/airco_chart.html must be uploaded to SPIFFS by hand
	#endif
};



extern aircoDevice *airco;





