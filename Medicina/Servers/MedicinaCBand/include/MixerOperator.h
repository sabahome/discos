#ifndef MED_C_BAND_MIXER_OP_H
#define MED_C_BAND_MIXER_OP_H

#include <IRA>
#include <ReceiverControl.h>
#include <LocalOscillatorInterfaceC.h>
#include <ReceiversErrors.h>
#include <ManagmentDefinitionsC.h>
#include "Configuration.h"

/**
 * @brief This class enclose both mixer stages into a unique interface
 */
class MixerOperator{

    public:

        /**
         * @brief Default Ctor
         * @param p_config Component's config ref
         * @param p_services 
         */
        MixerOperator(CConfiguration & p_config );

        /**
         * @brief Destroy the Mixer Operator object
         */
        ~MixerOperator();

			/**
			* @brief Set services
			*/
			void setServices(maci::ContainerServices * p_services);

			/**
			* @brief
			*/
		 	bool isLoaded()const;

        /**
         * @brief Mixer Setup
         * @details devices initialization, set default freq
         * @param[in] p_name_1st 1st LO component's name
         * @param[in] p_name_2nd 2nd LO component's name
         * @param[in] p_defaults Default mixer values
         * @param[in] p_mins Minimal values 
         * @param[in] p_defaults Maximum values
         */
        void setup(const IRA::CString p_name_1st,
                    const IRA::CString p_name_2nd,
                    ACS::doubleSeq p_defaults, 
                    ACS::doubleSeq p_mins,
                    ACS::doubleSeq p_maxs);

        /**
         * @brief Loading LOs components         
         */
        void loadComponents() throw (ComponentErrors::CouldntGetComponentExImpl);

        /**
         * @brief Freeing associate LOs components
         * 
         */
        void releaseComponents();

        /**
         * @brief Set LO equivalent value to the 2 stages LOs
         * @param p_values Values for what? why an array?
         * @return true Value is valid         
         */
        bool setValue(const ACS::doubleSeq& p_values)
                          throw (ComponentErrors::ValidationErrorExImpl,
                                ComponentErrors::ValueOutofRangeExImpl,
                                ComponentErrors::CouldntGetComponentExImpl,
                                ComponentErrors::CORBAProblemExImpl,
                                ReceiversErrors::LocalOscillatorErrorExImpl);

        /**
         * @brief Get the equivalent LO value 
         * @return double LO value
         */
        double getValue();

        /**
         * @brief Resurce lock check
         * 
         * @return true Resurce is locked         
         */
        bool isLocked();

    private:

        /**
         * @brief Single Lo device loading by instance name
         * @details Throw exceptions
         * @param[inout] p_loDev LOs device object
         * @param[in] p_lo_instance LO name to be loaded 
         */
        void loadDevice(Receivers::LocalOscillator_var p_loDev,
                        const char * p_lo_name);

        /**
         * @brief Realse LO device by isntance name
         * 
         * @param p_loDev instance container
         * @param p_lo_name instance name
         */
        void releaseDevice(Receivers::LocalOscillator_var p_loDev,
                        const char * p_lo_name);

			/**
			* @brief Single component lock check			 
			* @param p_loDev Component instance
			* @param p_lo_name Component instance name
			* @return true id Device is locked
			*/
			bool isDeviceLocked(Receivers::LocalOscillator_var p_loDev,
										const char* p_lo_name);
			
    private:

        CConfiguration & m_configuration;   /**< Reference to component's config */
		  maci::ContainerServices * m_services; /**< maci service */
        bool m_init_ok; /**< Init part completed flag */
        bool m_mixer_fault; /**< Flag indicating some faults on LO devices */
        double m_current_value; /**< memo current value seen from user */
        Receivers::LocalOscillator_var m_loDev_1st; /**< First LO component */
        Receivers::LocalOscillator_var m_loDev_2nd; /**< Second LO component */          
};

#endif