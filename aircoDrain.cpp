//-----------------------------------------------------------------------
// aircoDrain.ino
//-----------------------------------------------------------------------
// handle the drainPump portion of the airco device.
//
// The sensor is sampled once per second into a circular buffer.
// Use the Plot function to see once per second state changes.
//
// There are several user parameters that control the sensor:
//
//		- DRIVE_ON_TIME
//			time in microseconds between turning the DRIVE on and reading the SENSOR.
//			The default is 1000 us where higher settings lead directly to higher readings.
//		- SAMPLE_TIME, how often, in ms, to take a sample and add it to the circular buffer
//			The default is 1000ms (once per second)
//			This is also how often the pump may turn on or off.
//		- NUM_RISE_SAMPLES, how many samples to filter on rising values
//			Used when the pump is off to compare to ON_THRESH to start the pump,
//			Default=30 to smooth out sloshing etc
//      - NUM_FALL_SAMPLES = how many samples to filter on falling value
//			Default=5 also sets a "slop" extra pump run time
//
// With regards to charting, since all PUMP_ON and PUMP_OFFS cause a log record to
// be written, this will result, at a minimum, of straight lines to the ON_THRESH
// and OFF_THRESH.  However, more detail can be provided in the chart according
// to the following params:
//
//		- LOG_OFFINTERVAL;  default=60 seconds;	  0=off; how often to log sensor values when the pump is OFF
//		- LOG_ONINTERVAL;	default=1000 ms;	  0=off; how often to log sensor values when the pump is ON or in COOLDOWN mode
//
//	These values allow one to see the intermediate rise of the water level, how
//  the pump empties the container and a certain detail of backflow, at the expense
//  of taking more room in the datalog.

#include "airco.h"
#include <myIOTLog.h>

//---------------------------------
// static state machine variables
//---------------------------------

enum PumpState { PUMP_OFF, PUMP_ON, PUMP_EXTRA,	PUMP_COOLDOWN, PUMP_ILLEGAL };

static const char *pump_state_names[] = { "PUMP_OFF", "PUMP_ON", "PUMP_EXTRA", "COOLDOWN" };

static PumpState pump_state = PUMP_OFF;

static uint32_t last_ui = 0;
static uint32_t last_sample_time = 0;
static uint32_t pump_start = 0;
static uint32_t extra_end = 0;
static uint32_t cooldown_end = 0;
static bool add_log_record = false;


// static int 		m_sensor_avg = 0;

static int circ_buf[MAX_SENSOR_SAMPLES] = {0};
static int circ_ptr = 0;

static PumpState last_pump_state = PUMP_ILLEGAL;
static int 		last_pump_on = -1;
static int 		last_sensor_avg = -1;
static uint32_t last_run_flags = 0xFFFF;


//-------------------------------------------
// myIOT implementation
//-------------------------------------------

void aircoDevice::pumpOn(bool on)
{
	LOGI("pumpON(%d)",on);
	pinMode(PIN_PUMP,OUTPUT);
	digitalWrite(PIN_PUMP,on);
	m_pump_on = on;
}


static void initChange()
{
	// re-initialize state machine on any mode changes

	last_ui = 0;
	last_sample_time = 0;
	pump_start = 0;
	cooldown_end = 0;
	extra_end = 0;

	last_pump_on = -1;
	last_sensor_avg = -1;
	last_pump_state = PUMP_ILLEGAL;
	last_run_flags = 0xFFFF;

	airco->m_pump_on = 0;
	airco->m_run_flags = 0;
	airco->m_sensor_avg = 0;

	airco->pumpOn(0);
	airco->updateDrainPumpUI();
}


void aircoDevice::onDrainModeChanged(const myIOTValue *value, uint32_t val)
	// called BEFORE the member variable has changed
	// asynchrounous to loop(); since this takes some time,
{
	LOGU("onDrainModeChanged(%d)",val);
	if (m_pump_on)
		airco->endDrainRun();
	airco->_drain_mode = val;
	initChange();
}




static void addString(String &s, const char *text, bool cond)
{
	if (cond)
	{
		if (s != "") s += " ";
		s += text;
	}
}


void aircoDevice::updateDrainPumpUI()
{
	if (last_pump_on		!= m_pump_on	||
		last_sensor_avg	    != m_sensor_avg	||
		last_pump_state	    != pump_state	||
		last_run_flags		!= m_run_flags )
	{
		last_pump_on		= m_pump_on;
		last_sensor_avg	    = m_sensor_avg;
		last_pump_state	    = pump_state;
		last_run_flags		= m_run_flags;

		String flag_string;
		addString(flag_string,"FORCE",m_run_flags & DRAIN_FLAG_FORCE);
		addString(flag_string,"IDLE",m_run_flags & DRAIN_FLAG_IDLE_RUN);
		addString(flag_string,"TOO_LONG",m_run_flags & DRAIN_FLAG_TOO_LONG);

		static char buf[255];
		sprintf(buf,"%s m_pump_on(%d) sensor=%d %s",
			pump_state_names[(int)pump_state],
			m_pump_on,
			m_sensor_avg,
			flag_string.c_str());

		setString(ID_DRAIN_STATE,buf);
	}
}



//-----------------------------------------------------------------------
// handlePump()
//-----------------------------------------------------------------------

void aircoDevice::startCooldown()
{
	LOGI("COOLDOWN %d seconds",_cooldown_time);
	pumpOn(0);
	cooldown_end = millis() + (_cooldown_time * 1000);
	endDrainRun();
	pump_state = PUMP_COOLDOWN;
	updateDrainPumpUI();
	add_log_record = true;
}


void aircoDevice::handleDrainPump()
{
    uint32_t now = millis();
    time_t time_now = time(NULL);

	//------------------------------------------------------
	// let drainHistory.cpp decide on encoding
	//------------------------------------------------------

	if (add_log_record)
	{
		add_log_record = false;
		addLogRecord();
	}

    //--------------------------------------
    // Sample sensors on interval
    //--------------------------------------

    if (now - last_sample_time >= _sample_time)
    {
		//--------------------------------------
		// SAMPLE
		//--------------------------------------

        last_sample_time = now;
		digitalWrite(PIN_SDRIVE, HIGH);         		// drive rod to 3.3 V
		delayMicroseconds(_drive_time_us);              // excite for 1ms
			// higher delays here lead directly to higher readings.
			// 1000 ms leads to approx 0..1000
		volatile int tmp = analogRead(PIN_SENSOR);	// discard first read
		int cur = analogRead(PIN_SENSOR);               // keep second read
		digitalWrite(PIN_SDRIVE, LOW);          		// stop drive

		// add to circular buffer

		int head = circ_ptr;
		circ_buf[circ_ptr++] = cur;
		if (circ_ptr == MAX_SENSOR_SAMPLES) circ_ptr = 0;

		// sum and average circular buffer based on NUM_OFF/ON_SAMPLES

		int sum = 0;
		for (int i=0; i<_num_samples; i++)
		{
			sum += circ_buf[head--];
			if (head < 0) head = MAX_SENSOR_SAMPLES - 1;
		}
		m_sensor_avg = sum / _num_samples;


		//--------------------------------------
		// PLOT
		//--------------------------------------

		LOGV("drain_mode(%d) state(%d) flags(%d) on(%d) avg(%d) cur(%d)",
			 _drain_mode,
			 pump_state,
			 m_run_flags,
			 m_pump_on,
			 m_sensor_avg,
			 cur );

		// if myIOTDevice::_plot_data is ON,
		// created and broadcast the json

		if (_plot_data)
		{
			// static const char *plot_legend = "cur,avg,pump";
			static char plot_buf[255];

			sprintf(plot_buf,"{\"plot_data\":[%d,%d,%d]}",
				cur,
				m_sensor_avg,
				m_pump_on*300);
			wsBroadcast(plot_buf);
		}

		//--------------------------------------
		// LOG
		//--------------------------------------
		// ONLY if a _log_window is defined
		// log it if the sensor crossed fully into a new window

		static int cur_window = 0;
		int middle = cur_window * _log_window + _log_window/2;

		if (m_sensor_avg >= middle + _log_window)
		{
			cur_window++;
			add_log_record = true;
		}
		else if (m_sensor_avg <= middle - _log_window)
		{
			cur_window--;
			add_log_record = true;
		}

    }	// _sample_time


    //--------------------------------------
    // Periodic UI update
    //--------------------------------------

	if (now - last_ui >= _ui_interval)
	{
        last_ui = now;
        updateDrainPumpUI();
	}

	//---------------------------
	// short returns
	//---------------------------
	
	if (_drain_mode == DRAIN_MODE_OFF)
	{
		if (m_pump_on)
		{
			LOGU("PUMP OFF due to DRAIN_MODE_OFF");
			pumpOn(0);
			endDrainRun();
			m_run_flags = 0;
			updateDrainPumpUI();
			add_log_record = true;
		}
		return;
	}

    if (_drain_mode == DRAIN_MODE_FORCE)
	{
		if (!m_pump_on)
		{
			LOGU("PUMP ON due to FORCE");
			pumpOn(1);
			m_run_start = time_now;
			m_run_flags |= DRAIN_FLAG_FORCE;
			updateDrainPumpUI();
			add_log_record = true;
		}
		return;
	}


    //--------------------------------------
    // STATE MACHINE
    //--------------------------------------
	// Note that we clear the pending error when starting
	// a new run or they propogate indefinitely.

    switch (pump_state)
    {
        case PUMP_OFF:
			if (m_sensor_avg >= _on_thresh)
            {
				LOGU("PUMP ON_THRESH(%d) >= %d",m_sensor_avg,_on_thresh);
                pumpOn(1);
                pump_start = now;
                pump_state = PUMP_ON;
				m_run_flags = 0;
				m_run_start = time_now;
				updateDrainPumpUI();
				add_log_record = true;
            }
            else if (_idle_time && _time_last_run &&
				time_now - _time_last_run > _idle_time &&
				m_sensor_avg >= _idle_thresh)
            {
				LOGU("PUMP ON IDLE(%d) >= %d",m_sensor_avg,_idle_thresh);
                pumpOn(1);
                pump_start = now;
                pump_state = PUMP_ON;
				m_run_flags = DRAIN_FLAG_IDLE_RUN;
				m_run_start = time_now;
				updateDrainPumpUI();
				add_log_record = true;
            }
            break;

        case PUMP_ON:
			if (m_sensor_avg < _off_thresh)
			{
				LOGU("PUMP OFF_THRESH(%d) < %d",m_sensor_avg,_off_thresh);
				if (_extra_time)
				{
					LOGI("EXTRA_TIME %d seconds",_extra_time);
					pump_state = PUMP_EXTRA;
					extra_end = now + (_extra_time * 1000);
				}
				else
				{
					startCooldown();
				}
			}
			else if (now - pump_start >= (_max_time * 1000))
			{
				int dur = (now - pump_start)/1000;
				LOGE("PUMP OFF MAX_TIME dur(%d) >= max(%d)",dur,_max_time);
				m_run_flags |= DRAIN_FLAG_TOO_LONG;
				startCooldown();
			}
            break;

        case PUMP_EXTRA:
            if (now >= extra_end)
            {
				LOGU("EXTRA time complete");
				startCooldown();
            }
			break;
        case PUMP_COOLDOWN:
            if (now >= cooldown_end)
            {
				LOGU("COOLDOWN time complete");
                pump_state = PUMP_OFF;
				updateDrainPumpUI();
				add_log_record = true;
            }
            break;
    }

}	// handleDrainPump()


// end of aircoDrain.cpp



