#include "SRTLPBandCore.h"

#define NUMBER_OF_STAGES 3 // Amplification stages

enum Feed {LBAND_FEED, PBAND_FEED};

SRTLPBandCore::SRTLPBandCore() {}

SRTLPBandCore::~SRTLPBandCore() {}


void SRTLPBandCore::initialize(maci::ContainerServices* services)
{
    m_vdStageValues = std::vector<IRA::ReceiverControl::StageValues>(NUMBER_OF_STAGES);
    m_idStageValues = std::vector<IRA::ReceiverControl::StageValues>(NUMBER_OF_STAGES);
    m_vgStageValues = std::vector<IRA::ReceiverControl::StageValues>(NUMBER_OF_STAGES);

    CComponentCore::initialize(services);
}


ACS::doubleSeq SRTLPBandCore::getStageValues(const IRA::ReceiverControl::FetValue& control, DWORD ifs, DWORD stage)
{
    baci::ThreadSyncGuard guard(&m_mutex);

    ACS::doubleSeq values;
    values.length(getFeeds());
    for(size_t i=0; i<getFeeds(); i++)
        values[i] = 0.0;
    

    if (ifs >= m_configuration.getIFs() || stage > NUMBER_OF_STAGES || stage < 1)
        return values;

    // Left Channel
    if(ifs == (long)Receivers::RCV_LCP) {
        if (control == IRA::ReceiverControl::DRAIN_VOLTAGE) {
            for(size_t i=0; i<getFeeds(); i++)
                values[i] = (m_vdStageValues[stage-1]).left_channel[i];
        }
        else {
            if (control == IRA::ReceiverControl::DRAIN_CURRENT) {
                for(size_t i=0; i<getFeeds(); i++)
                    values[i] = (m_idStageValues[stage-1]).left_channel[i];
            }
            else {
                for(size_t i=0; i<getFeeds(); i++)
                    values[i] = (m_vgStageValues[stage-1]).left_channel[i];
            }
        }
    }

    // Right Channel
    if (ifs == (long)Receivers::RCV_RCP) {
        if (control==IRA::ReceiverControl::DRAIN_VOLTAGE)
            for(size_t i=0; i<getFeeds(); i++)
                values[i] = (m_vdStageValues[stage-1]).right_channel[i];
        else 
            if (control == IRA::ReceiverControl::DRAIN_CURRENT) 
                for(size_t i=0; i<getFeeds(); i++)
                    values[i] = (m_idStageValues[stage-1]).right_channel[i];
            else 
                for(size_t i=0; i<getFeeds(); i++)
                    values[i] = (m_vgStageValues[stage-1]).right_channel[i];
    }

    return values;
}


void SRTLPBandCore::setMode(const char * mode) throw (
        ReceiversErrors::ModeErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ComponentErrors::ValueOutofRangeExImpl,
        ComponentErrors::CouldntGetComponentExImpl,
        ComponentErrors::CORBAProblemExImpl,
        ReceiversErrors::LocalOscillatorErrorExImpl
    )
{
    baci::ThreadSyncGuard guard(&m_mutex);
    IRA::CString cmdMode(mode);
    cmdMode.MakeUpper();
    IRA::CString setupMode = getSetupMode();

    if(setupMode.IsEmpty()) {
        _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode(): setup mode not set");
    }

    IRA::CString feed0, feed1;
    try {
        feed0 = setupMode[0];
        feed1 = setupMode[1];
    }
    catch (...) {
        _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode()");
    }


    if(feed0 == feed1) { // Single feed
        if((feed0 == "L" && cmdMode.Left() != "X") || (feed0 == "P" && cmdMode.Right() != "X"))
            _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode(): mismatch with the setup mode");
    }
    else { // Dual Feed
        if(cmdMode.Find('X') != -1)
            _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode(): mismatch with the setup mode");
    }

    // In case of L band
    if(feed0 == "L" || feed1 == "L") {
        std::vector<double> rfMin = m_configuration.getLBandRFMinFromMode(cmdMode);
        std::vector<double> rfMax = m_configuration.getLBandRFMaxFromMode(cmdMode);
        ACS::doubleSeq lo;
        getLBandLO(lo);
        // Verify the LO value does not fall inside the mode band
        for(size_t i=0; i<getIFs(); i++) {
            if(lo[i] >= rfMin[i] && lo[i] <= rfMax[i]) {
                _EXCPT(ReceiversErrors::ModeErrorExImpl, 
                       impl, 
                       "SRTLPBandCore:setMode(): " \
                       "mode not allowed. The LO frequency value is within the band and "\
                       "might generate strong aliasing.");
                throw impl;
            }
        }
        // Verify the Mode does not move the IF over the filter
        for(size_t i=0; i<getIFs(); i++) {
            if((rfMin[i] - lo[i]) >= m_configuration.getLowpassFilterMax()[0]) {
                _EXCPT(ReceiversErrors::ModeErrorExImpl, 
                       impl, 
                       "SRTLPBandCore:setMode(): " \
                       "mode not allowed. The IF bandwidth is outside the low pass filter.");
                throw impl;
            }
        }
    }

    try {
        m_configuration.setMode(cmdMode);
    }
    catch (...) {
        _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode(): cannot set the conf mode");
    }

    try {
        setLBandFilter(m_configuration.getLBandFilterID());
        setPBandFilter(m_configuration.getPBandFilterID());
        setLBandPolarization(m_configuration.getLBandPolarization());
        setPBandPolarization(m_configuration.getPBandPolarization());
    }
    catch(...) {
        _THROW_EXCPT(ReceiversErrors::ModeErrorExImpl, "SRTLPBandCore::setMode(): cannot set the filter");
    }
        
    m_actualMode = m_configuration.getActualMode();
    ACS_LOG(LM_FULL_INFO,"CComponentCore::setMode()",(LM_NOTICE,"RECEIVER_MODE %s", string(m_actualMode).c_str()));
}


void SRTLPBandCore::updateVdLNAControls() throw (ReceiversErrors::ReceiverControlBoardErrorExImpl)
{
    // Not under the mutex protection because the m_control object is thread safe (at the micro controller board stage)
    try {
        for(size_t i=0; i<NUMBER_OF_STAGES; i++)
            m_vdStageValues[i] = m_control->stageValues(IRA::ReceiverControl::DRAIN_VOLTAGE, i+1, SRTLPBandCore::voltageConverter);
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl,impl, "SRTLPBandCore::updateVdLNAControls()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
    clearStatusBit(CONNECTIONERROR); // The communication was ok so clear the CONNECTIONERROR bit
}


void SRTLPBandCore::updateIdLNAControls() throw (ReceiversErrors::ReceiverControlBoardErrorExImpl)
{
    // Not under the mutex protection because the m_control object is thread safe (at the micro controller board stage)
    try {
        for(size_t i=0; i<NUMBER_OF_STAGES; i++)
            m_idStageValues[i] = m_control->stageValues(IRA::ReceiverControl::DRAIN_CURRENT, i+1, SRTLPBandCore::currentConverter);
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandCore::updateIdLNAControls()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
    clearStatusBit(CONNECTIONERROR); // The communication was ok so clear the CONNECTIONERROR bit
}


void SRTLPBandCore::updateVgLNAControls() throw (ReceiversErrors::ReceiverControlBoardErrorExImpl)
{
    // Not under the mutex protection because the m_control object is thread safe (at the micro controller board stage)
    try {
        for(size_t i=0; i<NUMBER_OF_STAGES; i++)
            m_vgStageValues[i] = m_control->stageValues(IRA::ReceiverControl::GATE_VOLTAGE, i+1, SRTLPBandCore::voltageConverter);
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandCore::updateVgLNAControls()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
    clearStatusBit(CONNECTIONERROR); // The communication was ok so clear the CONNECTIONERROR bit
}


void SRTLPBandCore::setLBandFilter(long filter_id) throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }

    try {
        std::vector<BYTE> parameters;
        switch (filter_id) {
            case 1: // 1300-1800 MHz (All Band)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x02);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x03);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x04); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x00);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x01);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x04);
                // TODO: do I turn the antenna unit OFF?
                break;

            case 2: // 1320-1780 MHz 
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x02);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x03); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x03);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x03); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x00);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x02); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x01);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x02);
                // TODO: do I turn the antenna unit OFF?
                break;

            case 3: // 1350-1450 MHz (VLBI)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x02);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x04); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x03);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x01); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x00);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x04); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x01);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01);
                // TODO: turn the antenna unit ON
                break;

            case 4: // 1300-1800 MHz (band-pass filter)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x02);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x05); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x03);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x06); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x00);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x05); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x01);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x06);
                // TODO: do I turn the antenna unit OFF?
                break;

            case 5: // 1625-1715 MHz (VLBI)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x02);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x06); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x03);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x05); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x00);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x06); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x01);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x05);
                // TODO: turn the antenna unit ON
                break;

            default:
                _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
                impl.setDetails("Unknown filter id.");
                throw impl;
        }

        m_control->setPath(
                MCB_CMD_DATA_TYPE_STRUCT, // Data type
                MCB_PORT_TYPE_IIC,        // Port type
                MCB_PORT_NUMBER_00,       // Port number
                6,   // Width
                parameters 
                );
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setPBandFilter(long filter_id) throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }

    try {
        std::vector<BYTE> parameters;
        switch (filter_id) {
            case 1: // 305-410 MHz (All Band)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x06);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x06); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x07);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x05); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x04);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x06); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x05);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x05);
                break;
                  
            case 2: // 310-350 MHz 
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x06);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x04); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x07);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x01); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x04);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x04); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x05);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01);
                break;
 
            case 3: // 305-410 MHz (band-pass filter)
                parameters.push_back(0x00); // Begin first switch, left channel
                parameters.push_back(0x06);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01); // End firs switch, left channel
                parameters.push_back(0x00); // Begin second switch, left channel
                parameters.push_back(0x07);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00); 
                parameters.push_back(0x04); // End second switch, left channel
                parameters.push_back(0x00); // Begin first switch, right channel
                parameters.push_back(0x04);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x01); // End first switch, right channel
                parameters.push_back(0x00); // Begin second switch, right channel
                parameters.push_back(0x05);
                parameters.push_back(0x00);
                parameters.push_back(0x60);
                parameters.push_back(0x00);
                parameters.push_back(0x04);
                break;

            default:
                _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandFilter()");
                impl.setDetails("Unknown filter id.");
                throw impl;
        }

        m_control->setPath(
                MCB_CMD_DATA_TYPE_STRUCT, // Data type
                MCB_PORT_TYPE_IIC,        // Port type
                MCB_PORT_NUMBER_00,       // Port number
                6,   // Width
                parameters 
                );
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandFilter()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setLBandPolarization(const char * p) throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{

    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    

    IRA::CString polarization(p);
    polarization.MakeUpper();

    try {
        std::vector<BYTE> parameters;
        if (polarization == "C") {
            parameters.push_back(0x00); // Begin first switch, left channel
            parameters.push_back(0x01);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40); // End firs switch, left channel
            parameters.push_back(0x00); // Begin second switch, left channel
            parameters.push_back(0x00);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00); 
            parameters.push_back(0x40); // End second switch, left channel
            parameters.push_back(0x00); // Begin first switch, right channel
            parameters.push_back(0x03);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40); // End first switch, right channel
            parameters.push_back(0x00); // Begin second switch, right channel
            parameters.push_back(0x02);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40);
        }
        else if (polarization == "L") {
            parameters.push_back(0x00); // Begin first switch, left channel
            parameters.push_back(0x01);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30); // End firs switch, left channel
            parameters.push_back(0x00); // Begin second switch, left channel
            parameters.push_back(0x00);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00); 
            parameters.push_back(0x30); // End second switch, left channel
            parameters.push_back(0x00); // Begin first switch, right channel
            parameters.push_back(0x03);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30); // End first switch, right channel
            parameters.push_back(0x00); // Begin second switch, right channel
            parameters.push_back(0x02);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30);
        }
        else {
            _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setLBandPolarization()");
            impl.setDetails("Unknown polarization code.");
            throw impl;
        }

        m_control->setPath(
                MCB_CMD_DATA_TYPE_STRUCT, // Data type
                MCB_PORT_TYPE_IIC,        // Port type
                MCB_PORT_NUMBER_00,       // Port number
                6,   // Width
                parameters 
                );
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandFilter()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setPBandPolarization(const char * p) throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{

    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    

    IRA::CString polarization(p);
    polarization.MakeUpper();

    try {
        std::vector<BYTE> parameters;
        if (polarization == "C") {
            parameters.push_back(0x00); // Begin first switch, left channel
            parameters.push_back(0x04);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40); // End firs switch, left channel
            parameters.push_back(0x00); // Begin second switch, left channel
            parameters.push_back(0x05);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00); 
            parameters.push_back(0x40); // End second switch, left channel
            parameters.push_back(0x00); // Begin first switch, right channel
            parameters.push_back(0x06);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40); // End first switch, right channel
            parameters.push_back(0x00); // Begin second switch, right channel
            parameters.push_back(0x07);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x40);
        }
        else if (polarization == "L") {
            parameters.push_back(0x00); // Begin first switch, left channel
            parameters.push_back(0x04);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30); // End firs switch, left channel
            parameters.push_back(0x00); // Begin second switch, left channel
            parameters.push_back(0x05);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00); 
            parameters.push_back(0x30); // End second switch, left channel
            parameters.push_back(0x00); // Begin first switch, right channel
            parameters.push_back(0x06);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30); // End first switch, right channel
            parameters.push_back(0x00); // Begin second switch, right channel
            parameters.push_back(0x07);
            parameters.push_back(0x00);
            parameters.push_back(0x60);
            parameters.push_back(0x00);
            parameters.push_back(0x30);
        }
        else {
            _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandPolarization()");
            impl.setDetails("Unknown polarization code.");
            throw impl;
        }
        m_control->setPath(
                MCB_CMD_DATA_TYPE_STRUCT, // Data type
                MCB_PORT_TYPE_IIC,        // Port type
                MCB_PORT_NUMBER_00,       // Port number
                6,   // Width
                parameters 
                );
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandFilter()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setLBandColdLoadPath() throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    
    try {
        m_control->setColdLoadPath(0); // The L band feed has ID 0
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setColdLoadPath()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setPBandColdLoadPath() throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    
    try {
        m_control->setColdLoadPath(1); // The P band feed has ID 1
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setColdLoadPath()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


void SRTLPBandCore::setLBandSkyPath() throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    
    try {
        m_control->setSkyPath(0); // The L band feed has ID 0
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setLBandSkyPath()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}



void SRTLPBandCore::setPBandSkyPath() throw (
        ReceiversErrors::NoRemoteControlErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ReceiversErrors::ReceiverControlBoardErrorExImpl
        )
{
    if (checkStatusBit(LOCAL)) {
        _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
        throw impl;
    }
    
    try {
        m_control->setSkyPath(1); // The P band feed has ID 1
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl, impl, "SRTLPBandComponentCore::setPBandSkyPath()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
}


