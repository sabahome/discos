#include "MedicinaCBandImpl.h"
#include "DevIOBandWidth.h"
#include "DevIOInitialFrequency.h"
#include "DevIOLocalOscillator.h"
#include "DevIOPolarization.h"
#include "DevIOMode.h"
#include "DevIOVacuum.h"
#include "DevIOLNAControls.h"
#include "DevIOStatus.h"
#include "DevIOComponentStatus.h"
#include <cstdio>
#include <LogFilter.h>

_IRA_LOGFILTER_DECLARE;

MedicinaCBandImpl::MedicinaCBandImpl(const ACE_CString &CompName,maci::ContainerServices *containerServices) :
	CharacteristicComponentImpl(CompName,containerServices),
	m_plocalOscillator(this),
	m_pfeeds(this),
	m_pIFs(this),
	m_pinitialFrequency(this),
	m_pbandWidth(this),
	m_ppolarization(this),
	m_pstatus(this),
	m_pvacuum(this),	
	m_pmode(this),
	m_preceiverStatus(this)
{	
	AUTO_TRACE("MedicinaCBandImpl::MedicinaCBandImpl()");
	MED_TRACE();	 
}

MedicinaCBandImpl::~MedicinaCBandImpl()
{
	AUTO_TRACE("MedicinaCBandImpl::~MedicinaCBandImpl()");
}

void MedicinaCBandImpl::initialize() throw (ACSErr::ACSbaseExImpl)
{
	MED_TRACE();
	AUTO_TRACE("MedicinaCBandImpl::initialize()");
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::initialize()",(LM_INFO,"MedicinaCBandImpl::COMPSTATE_INITIALIZING"));
	m_core.initialize(getContainerServices());
	m_monitor=NULL;
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::initialize()",(LM_INFO,"COMPSTATE_INITIALIZED"));
	 
}

void MedicinaCBandImpl::execute() throw (ACSErr::ACSbaseExImpl)
{
	 
	AUTO_TRACE("MedicinaCBandImpl::execute()");
	ACS::Time timestamp;
	const CConfiguration *config=m_core.execute(); //throw (ComponentErrors::CDBAccessExImpl,ComponentErrors::MemoryAllocationExImpl,ComponentErrors::SocketErrorExImpl)

	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::execute()",(LM_INFO,"ACTIVATING_LOG_REPETITION_FILTER"));
	_IRA_LOGFILTER_ACTIVATE(config->getRepetitionCacheTime(),config->getRepetitionExpireTime());

	try {
		m_plocalOscillator=new baci::ROdoubleSeq(getContainerServices()->getName()+":LO",getComponent(),new DevIOLocalOscillator(&m_core),true);
		m_ppolarization=new baci::ROlongSeq(getContainerServices()->getName()+":polarization",getComponent(),new DevIOPolarization(&m_core),true);
		m_pmode=new baci::ROstring(getContainerServices()->getName()+":mode",getComponent(),new DevIOMode(&m_core),true);
		m_pinitialFrequency=new baci::ROdoubleSeq(getContainerServices()->getName()+":initialFrequency",getComponent(),new DevIOInitialFrequency(&m_core),true);
		m_pbandWidth=new baci::ROdoubleSeq(getContainerServices()->getName()+":bandWidth",getComponent(),new DevIOBandWidth(&m_core),true);
		m_pIFs=new baci::ROlong(getContainerServices()->getName()+":IFs",getComponent());
		m_pfeeds=new baci::ROlong(getContainerServices()->getName()+":feeds",getComponent());
		m_pvacuum=new baci::ROdouble(getContainerServices()->getName()+":vacuum",getComponent(),new DevIOVacuum(&m_core),true);				
				// new DevIOEnvTemperature(&m_core),true); // Is there a sensor?
		m_pstatus=new baci::ROpattern(getContainerServices()->getName()+":status",getComponent(),
				new DevIOStatus(&m_core),true);
		m_preceiverStatus=new ROEnumImpl<ACS_ENUM_T(Management::TSystemStatus),POA_Management::ROTSystemStatus>
				  (getContainerServices()->getName()+":receiverStatus",getComponent(),new DevIOComponentStatus(&m_core),true);
	}
	catch (std::bad_alloc& ex) {
		_EXCPT(ComponentErrors::MemoryAllocationExImpl,dummy,"MedicinaCBandImpl::initialize()");
		throw dummy;
	}

	// write some fixed values
	m_pfeeds->getDevIO()->write(m_core.getFeeds(),timestamp);
	m_pIFs->getDevIO()->write(m_core.getIFs(),timestamp);
	m_core.setVacuumDefault(m_pvacuum->default_value());

	CComponentCore *temp=&m_core;
	try {
		m_monitor=getContainerServices()->getThreadManager()->create<CMonitorThread,CComponentCore*> (
				"WHATCHDOG7GHZ",temp,config->getWarchDogResponseTime()*10,config->getWatchDogSleepTime()*10);
	}
	catch (acsthreadErrType::acsthreadErrTypeExImpl& ex) {
		_ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl,_dummy,ex,"MedicinaCBandImpl::execute()");
		throw _dummy;
	}
	catch (...) {
		_THROW_EXCPT(ComponentErrors::UnexpectedExImpl,"MedicinaCBandImpl::execute()");
	}
	m_monitor->setLNASamplingTime(config->getLNASamplingTime());
	m_monitor->resume();
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::execute()",(LM_INFO,"WATCH_DOG_SPAWNED"));
	try {
		startPropertiesMonitoring();
	}
	catch (acsthreadErrType::CanNotStartThreadExImpl& E) {
		_ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl,__dummy,E,"MedicinaCBandImpl::execute()");
		throw __dummy;
	}
	catch (ACSErrTypeCommon::NullPointerExImpl& E) {
		_ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl,__dummy,E,"MedicinaCBandImpl::execute()");
		throw __dummy;		
	}
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::execute()",(LM_INFO,"COMPSTATE_OPERATIONAL"));
	 
}

void MedicinaCBandImpl::cleanUp()
{
	AUTO_TRACE("MedicinaCBandImpl::cleanUp()");
	stopPropertiesMonitoring();
	if (m_monitor!=NULL) {
		m_monitor->suspend();
		getContainerServices()->getThreadManager()->destroy(m_monitor);
		m_monitor=NULL;
	}
	m_core.cleanup();
	_IRA_LOGFILTER_FLUSH;
	_IRA_LOGFILTER_DESTROY;
	CharacteristicComponentImpl::cleanUp();	
}

void MedicinaCBandImpl::aboutToAbort()
{
	AUTO_TRACE("MedicinaCBandImpl::aboutToAbort()");
	if (m_monitor!=NULL) {
		getContainerServices()->getThreadManager()->destroy(m_monitor);
	}
	m_core.cleanup();
}

void MedicinaCBandImpl::activate(const char * setup_mode) throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.activate(setup_mode);
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::activate()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::deactivate() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.deactivate();
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::deactivate()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::calOn() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{	
	try {
		m_core.calOn();
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::calOn()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::calOff() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.calOff();
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::calOff()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::externalCalOn() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.externalCalOn();
    }
    catch (ComponentErrors::ComponentErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getComponentErrorsEx();        
    }
    catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getReceiversErrorsEx();
    }
    catch (...) {
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "MedicinaCBandImpl::externalCalOn()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void MedicinaCBandImpl::externalCalOff() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.externalCalOff();
    }
    catch (ComponentErrors::ComponentErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getComponentErrorsEx();        
    }
    catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getReceiversErrorsEx();
    }
    catch (...) {
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "MedicinaCBandImpl::externalCalOff()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void MedicinaCBandImpl::setLO(const ACS::doubleSeq& lo) throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.setLO(lo);
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::setLO()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::setMode(const char * mode) throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.setMode(mode);
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::setMode()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

ACS::doubleSeq *MedicinaCBandImpl::getCalibrationMark(const ACS::doubleSeq& freqs, const ACS::doubleSeq& bandwidths, const ACS::longSeq& feeds,const ACS::longSeq& ifs,
		ACS::doubleSeq_out skyFreq,ACS::doubleSeq_out skyBw,CORBA::Boolean_out onoff,CORBA::Double_out scaleFactor) throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	ACS::doubleSeq_var result=new ACS::doubleSeq;
	ACS::doubleSeq_var resFreq=new ACS::doubleSeq;
	ACS::doubleSeq_var resBw=new ACS::doubleSeq;
	try {
		m_core.getCalibrationMark(result.inout(),resFreq.inout(),resBw.inout(),freqs,bandwidths,feeds,ifs,onoff,scaleFactor);
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::getCalibrationMark()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
	skyFreq=resFreq._retn();
	skyBw=resBw._retn();
	return result._retn();
}

CORBA::Long MedicinaCBandImpl::getFeeds(ACS::doubleSeq_out X,ACS::doubleSeq_out Y,ACS::doubleSeq_out power) throw (CORBA::SystemException,
		ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	ACS::doubleSeq_var tempX=new ACS::doubleSeq;
	ACS::doubleSeq_var tempY=new ACS::doubleSeq;
	ACS::doubleSeq_var tempPower=new ACS::doubleSeq;
	long res;
	try {
		res=m_core.getFeeds(tempX.inout(),tempY.inout(),tempPower.inout());
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::getFeeds()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
	X=tempX._retn();
	Y=tempY._retn();
	power=tempPower._retn();
	return res;
}

void MedicinaCBandImpl::getIFOutput(
		const ACS::longSeq& feeds,
        const ACS::longSeq& ifs,
        ACS::doubleSeq_out freqs,
        ACS::doubleSeq_out bw,
        ACS::longSeq_out pols,
        ACS::doubleSeq_out LO
) throw (CORBA::SystemException, ComponentErrors::ComponentErrorsEx, ReceiversErrors::ReceiversErrorsEx)
{
    ACS::doubleSeq_var freqs_res = new ACS::doubleSeq;
    ACS::doubleSeq_var bw_res = new ACS::doubleSeq;
    ACS::longSeq_var pols_res = new ACS::longSeq;
    ACS::doubleSeq_var LO_res = new ACS::doubleSeq;

    try {
        m_core.getIFOutput(feeds, ifs, freqs_res.inout(), bw_res.inout(), pols_res.inout(), LO_res.inout());
    }
    catch (ComponentErrors::ComponentErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getComponentErrorsEx();
    }
    catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getReceiversErrorsEx();
    }
    catch (...) {
        _EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::getIFOutput()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
    freqs = freqs_res._retn();
    bw = bw_res._retn();
    pols = pols_res._retn();
    LO = LO_res._retn();
}

CORBA::Double MedicinaCBandImpl::getTaper(CORBA::Double freq,CORBA::Double bandWidth,CORBA::Long feed,CORBA::Long ifNumber,CORBA::Double_out waveLen) throw (
		CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	CORBA::Double res;
	double wL;
	try {
		res=(CORBA::Double)m_core.getTaper(freq,bandWidth,feed,ifNumber,wL);
		waveLen=wL;
		return res;
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();		
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::getTaper()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}	
}

void MedicinaCBandImpl::turnLNAsOn() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.lnaOn();
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::turnLNAsOn()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::turnLNAsOff() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	try {
		m_core.lnaOff();
	}
	catch (ComponentErrors::ComponentErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getComponentErrorsEx();
	}
	catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
		ex.log(LM_DEBUG);
		throw ex.getReceiversErrorsEx();
	}
	catch (...) {
		_EXCPT(ComponentErrors::UnexpectedExImpl,impl,"MedicinaCBandImpl::turnLNAsOff()");
		impl.log(LM_DEBUG);
		throw impl.getComponentErrorsEx();
	}
}

void MedicinaCBandImpl::turnAntennaUnitOn() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	//has it to be implemented?
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::turnAntennaUnitOn()",(LM_NOTICE,"CBAND_ANTENNA_UNIT_ON"));
}

void MedicinaCBandImpl::turnAntennaUnitOff() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
	//has it to be implemented?
	ACS_LOG(LM_FULL_INFO,"MedicinaCBandImpl::turnAntennaUnitOff()",(LM_NOTICE,"CBAND_ANTENNA_UNIT_OFF"));
}

_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROdoubleSeq,m_plocalOscillator,LO);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROpattern,m_pstatus,status);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROlongSeq,m_ppolarization,polarization);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROlong,m_pfeeds,feeds);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROlong,m_pIFs,IFs);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROdoubleSeq,m_pbandWidth,bandWidth);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROdoubleSeq,m_pinitialFrequency,initialFrequency);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROdouble,m_pvacuum,vacuum);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,ACS::ROstring,m_pmode,mode);
_PROPERTY_REFERENCE_CPP(MedicinaCBandImpl,Management::ROTSystemStatus,m_preceiverStatus,receiverStatus);


/* --------------- [ MACI DLL support functions ] -----------------*/
#include <maciACSComponentDefines.h>
MACI_DLL_SUPPORT_FUNCTIONS(MedicinaCBandImpl)