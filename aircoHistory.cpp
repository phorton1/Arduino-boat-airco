//-----------------------------------------------------------------------
// aircoHistory.ino
//-----------------------------------------------------------------------

#include "airco.h"

#include <myIOTLog.h>
#include <myIOTWebServer.h>
#include <myIOTDataLog.h>


extern myIOTDataLog data_log;
	// defined in airco.ino

typedef struct
{
	uint32_t	dt;
	float		intake;			// cyan
	float		condensor;		// blue
	uint32_t	fan;			// green
	uint32_t	compressor;		// orange
	uint32_t	mode;			// gray
} aircoHistory_t;


static String series_colors = "[ \"#00aaaa\" , \"#0000ff\",\"#00ff00\",\"#ff8800\",\"#888888\" ]";



void addHistoryRecord(uint8_t mode, uint8_t fan, uint8_t intake, uint8_t cond)
	// extern'd in airco.ino
{
	aircoHistory_t log_rec;
	log_rec.intake = intake - 40;
	log_rec.condensor = cond - 40;
	log_rec.mode = mode;

	uint8_t speed_enum = fan & 0x70;	// 0x00=off; 0x40=low; 0x20=med; 0x10=high
	bool	user = fan & 0x80;			// as opposed to 'auto'

	uint32_t speed =
		speed_enum == 0x40 ? 100 :		// low
		speed_enum == 0x20 ? 200 :		// medium
		speed_enum == 0x10 ? 300 :		// high
		0;								// off
	if (!user) speed += 20;				// bump for auto

	log_rec.fan = speed;
	log_rec.compressor = fan & 0xf;
	data_log.addRecord((logRecord_t) &log_rec);
}


String aircoDevice::onCustomLink(const String &path,  const char **mime_type)
	// called from myIOTHTTP.cpp::handleRequest()
	// for any paths that start with /custom/
{
	LOGD("aircoDevice::onCustomLink(%s)",path.c_str());
	if (path.startsWith("chart_html"))
	{
		int period = myiot_web_server->getArg("period",86400);	// day default
		return data_log.getChartHTML(period,true);
			// true == with_degrees
	}
	else if (path.startsWith("chart_header"))
	{
		*mime_type = "application/json";
		return data_log.getChartHeader(&series_colors);
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
	return "";
}



