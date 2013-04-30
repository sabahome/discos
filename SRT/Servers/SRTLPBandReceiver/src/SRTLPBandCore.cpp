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
        ReceiversErrors::ReceiverControlBoardErrorExImpl,
        ComponentErrors::ValidationErrorExImpl,
        ComponentErrors::ValueOutofRangeExImpl,
        ComponentErrors::CouldntGetComponentExImpl,
        ComponentErrors::CORBAProblemExImpl,
        ReceiversErrors::LocalOscillatorErrorExImpl
        )
{
    baci::ThreadSyncGuard guard(&m_mutex);
    m_setupMode = ""; // If we don't reach the end of the method then the mode will be unknown
    IRA::CString cmdMode(mode);
	cmdMode.MakeUpper();

    _EXCPT(ReceiversErrors::ModeErrorExImpl, impl, "CConfiguration::setMode()");
    
    /*
    // Set the operating mode to the board
    try {
        if(cmdMode == "SINGLEDISH")
                m_control->setSingleDishMode();
        else
            if(cmdMode == "VLBI")
                m_control->setVLBIMode();
            else 
                throw impl; // If the mode is not supported, raise an exception
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl,impl, "SRTLPBandCore::setMode()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
    */

    m_configuration.setMode(cmdMode);

    /* TODO! Fix the code below

    for (WORD i=0;i<m_configuration.getIFs();i++) {
        m_startFreq[i]=m_configuration.getIFMin()[i];
        m_bandwidth[i]=m_configuration.getIFBandwidth()[i];
        m_polarization[i]=(long)m_configuration.getPolarizations()[i];
    }
    // The set the default LO for the default LO for the selected mode.....
    ACS::doubleSeq lo;
    lo.length(m_configuration.getIFs());
    for (WORD i=0;i<m_configuration.getIFs();i++) {
        lo[i]=m_configuration.getDefaultLO()[i];
    }
    // setLO throws:
    //     ComponentErrors::ValidationErrorExImpl,
    //     ComponentErrors::ValueOutofRangeExImpl,
    //     ComponentErrors::CouldntGetComponentExImpl,
    //     ComponentErrors::CORBAProblemExImpl,
    //     ReceiversErrors::LocalOscillatorErrorExImpl
    // setLO(lo); 

    // Verify the m_setupMode is the same mode active on the board
    bool isSingleDishModeOn, isVLBIModeOn;
    try {
        isSingleDishModeOn = m_control->isSingleDishModeOn();
        isVLBIModeOn = m_control->isVLBIModeOn();
    }
    catch (IRA::ReceiverControlEx& ex) {
        _EXCPT(ReceiversErrors::ReceiverControlBoardErrorExImpl,impl, "SRTLPBandCore::setMode()");
        impl.setDetails(ex.what().c_str());
        setStatusBit(CONNECTIONERROR);
        throw impl;
    }
    if((cmdMode == "SINGLEDISH" && !isSingleDishModeOn) || (cmdMode == "VLBI" && !isVLBIModeOn)) {
        m_setupMode = ""; // If m_setupMode doesn't match the mode active on the board, then set un unknown mode
        throw impl;
    }

    m_setupMode = cmdMode;
    ACS_LOG(LM_FULL_INFO,"CComponentCore::setMode()",(LM_NOTICE,"RECEIVER_MODE %s",mode));
    */
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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
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

    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    

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

    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    

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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    
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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    
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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    
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
    // TODO: decomment this code when we will use the real receiver
    // if (checkStatusBit(LOCAL)) {
    //     _EXCPT(ReceiversErrors::NoRemoteControlErrorExImpl, impl, "SRTLPBandComponentCore::setLBandFilter()");
    //     throw impl;
    // }
    
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

