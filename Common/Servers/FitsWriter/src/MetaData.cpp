#include "MetaData.h"

using namespace FitsWriter_private;

CMetaData::CMetaData()
{
	m_receiverCode="";
	m_feeds=NULL;
	m_feedNumber=0;
	m_receiversIFID.length(0);
	m_backendAttenuations.length(0);
	m_sectionsID.length(0);
	m_skyFrequency.length(0);
	m_skyBandwidth.length(0);
	m_localOscillator.length(0);
	m_calibrationMarks.length(0);
	m_calibrationMarks.length(0);
	m_receiverPolarization.length(0);
	m_sourceName="";
	m_sourceRa=m_sourceDec=m_sourceVlsr=0.0;
	m_sourceFlux.length(0);
	m_dewarMode=Receivers::RCV_UNDEF_DEROTCONF;
	m_dewarPos=0.0;
	m_servoAxisNames.length(0);
	m_servoAxisNames.length(0);
	m_restFreq.length(0);
	m_sysAzOff=m_sysElOff=m_lonOff=m_latOff=0.0;
	m_offFrame=Antenna::ANT_HORIZONTAL;
	//m_azOff=m_elOff=m_raOff=m_decOff=m_lonOff=m_latOff=0.0;
	m_subScanConf.signal=Management::MNG_SIGNAL_NONE;
}
	
CMetaData::~CMetaData()
{
	if (m_feeds!=NULL) {
		delete [] m_feeds;
		m_feeds=NULL;
	}
}

void CMetaData::saveFeedHeader(CFitsWriter::TFeedHeader * fH,const WORD& number)
{
	if (m_feeds!=NULL) {
		delete [] m_feeds;
		m_feedNumber=0;
	}
	m_feeds=fH;
	m_feedNumber=number;
}

/*************** Private ***********************************************************/





