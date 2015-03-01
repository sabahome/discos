#include "SRTLPBandReceiverImpl.h"
#include "DevIOBandWidth.h"
#include "DevIOInitialFrequency.h"
#include "DevIOLocalOscillator.h"
#include "DevIOPolarization.h"
#include "DevIOMode.h"
#include "DevIOFeeds.h"
#include "DevIOVacuum.h"
#include "DevIOCryoTemperatureCoolHead.h"
#include "DevIOCryoTemperatureCoolHeadWindow.h"
#include "DevIOCryoTemperatureLNA.h"
#include "DevIOCryoTemperatureLNAWindow.h"
#include "DevIOEnvTemperature.h"
#include "DevIOLNAControls.h"
#include "DevIOStatus.h"
#include "DevIOComponentStatus.h"
#include <LogFilter.h>

#include <iostream>

_IRA_LOGFILTER_DECLARE;

SRTLPBandReceiverImpl::SRTLPBandReceiverImpl(const ACE_CString &CompName,maci::ContainerServices *containerServices) :
    CharacteristicComponentImpl(CompName,containerServices),
    m_plocalOscillator(this),
    m_pfeeds(this),
    m_pIFs(this),
    m_pinitialFrequency(this),
    m_pbandWidth(this),
    m_ppolarization(this),
    m_pstatus(this),
    m_pvacuum(this),
    m_pvdL1(this),
    m_pvdR1(this),
    m_pvdL2(this),
    m_pvdR2(this),
    m_pvdL3(this),
    m_pvdR3(this),
    m_pidL1(this),
    m_pidR1(this),
    m_pidL2(this),
    m_pidR2(this),
    m_pidL3(this),
    m_pidR3(this),
    m_pvgL1(this),
    m_pvgR1(this),
    m_pvgL2(this),
    m_pvgR2(this),
    m_pvgL3(this),
    m_pvgR3(this),
    m_pcryoTemperatureCoolHead(this),
    m_pcryoTemperatureCoolHeadWindow(this),
    m_pcryoTemperatureLNA(this),
    m_pcryoTemperatureLNAWindow(this),
    m_penvironmentTemperature(this),
    m_pmode(this),
    m_preceiverStatus(this),
    m_preceiverName(this)
{   
    AUTO_TRACE("SRTLPBandReceiverImpl::SRTLPBandReceiverImpl()");
}


SRTLPBandReceiverImpl::~SRTLPBandReceiverImpl()
{
    AUTO_TRACE("SRTLPBandReceiverImpl::~SRTLPBandReceiverImpl()");
}


void SRTLPBandReceiverImpl::initialize() throw (ACSErr::ACSbaseExImpl)
{
    AUTO_TRACE("SRTLPBandReceiverImpl::initialize()");
    ACS_LOG(
            LM_FULL_INFO, 
            "SRTLPBandReceiverImpl::initialize()", 
            (LM_INFO, "SRTLPBandReceiverImpl::COMPSTATE_INITIALIZING")
    );
    m_core.initialize(getContainerServices());
    m_monitor = NULL;
    ACS_LOG(LM_FULL_INFO, "SRTLPBandReceiverImpl::initialize()", (LM_INFO, "COMPSTATE_INITIALIZED"));
}


void SRTLPBandReceiverImpl::execute() throw (ACSErr::ACSbaseExImpl, ComponentErrors::MemoryAllocationExImpl)
{
    AUTO_TRACE("SRTLPBandReceiverImpl::execute()");
    ACS::Time timestamp;
    const CConfiguration *config = m_core.execute(); 

    ACS_LOG(LM_FULL_INFO, "SRTLPBandReceiverImpl::execute()",(LM_INFO, "ACTIVATING_LOG_REPETITION_FILTER"));
    _IRA_LOGFILTER_ACTIVATE(config->getRepetitionCacheTime(), config->getRepetitionExpireTime());

    try {
        m_preceiverName = new ROstring(getContainerServices()->getName() + ":receiverName", getComponent());
        m_pIFs = new baci::ROlong(getContainerServices()->getName() + ":IFs", getComponent());
        m_pfeeds = new baci::ROlong(
                getContainerServices()->getName() + ":feeds", 
                getComponent(), 
                new DevIOFeeds(&m_core), 
                true
        );
        m_pvacuum = new baci::ROdouble(
                getContainerServices()->getName() + ":vacuum", 
                getComponent(), 
                new DevIOVacuum(&m_core), 
                true
        );
        m_plocalOscillator = new baci::ROdoubleSeq(
                getContainerServices()->getName() + ":LO", 
                getComponent(),
                new DevIOLocalOscillator(&m_core),
                true
        );

        m_ppolarization = new baci::ROlongSeq(
                getContainerServices()->getName() + ":polarization", 
                getComponent(),
                new DevIOPolarization(&m_core), 
                true
        );

        m_pmode = new baci::ROstring(getContainerServices()->getName() + ":mode", getComponent(), new DevIOMode(&m_core), true);

        m_pinitialFrequency = new baci::ROdoubleSeq(
                getContainerServices()->getName() + ":initialFrequency", 
                getComponent(),
                new DevIOInitialFrequency(&m_core), 
                true
        );

        m_pbandWidth = new baci::ROdoubleSeq(
                getContainerServices()->getName() + ":bandWidth", 
                getComponent(),
                new DevIOBandWidth(&m_core), 
                true
        );

        // Drain Voltage
        m_pvdL1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdL1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 0, 1), true);
        m_pvdR1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdR1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 1, 1), true);
        m_pvdL2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdL2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 0, 2), true);
        m_pvdR2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdR2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 1, 2), true);
        m_pvdL3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdL3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 0, 3), true);
        m_pvdR3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vdR3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_VOLTAGE, 1, 3), true);

        // Drain Current
        m_pidL1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idL1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 0, 1), true);
        m_pidR1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idR1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 1, 1), true);
        m_pidL2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idL2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 0, 2), true);
        m_pidR2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idR2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 1, 2), true);
        m_pidL3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idL3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 0, 3), true);
        m_pidR3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":idR3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::DRAIN_CURRENT, 1, 3), true);

        // Gate Voltage
        m_pvgL1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgL1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 0, 1), true);
        m_pvgR1=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgR1", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 1, 1), true);
        m_pvgL2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgL2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 0, 2), true);
        m_pvgR2=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgR2", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 1, 2), true);
        m_pvgL3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgL3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 0, 3), true);
        m_pvgR3=new baci::ROdoubleSeq(getContainerServices()->getName() + ":vgR3", getComponent(),
                new DevIOLNAControls(&m_core, IRA::ReceiverControl::GATE_VOLTAGE, 1, 3), true);

        m_pcryoTemperatureCoolHead = new baci::ROdouble(getContainerServices()->getName() + ":cryoTemperatureCoolHead", \
                getComponent(), new DevIOCryoTemperatureCoolHead(&m_core),true);
        m_pcryoTemperatureCoolHeadWindow = new baci::ROdouble(getContainerServices()->getName() + ":cryoTemperatureCoolHeadWindow", \
                getComponent(), new DevIOCryoTemperatureCoolHeadWin(&m_core), true);
        m_pcryoTemperatureLNA = new baci::ROdouble(getContainerServices()->getName() + ":cryoTemperatureLNA", getComponent(), \
                new DevIOCryoTemperatureLNA(&m_core), true);
        m_pcryoTemperatureLNAWindow = new baci::ROdouble(getContainerServices()->getName() + ":cryoTemperatureLNAWindow", \
                getComponent(), new DevIOCryoTemperatureLNAWin(&m_core), true);
        m_penvironmentTemperature = new baci::ROdouble(getContainerServices()->getName() + ":environmentTemperature", \
                getComponent(), new DevIOEnvTemperature(&m_core), true);
        m_pstatus = new baci::ROpattern(getContainerServices()->getName()+":status",getComponent(), \
                new DevIOStatus(&m_core), true);
        m_preceiverStatus = new ROEnumImpl<ACS_ENUM_T(Management::TSystemStatus), POA_Management::ROTSystemStatus>
                 (getContainerServices()->getName() + ":receiverStatus", getComponent(), new DevIOComponentStatus(&m_core), true);
    }
    catch (std::bad_alloc& ex) {
        _EXCPT(ComponentErrors::MemoryAllocationExImpl, dummy, "SRTLPBandReceiverImpl::initialize()");
        throw dummy;
    }

    // Write some fixed values
    m_preceiverName->getDevIO()->write(getComponent()->getName(), timestamp);
    m_pIFs->getDevIO()->write(m_core.getIFs(), timestamp);
    m_core.setVacuumDefault(m_pvacuum->default_value());

    SRTLPBandCore *temp = &m_core;
    try {
         m_monitor = getContainerServices()->getThreadManager()->create<CMonitorThread, SRTLPBandCore*> (
                 "WHATCHDOKBANDMF", temp, config->getWarchDogResponseTime(), config->getWatchDogSleepTime());
    }
    catch (acsthreadErrType::acsthreadErrTypeExImpl& ex) {
        _ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl, _dummy, ex, "SRTLPBandReceiverImpl::execute()");
        throw _dummy;
    }
    catch (...) {
        _THROW_EXCPT(ComponentErrors::UnexpectedExImpl, "SRTLPBandReceiverImpl::execute()");
    }
    m_monitor->setLNASamplingTime(config->getLNASamplingTime());
    m_monitor->resume();
    ACS_LOG(LM_FULL_INFO, "SRTLPBandReceiverImpl::execute()", (LM_INFO, "WATCH_DOG_SPAWNED"));
    try {
        startPropertiesMonitoring();
    }
    catch (acsthreadErrType::CanNotStartThreadExImpl& E) {
        _ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl, __dummy, E, "SRTLPBandReceiverImpl::execute()");
        throw __dummy;
    }
    catch (ACSErrTypeCommon::NullPointerExImpl& E) {
        _ADD_BACKTRACE(ComponentErrors::ThreadErrorExImpl, __dummy, E, "SRTLPBandReceiverImpl::execute()");
        throw __dummy;      
    }
    ACS_LOG(LM_FULL_INFO, "SRTLPBandReceiverImpl::execute()", (LM_INFO, "COMPSTATE_OPERATIONAL"));
}


void SRTLPBandReceiverImpl::cleanUp()
{
    AUTO_TRACE("SRTLPBandReceiverImpl::cleanUp()");
    stopPropertiesMonitoring();
    if (m_monitor != NULL) {
        m_monitor->suspend();
        getContainerServices()->getThreadManager()->destroy(m_monitor);
        m_monitor = NULL;
    }
    m_core.cleanup();
    _IRA_LOGFILTER_FLUSH;
    _IRA_LOGFILTER_DESTROY;
    CharacteristicComponentImpl::cleanUp(); 
}


void SRTLPBandReceiverImpl::aboutToAbort()
{
    AUTO_TRACE("SRTLPBandReceiverImpl::aboutToAbort()");
    stopPropertiesMonitoring();
    if (m_monitor != NULL) {
        getContainerServices()->getThreadManager()->destroy(m_monitor);
    }
    m_core.cleanup();
    CharacteristicComponentImpl::aboutToAbort(); 
}


void SRTLPBandReceiverImpl::activate(const char * setup_mode) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx, 
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::activate()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}

void SRTLPBandReceiverImpl::deactivate() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
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
        _EXCPT(ComponentErrors::UnexpectedExImpl,impl,"SRTLPBandReceiverImpl::deactivate()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::calOn() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::calOn()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::calOff() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::calOff()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}

void SRTLPBandReceiverImpl::setLO(const ACS::doubleSeq& lo) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx)
{
    ; // Do nothing
}


void SRTLPBandReceiverImpl::setMode(const char * mode) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setMode()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setSetupMode(const char * setup_mode) throw (
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.setSetupMode(setup_mode);
    }
    catch (ComponentErrors::ComponentErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getComponentErrorsEx();
    }
    catch (ReceiversErrors::ReceiversErrorsExImpl& ex) {
        ex.log(LM_DEBUG);
        throw ex.getReceiversErrorsEx();
    }
}


ACS::doubleSeq * SRTLPBandReceiverImpl::getCalibrationMark(
        const ACS::doubleSeq& freqs, 
        const ACS::doubleSeq& bandwidths, 
        const ACS::longSeq& feeds,
        const ACS::longSeq& ifs,
        ACS::doubleSeq_out skyFreq,
        ACS::doubleSeq_out skyBw,
        CORBA::Double_out scaleFactor
        ) throw (
            CORBA::SystemException,
            ComponentErrors::ComponentErrorsEx,
            ReceiversErrors::ReceiversErrorsEx
        )
{
    ACS::doubleSeq_var result = new ACS::doubleSeq;
    ACS::doubleSeq_var resFreq = new ACS::doubleSeq;
    ACS::doubleSeq_var resBw = new ACS::doubleSeq;

    try {
        m_core.getCalibrationMark(result.inout(), resFreq.inout(), resBw.inout(), freqs, bandwidths, feeds,ifs,scaleFactor);
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
        _EXCPT(ComponentErrors::UnexpectedExImpl,impl,"SRTLPBandReceiverImpl::getCalibrationMark()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
    skyFreq = resFreq._retn();
    skyBw = resBw._retn();
    return result._retn();
}


void SRTLPBandReceiverImpl::getIFOutput(
            const ACS::longSeq& feeds,
            const ACS::longSeq& ifs,
            ACS::doubleSeq_out freqs,
            ACS::doubleSeq_out bw,
            ACS::longSeq_out pols,
            ACS::doubleSeq_out LO
    ) throw (CORBA::SystemException, ComponentErrors::ComponentErrorsEx, ReceiversErrors::ReceiversErrorsEx)
{
    ACS_SHORT_LOG((LM_INFO, "SRTLPBandReceiverImpl::getIFOutput()"));
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
        _EXCPT(ComponentErrors::UnexpectedExImpl,impl,"SRTLPBandReceiverImpl::getIFOutput()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }

    freqs = freqs_res._retn();
    bw = bw_res._retn();
    pols = pols_res._retn();
    LO = LO_res._retn();
}
            

CORBA::Long SRTLPBandReceiverImpl::getFeeds(
        ACS::doubleSeq_out X,
        ACS::doubleSeq_out Y,
        ACS::doubleSeq_out power
        ) throw (
            CORBA::SystemException, 
            ComponentErrors::ComponentErrorsEx,
            ReceiversErrors::ReceiversErrorsEx
          )
{
    ACS::doubleSeq_var tempX = new ACS::doubleSeq;
    ACS::doubleSeq_var tempY = new ACS::doubleSeq;
    ACS::doubleSeq_var tempPower = new ACS::doubleSeq;
    long res;
    try {
        res = m_core.getFeeds(tempX.inout(), tempY.inout(), tempPower.inout());
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::getFeeds()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
    X = tempX._retn();
    Y = tempY._retn();
    power = tempPower._retn();
    return res;
}


CORBA::Double SRTLPBandReceiverImpl::getTaper(
        CORBA::Double freq,
        CORBA::Double bandWidth,
        CORBA::Long feed,
        CORBA::Long ifNumber,
        CORBA::Double_out waveLen
        ) throw (
            CORBA::SystemException,
            ComponentErrors::ComponentErrorsEx,
            ReceiversErrors::ReceiversErrorsEx
          )
{
    CORBA::Double res;
    double wL;
    try {
        res = (CORBA::Double)m_core.getTaper(freq, bandWidth, feed, ifNumber, wL); 
        waveLen = wL;
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::getTaper()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }   
}


void SRTLPBandReceiverImpl::setLBandFilter(CORBA::Long filter_id) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.setLBandFilter(filter_id);
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setLBandFilter()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setPBandFilter(CORBA::Long filter_id) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.setPBandFilter(filter_id);
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setPBandFilter()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setLBandPolarization(const char * polarization) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.setLBandPolarization(polarization);
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setLBandPolarization()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setPBandPolarization(const char * polarization) throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.setPBandPolarization(polarization);
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setPBandPolarization()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setLBandColdLoadPath() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.setLBandColdLoadPath();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setLBandColdLoadPath()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setPBandColdLoadPath() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.setPBandColdLoadPath();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setPBandColdLoadPath()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setLBandSkyPath() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.setLBandSkyPath();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setLBandSkyPath()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::setPBandSkyPath() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{   
    try {
        m_core.setPBandSkyPath();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::setPBandSkyPath()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::turnLNAsOn() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::turnLNAsOn()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::turnLNAsOff() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::turnLNAsOff()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}

void SRTLPBandReceiverImpl::turnVacuumSensorOn() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.vacuumSensorOn();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::turnVacuumSensorOn()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}


void SRTLPBandReceiverImpl::turnVacuumSensorOff() throw (
        CORBA::SystemException,
        ComponentErrors::ComponentErrorsEx,
        ReceiversErrors::ReceiversErrorsEx
        )
{
    try {
        m_core.vacuumSensorOff();
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
        _EXCPT(ComponentErrors::UnexpectedExImpl, impl, "SRTLPBandReceiverImpl::turnVacuumSensorOff()");
        impl.log(LM_DEBUG);
        throw impl.getComponentErrorsEx();
    }
}

void SRTLPBandReceiverImpl::turnAntennaUnitOn() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
    //has it to be implemented?
    ACS_LOG(LM_FULL_INFO,"SRTLPBandReceiverImpl::turnAntennaUnitOn()",(LM_NOTICE,"KBAND_ANTENNA_UNIT_ON"));
}

void SRTLPBandReceiverImpl::turnAntennaUnitOff() throw (CORBA::SystemException,ComponentErrors::ComponentErrorsEx,ReceiversErrors::ReceiversErrorsEx)
{
    //has it to be implemented?
    ACS_LOG(LM_FULL_INFO,"SRTLPBandReceiverImpl::turnAntennaUnitOff()",(LM_NOTICE,"KBAND_ANTENNA_UNIT_OFF"));
}


_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_plocalOscillator, LO);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROpattern, m_pstatus, status);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROlongSeq, m_ppolarization, polarization);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROlong, m_pfeeds, feeds);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROlong, m_pIFs, IFs);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pbandWidth, bandWidth);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pinitialFrequency, initialFrequency);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_pvacuum, vacuum);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdL1, vdL1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdR1, vdR1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdL2, vdL2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdR2, vdR2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdL3, vdL3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvdR3, vdR3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidL1, idL1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidR1, idR1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidL2, idL2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidR2, idR2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidL3, idL3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pidR3, idR3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgL1, vgL1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgR1, vgR1);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgL2, vgL2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgR2, vgR2);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgL3, vgL3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdoubleSeq, m_pvgR3, vgR3);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_pcryoTemperatureCoolHead, cryoTemperatureCoolHead);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_pcryoTemperatureCoolHeadWindow, cryoTemperatureCoolHeadWindow);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_pcryoTemperatureLNA, cryoTemperatureLNA);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_pcryoTemperatureLNAWindow, cryoTemperatureLNAWindow);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROdouble, m_penvironmentTemperature, environmentTemperature);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROstring, m_pmode, mode);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, Management::ROTSystemStatus, m_preceiverStatus, receiverStatus);
_PROPERTY_REFERENCE_CPP(SRTLPBandReceiverImpl, ACS::ROstring, m_preceiverName, receiverName);

/* --------------- [ MACI DLL support functions ] -----------------*/
#include <maciACSComponentDefines.h>
MACI_DLL_SUPPORT_FUNCTIONS(SRTLPBandReceiverImpl)
