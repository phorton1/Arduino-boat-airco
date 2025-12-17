//-----------------------------------------------------------------------
// aircoHistory.ino
//-----------------------------------------------------------------------

#include "airco.h"

#include <myIOTLog.h>
#include <myIOTWebServer.h>
#include <myIOTDataLog.h>


extern myIOTDataLog airco_data_log;
	// defined in airco.ino


#define NUM_MEM_RECS	3000		// 28K
	// The airco cycles about every 10 minutes eating about 30 records.


typedef struct		// in memory record = 8 bytes per record
{
	uint32_t	dt;
	uint8_t		mode;	// raw PANEL control mode
	uint8_t		fan;	// raw PCB fan byte
	uint8_t		intake;	// raw (bias 40) intake temperature
	uint8_t		cond;	// raw (bias 40) condensor temperature
} memHistory_t;


// expanded record matches chart columns
// note that series are drawn in ordeer and
// we want the temperatures on top, so this
// order is reversed from the raw bytes

typedef struct
{
	uint32_t	dt;
	float		intake;			// cyan
	float		condensor;		// blue
	uint32_t	fan;			// green
	uint32_t	compressor;		// orange
	uint32_t	mode;			// gray
} sendHistory_t;


memHistory_t history[NUM_MEM_RECS];
volatile int hist_head;
volatile int hist_tail;

static String series_colors = "[ \"#00aaaa\" , \"#0000ff\",\"#00ff00\",\"#ff8800\",\"#888888\" ]";



void addHistoryRecord(uint8_t mode, uint8_t fan, uint8_t intake, uint8_t cond)
	// extern'd in airco.cpp
{
	int new_head = hist_head + 1;
	if (new_head >= NUM_MEM_RECS)
		new_head = 0;
	if (hist_tail == new_head)
	{
		hist_tail++;
		if (hist_tail >= NUM_MEM_RECS)
			hist_tail = 0;
	}

	memHistory_t *hist = &history[hist_head];
	hist->dt = time(NULL);
	hist->mode 	 = mode;
	hist->fan 	 = fan;
	hist->intake = intake;
	hist->cond	 = cond;
	hist_head = new_head;

	LOGV("addHistoryRecord(%d)",hist_head);
}


static bool sendOne(uint32_t cutoff, memHistory_t *in_rec)
{
	if (in_rec->dt >= cutoff)
	{
		sendHistory_t out_rec;
		out_rec.dt = in_rec->dt;

		out_rec.intake = in_rec->intake - 40;
		out_rec.condensor = in_rec->cond - 40;
		out_rec.mode = in_rec->mode;

		uint8_t fan = in_rec->fan;
		uint8_t speed_enum = fan & 0x70;	// 0x00=off; 0x40=low; 0x20=med; 0x10=high
		bool	user = fan & 0x80;	// as opposed to 'auto'

		uint32_t speed =
			speed_enum == 0x40 ? 100 :		// low
			speed_enum == 0x20 ? 200 :		// medium
			speed_enum == 0x10 ? 300 :		// high
			0;								// off
		if (!user) speed += 20;				// bump for auto
			
		out_rec.fan = speed;
		out_rec.compressor = fan & 0xf;
			// known values are 0x0 & 0x8

		if (!myiot_web_server->writeBinaryData((const char*)&out_rec, sizeof(sendHistory_t)))
			return false;
	}
	return true;
}



String aircoDevice::onCustomLink(const String &path,  const char **mime_type)
	// called from myIOTHTTP.cpp::handleRequest()
	// for any paths that start with /custom/
{
	LOGD("aircoDevice::onCustomLink(%s)",path.c_str());
	if (path.startsWith("chart_html/aircoData"))
	{
		// only used by airco_chart.html inasmuch as the
		// chart html is baked into the myIOT widget
		int height = myiot_web_server->getArg("height",400);
		int width  = myiot_web_server->getArg("width",800);
		int period = myiot_web_server->getArg("period",86400);	// day default
		int refresh = myiot_web_server->getArg("refresh",0);
		return airco_data_log.getChartHTML(height,width,period,refresh);
	}
	else if (path.startsWith("chart_header/aircoData"))
	{
		*mime_type = "application/json";
		return airco_data_log.getChartHeader(&series_colors);
	}
	else if (path.startsWith("chart_data/aircoData"))
	{
		uint32_t secs = myiot_web_server->getArg("secs",0);
		uint32_t cutoff = time(NULL) - secs;

		int tail = hist_tail;
		int head = hist_head;

		if (!myiot_web_server->startBinaryResponse("application/octet-stream", CONTENT_LENGTH_UNKNOWN))
			return "";

		if (head < tail)
		{
			while (tail < NUM_MEM_RECS)
			{
				if (!sendOne(cutoff,&history[tail++]))
					return "";
			}
			tail = 0;
		}
		while (tail < head)
		{
			if (!sendOne(cutoff,&history[tail++]))
				return "";
		}

		return RESPONSE_HANDLED;
	}

	return "";
}






