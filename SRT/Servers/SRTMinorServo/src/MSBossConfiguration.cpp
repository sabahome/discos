#include "MSBossConfiguration.h"
#include "utils.h"

using namespace IRA;
using namespace std;

MSBossConfiguration::MSBossConfiguration(maci::ContainerServices *Services)
{
    m_actualSetup = "unknown";
    m_commandedSetup = "";
    m_isConfigured = false;
    m_isStarting = false;
    m_isParking = false;
    m_isElevationTracking = false;
    m_dynamic_comps.clear();
    m_services = Services;
}


MSBossConfiguration::~MSBossConfiguration() {}


void MSBossConfiguration::init(string setupMode) throw (ManagementErrors::ConfigurationErrorEx)
{
    // Starting
    m_isStarting = true;
    m_commandedSetup = string(setupMode);

    // Read the component configuration
    IRA::CString config;
    if(!CIRATools::getDBValue(m_services, setupMode.c_str(), config)) {
        m_isStarting = false;
        THROW_EX(ManagementErrors, ConfigurationErrorEx, setupMode + ": unavailable configuration code.", true);
    }

    // initializing
    m_isConfigured = false;
    m_servosToPark.clear();
    m_servosToMove.clear();
    m_servosCoefficients.clear();
    m_dynamic_comps.clear();
    m_isValidCDBConfiguration = false;

    vector<string> actions = split(string(config), actions_separator);

    // Get the actions
    for(vector<string>::iterator iter = actions.begin(); iter != actions.end(); iter++) {
        // Split the action in items.
        vector<string> items = split(*iter, items_separator);

        // Get the name of component 
        string comp_name = items.front();
        strip(comp_name);

        // Get the component
        IRA::CError error;
        try {
            MinorServo::WPServo_var component_ref = MinorServo::WPServo::_nil();
            component_ref = m_services->getComponent<MinorServo::WPServo>(("MINORSERVO/" + comp_name).c_str());
            if(!CORBA::is_nil(component_ref))
                m_component_refs[comp_name] = component_ref;
            else {
                m_isStarting = false;
                THROW_EX(ManagementErrors, ConfigurationErrorEx, ("Cannot get component " + comp_name).c_str(), true);
            }
        }
        catch (maciErrType::CannotGetComponentExImpl& ex) {
            m_isStarting = false;
            THROW_EX(ManagementErrors, ConfigurationErrorEx, ("Cannot get the reference of " + comp_name).c_str(), true);
        }

        // Get the action
        string action(items.back());
        strip(action);

        if(startswith(action, "none") || startswith(action, "NONE")) {
            continue;
        }

        if(startswith(action, "park") || startswith(action, "PARK")) {
            m_servosToPark.push_back(comp_name);
            continue;
        }

        m_servosToMove.push_back(comp_name);
        vector<double> position_values;
        vector<string> aitems = split(action, coeffs_separator);
        vector<string> values;
        for(vector<string>::iterator viter = aitems.begin(); viter != aitems.end(); viter++) {
            vector<string> labels = split(*viter, coeffs_id);
            string coeffs = labels.back();
            strip(coeffs);

            for(string::size_type idx = 0; idx != boundary_tokens.size(); idx++)
                strip(coeffs, char2string(boundary_tokens[idx]));

            values = split(coeffs, pos_separator);
            vector<double> coefficients;
            for(vector<string>::size_type idx = 0; idx != values.size(); idx++) {
                coefficients.push_back(str2double(values[idx]));
            }

            if(coefficients.empty()) {
                m_isStarting = false;
                THROW_EX(ManagementErrors, ConfigurationErrorEx, ("No coefficients for " + comp_name).c_str(), true);
            }

            (m_servosCoefficients[comp_name]).push_back(coefficients);
            if(coefficients.size() > 1)
                m_dynamic_comps.push_back(comp_name);
        }

        if(m_servosCoefficients.count(comp_name) && m_component_refs.count(comp_name)) {
            if((m_servosCoefficients[comp_name]).size() != (m_component_refs[comp_name])->numberOfAxes()) {
                m_isStarting = false;
                THROW_EX(ManagementErrors, ConfigurationErrorEx, "Mismatch between number of coefficients and number of axes", true);
            }
        }
    }
    m_isValidCDBConfiguration = true;
}



ACS::doubleSeq MSBossConfiguration::getPosition(string comp_name, double elevation)
    throw (ManagementErrors::ConfigurationErrorExImpl)
{
    ACS::doubleSeq positions;
    if(m_servosCoefficients.count(comp_name)) {
        vector<vector<double> > vcoeff = m_servosCoefficients[comp_name]; 
        positions.length(vcoeff.size());
        size_t position_idx(0);
        for(size_t j=0; j != vcoeff.size(); j++) {
            double axis_value(0);
            // For instance if an axis has the following coefficients: [C0, C1, C2], the axis value will be:
            // C0 + C1*E + C2*E^2
            for(size_t idx = 0; idx != (vcoeff[j]).size(); idx++)
                axis_value += (vcoeff[j])[idx] * pow(elevation, idx);

            positions[position_idx] = axis_value;
            position_idx++;
        }
    }
    else {
        THROW_EX(ManagementErrors, ConfigurationErrorEx, comp_name + "has no coefficients", true);
    }

    return positions;
}
