//-----------------------------------------------------------------------
// aircoHistory.ino
//-----------------------------------------------------------------------
#include "airco.h"
#include <myIOTLog.h>
#include <myIOTWebServer.h>
#include <myIOTDataLog.h>
#if WITH_SD
	#include <SD.h>
	#include <FS.h>
#endif


#define DRAIN_HISTORY_FILE	"/drain_history.data"


typedef struct
{
	uint32_t	dt;
	#if WITH_WEBASTO
		float		intake;			// cyan
		float		condensor;		// magenta
		uint32_t	fan;			// green
		uint32_t	compressor;		// orange
		uint32_t	mode;			// gray
	#endif
	uint32_t	sensor;			// blue
	uint32_t	pump_on;		// red
} 	aircoData_t;


typedef struct {
	time_t 		start;		// starting time
	uint16_t 	dur;		// duration cannot be larger than 65534 (like 18 hours)
	uint16_t	flags;		// uint32_t error_code enum compressed into a word
}	drainHistory_t;			// 8 bytes




logColumn_t  airco_cols[] = {
	#if WITH_WEBASTO
		{"intake",		LOG_COL_TYPE_TEMPERATURE,	10,	},
		{"condensor",	LOG_COL_TYPE_TEMPERATURE,	10,	},
		{"fan",			LOG_COL_TYPE_UINT32,		100,},
		{"compressor",	LOG_COL_TYPE_UINT32,		10,	},
		{"mode",		LOG_COL_TYPE_UINT32,		100,},
	#endif
	{"sensor",		LOG_COL_TYPE_UINT32,		100,},
	{"pump",		LOG_COL_TYPE_UINT32,		1,  },
};

#if WITH_WEBASTO
	myIOTDataLog data_log("aircoData",7,airco_cols,0);
		// 0 = debug_send_data level
#else
	myIOTDataLog data_log("drainData",2,airco_cols,0);
		// 0 = debug_send_data level
#endif

static String series_colors = "[ \"#00aaaa\",\"#ff00ff\",\"#00ff00\",\"#ff8800\",\"#888888\",\"#0000ff\",\"#ff0000\" ]";




static String flagsToString(uint32_t flags, bool forHTML)
{
	String text;
	if (flags & DRAIN_FLAG_FORCE)
		text = "FORCE";
	else
	{
		if (flags & DRAIN_FLAG_IDLE_RUN)
			text = "IDLE_RUN";
		if (flags & DRAIN_FLAG_TOO_LONG)
		{
			if (text != "") text += " ";
			text += "TOO_LONG";
		}
	}
	if (text == "")
	{
		if (forHTML)
			text += "&nbsp;";
		else
			text += "NORMAL";
	}
	return text;
}



void aircoDevice::endDrainRun()
{
	time_t time_now = time(NULL);
	time_t elapsed = time_now - m_run_start;
	LOGD("endRun elapsed(%u)",elapsed);

    #if WITH_SD
		if (myIOTDevice::hasSD())
		{
			drainHistory_t hist_rec;
			hist_rec.start = time_now;
			hist_rec.dur = elapsed;
			hist_rec.flags = m_run_flags;
			File file = SD.open(DRAIN_HISTORY_FILE, FILE_APPEND);
			if (!file)
			{
				LOGE("Could not open %s for APPEND",DRAIN_HISTORY_FILE);
			}
			else
			{
				file.write((const uint8_t*) &hist_rec,sizeof(drainHistory_t));
				file.close();
			}
		}
	#endif

	setString(ID_STATE_LAST_RUN,flagsToString(m_run_flags,false));
	setTime(ID_TIME_LAST_RUN,m_run_start);
	setInt(ID_SINCE_LAST_RUN,m_run_start+elapsed);
    setInt(ID_DUR_LAST_RUN,elapsed);

	m_flags_last_run = m_run_flags;
	m_run_flags = 0;
	m_run_start = 0;
	addLogRecord();
}


void aircoDevice::addLogRecord()
{
	aircoData_t log_rec;

#if WITH_WEBASTO
	log_rec.intake = m_pcb_intake_temp - 40;
	log_rec.condensor = m_pcb_cond_temp - 40;
	log_rec.mode = m_pan_mode;

	uint8_t speed_enum = m_pcb_fan & 0x70;	// 0x00=off; 0x40=low; 0x20=med; 0x10=high
	bool	user = m_pcb_fan & 0x80;			// as opposed to 'auto'

	uint32_t speed =
		speed_enum == 0x40 ? 100 :		// low
		speed_enum == 0x20 ? 200 :		// medium
		speed_enum == 0x10 ? 300 :		// high
		0;								// off
	if (!user) speed += 20;				// bump for auto

	log_rec.fan = speed;
	log_rec.compressor = m_pcb_fan & 0xf;
#endif

	log_rec.sensor = m_sensor_avg;
	log_rec.pump_on = m_pump_on;
	
	data_log.addRecord((logRecord_t) &log_rec);
}



//---------------------------------------------------
// Drain History HTML
//---------------------------------------------------


static String durString(uint32_t since)
{
	char buf[128] = "&nbsp;";
	int days = since / (24 * 60 * 60);
	int hours = ((since % (24 * 60 * 60)) / (60 * 60)) % 24;
	int minutes = ((since % (24 * 60 * 60 * 60)) / 60) % 60;
	int secs = since % 60;
	if (days)
	{
		sprintf(buf,"%d days  %02d:%02d:%02d",days, hours, minutes, secs);
	}
	else
	{
		sprintf(buf,"%02d:%02d:%02d",hours, minutes, secs);
	}
	return String(buf);
}



static bool sendRecordHTML(int num,time_t time_now,const drainHistory_t *rec, const drainHistory_t *next_rec)
	// Note that the "ago" is since the run ENDED, to be consistent
	// with the interval between runs, which is from when the this one ENDED
	// till the next one STARTED
{
	String text = "";
	String dur = "";

	text += "<tr><td>";
	text += String(num);
	text += "</td><td>";
	text += timeToString(rec->start);
	text += "</td><td align='center'>";
	text += String(rec->dur);
	text += "</td><td>";
	text += durString(time_now - rec->start - rec->dur);	// since it ENDED
	text += "</td><td>";
	if (next_rec->start)
	{
		uint32_t interval = next_rec->start - (rec->start + rec->dur);
		text += durString(interval);
	}
	else
	{
		text += "&nbsp;";
	}

	// FLAGS

	text += "</td><td align='center'>";
	text += flagsToString(rec->flags,true);
	text += "</td></tr>";
	if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
		return false;
	
	return true;
}



static String buildHTML()
	// Only called with at least one valid entry.
	// Head can only be zero if we have wrapped (i.e. whole buffer is full)
	// 	so, if the head happens to be zero, then we start at the end of the array.
	// Start at the one before the head.
	// We pull off the head/tail values in case a run is going on and ends
	// 	while this is called.
	// We are done when we've completed the tail entry.
{
    if (!myiot_web_server->startBinaryResponse("text/html", CONTENT_LENGTH_UNKNOWN))
        return "";

	File file = SD.open(DRAIN_HISTORY_FILE, FILE_READ);

    String text = "<head>\n";
    text += "<title>";
    text += airco->getName();
    text += " History</title>\n";
    text += "<body>\n";
    text += "<style>\n";
    text += "th, td { padding-left: 12px; padding-right: 12px; }\n";
    text += "</style>\n";
	text += "<b>";
	text +=  airco->getName();

	if (!file)
	{
		LOGE("Could not open %s for READ", DRAIN_HISTORY_FILE);
		text += "<br>File ";
		text += DRAIN_HISTORY_FILE;
		text += "Not found";
		myiot_web_server->writeBinaryData(text.c_str(),text.length());
		return RESPONSE_HANDLED;
	}

	size_t recSize = sizeof(drainHistory_t);
	size_t fileSize = file.size();
	size_t count = fileSize / recSize;
    LOGD("buildHTML() found %d records in %s",count,DRAIN_HISTORY_FILE);

	if (!count)
	{
		LOGW("No records found in %s", DRAIN_HISTORY_FILE);
		text += "<br>";
		text += DRAIN_HISTORY_FILE;
		text += "is empty!!";
		myiot_web_server->writeBinaryData(text.c_str(),text.length());
		return RESPONSE_HANDLED;
	}

	text += " History Items</b><br><br>\n";
	text += "<table border='1' padding='6' style='border-collapse:collapse'>\n";
	text += "<tr><th>num</th><th>time</th><th>dur</th><th>ago</th></th><th>interval</th><th>flags</th></tr>\n";
	if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
		return "";

	// Loop through records

	time_t time_now = time(NULL);
	drainHistory_t rec;
	drainHistory_t next_rec;
	next_rec.start = 0;		// init "next" dt to zero for identification

	for (int i=count-1; i>=0; i--)
	{
		size_t pos = i * recSize;
		file.seek(pos);
		file.read((uint8_t*)&rec, recSize);
		if (!sendRecordHTML(i+1,time_now,&rec,&next_rec))
			return "";
		memcpy(&next_rec,&rec,recSize);
	}

	file.close();

    text = "</table>";
    text += "</body>\n";
    if (!myiot_web_server->writeBinaryData(text.c_str(),text.length()))
        return "";

    myiot_web_server->finishBinaryResponse();
    LOGD("getHistoryHTML() sent %d records",count);
    return RESPONSE_HANDLED;
}




//---------------------------------------------------------
// onCustomLink()
//---------------------------------------------------------


String aircoDevice::onCustomLink(const String &path,  const char **mime_type)
	// called from myIOTHTTP.cpp::handleRequest()
	// for any paths that start with /custom/
{

	String data_name = "";
	if (myiot_web_server->hasArg("data_name"))
		data_name = myiot_web_server->arg("data_name");

	LOGD("aircoDevice::onCustomLink(%s) data_name='%s'",path.c_str(),data_name.c_str());

	if (path.startsWith("chart_header"))
	{
		*mime_type = "application/json";
		return data_log.getChartHeader(86400,1,&series_colors);
			// 84600 = default_period = day
			// 1 = with_degrees
			// uses device specific series colors
	}
	else if (path.startsWith("chart_data"))
	{
		uint32_t secs = myiot_web_server->getArg("secs",0);
		return data_log.sendChartData(secs);
	}
	else if (path.startsWith("update_chart_data"))
	{
		uint32_t since = myiot_web_server->getArg("since",0);
		return data_log.sendChartData(since,true);
	}
	// HTML History

    else if (path.startsWith("getDrainHistory"))
		return buildHTML();

	return "";
}
