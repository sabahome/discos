// $Id: Configuration.cpp,v 1.4 2011-04-05 13:01:57 c.migoni Exp $

#include "Configuration.h"

using namespace IRA;
using namespace CalibrationTool_private;

#define _GET_DWORD_ATTRIBUTE(ATTRIB,DESCR,FIELD) { \
	DWORD tmpw; \
	if (!CIRATools::getDBValue(Services,ATTRIB,tmpw)) { \
		_EXCPT(ComponentErrors::CDBAccessExImpl,dummy,"CConfiguration::Init()"); \
		dummy.setFieldName(ATTRIB); \
		throw dummy; \
	} \
	else { \
		FIELD=tmpw; \
		ACS_DEBUG_PARAM("CConfiguration::Init()",DESCR" %lu",tmpw); \
	} \
}

#define _GET_STRING_ATTRIBUTE(ATTRIB,DESCR,FIELD) { \
	CString tmps; \
	if (!CIRATools::getDBValue(Services,ATTRIB,tmps)) { \
		_EXCPT(ComponentErrors::CDBAccessExImpl,dummy,"::CConfiguration::Init()"); \
		dummy.setFieldName(ATTRIB); \
		throw dummy; \
	} \
	else { \
		FIELD=tmps; \
		ACS_DEBUG_PARAM("CConfiguration::Init()",DESCR" %s",(const char*)tmps); \
	} \
}

#define _GET_DOUBLE_ATTRIBUTE(ATTRIB,DESCR,FIELD) { \
	double tmpd; \
	if (!CIRATools::getDBValue(Services,ATTRIB,tmpd)) { \
		_EXCPT(ComponentErrors::CDBAccessExImpl,dummy,"CConfiguration::Init()"); \
		dummy.setFieldName(ATTRIB); \
		throw dummy; \
	} \
	else { \
		FIELD=tmpd; \
		ACS_DEBUG_PARAM("CConfiguration::Init()",DESCR" %lf",tmpd); \
	} \
}


CConfiguration::CConfiguration()
{
}

CConfiguration::~CConfiguration()
{
}

void CConfiguration::init(maci::ContainerServices *Services) throw (ComponentErrors::CDBAccessExImpl)
{
	_GET_DWORD_ATTRIBUTE("WorkingThreadTime","Sleep time of working thread (uSec)",m_workingThreadTime);
	_GET_DWORD_ATTRIBUTE("CollectorThreadTime","Sleep time of collector thread (uSec)",m_collectorThreadTime);	
	_GET_DWORD_ATTRIBUTE("WorkingThreadTimeSlice","Time slice of working thread (uSec)",m_workingThreadTimeSlice);
	_GET_DWORD_ATTRIBUTE("RepetitionCacheTime","Log repetition filter cache time (uSec)",m_repetitionCacheTime);
	_GET_DWORD_ATTRIBUTE("RepetitionExpireTime","Log repetition filter expire time  (uSec)",m_repetitionExpireTime);
	_GET_DWORD_ATTRIBUTE("TrackingFlagDutyCycle","Tracking flag enquery gap  (uSec)",m_trackingFlagDutyCycle);
	_GET_STRING_ATTRIBUTE("AntennaBossInterface","Antenna Boss component interface is ",m_antennaBossComp);
	_GET_STRING_ATTRIBUTE("MinorServoBossInterface","Minor Servo  Boss component interface is ",m_minorServoBossComp);
	_GET_STRING_ATTRIBUTE("SchedulerInterface","Scheduler component interface is ",m_schedulerComp);
	_GET_STRING_ATTRIBUTE("ObservatoryInterface","Observatory component interface is ",m_observatoryComp);
	_GET_DWORD_ATTRIBUTE("GenerateFile","File generation enabled ",m_generateFile);
	_GET_DOUBLE_ATTRIBUTE("FwhmTolerance","FWHM tollerance is ",m_fwhmTol);
	if (m_fwhmTol!=0) {
		m_fwhmTol/=100.0;
	}
	else {
		m_fwhmTol=0.0001;
	}
	//m_workingThreadTime*=10;
	//m_workingThreadTimeSlice*=10;
	//m_collectorThreadTime*=10;
}
