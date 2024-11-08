#ifndef __H__MBFitsManager__
#define __H__MBFitsManager__

#include "Baseband.h"
#include "FitsWriter.h"
#include "MBFitsWriterTable.h"
#include "Scan.h"
#include "SpectralAxis.h"

#include <map>
#include <string>

using std::map;
using std::pair;
using std::string;

class MBFitsManager {
	public:
		typedef vector<string>							FeBe_v_t;
		typedef FeBe_v_t::iterator					FeBe_i_v_t;
		typedef FeBe_v_t::const_iterator		FeBe_ci_v_t;

		typedef vector<double>							Double_v_t;
		typedef Double_v_t::iterator				Double_i_v_t;
		typedef Double_v_t::const_iterator	Double_ci_v_t;

		typedef vector<float>								Float_v_t;
		typedef Float_v_t::iterator					Float_i_v_t;
		typedef Float_v_t::const_iterator		Float_ci_v_t;

		typedef vector<int>									Int_v_t;
		typedef Int_v_t::iterator						Int_i_v_t;
		typedef Int_v_t::const_iterator			Int_ci_v_t;

		typedef vector<long>								Long_v_t;
		typedef Long_v_t::iterator					Long_i_v_t;
		typedef Long_v_t::const_iterator		Long_ci_v_t;

		typedef vector<string>							String_v_t;
		typedef String_v_t::iterator				String_i_v_t;
		typedef String_v_t::const_iterator	String_ci_v_t;

		static double deg2Rad( const double value_ );
		static double rad2Deg( const double value_ );

		static string	getLastVersion();

		MBFitsManager( const string& mbFitsVersion_ );
		virtual ~MBFitsManager();

		bool isScanStarted() const;

		/**
		 * Get the path.
		 *  \return the path
		 */
		string getPath() const;

		/**
		 * Set the path of the file
		 * @param path_ contains the path
		 */
		void setPath( const string& path_ );

		void startScan( const string&											telescop_,
										const string&											origin_,
										const string&											creator_,

//										const string&											comment_,

//										const string&											hierarchEsoDprCatg_,
//										const string&											hierarchEsoDprType_,
//										const string&											hierarchEsoDprTech_,
//										const string&											hierarchEsoObsProgID_,
//										const int													hierarchEsoObsID_,
//										const double											hierarchEsoTelAirmStart_,
//										const double											hierarchEsoTelAirmEnd_,
//										const double											hierarchEsoTelAlt_,
//										const double											hierarchEsoTelAz_,

										const string&											instrume_,
										const MBFitsManager::String_v_t&	febes_,
										const MBFitsManager::Double_v_t&	restFreqencies_,
										const MBFitsManager::String_v_t&	lines_,
										const MBFitsManager::Double_v_t&	bandwidths_,
										const string&											object_,
										const string&											timeSys_,
										const double											mjdObs_,
										const string&											dateObs_,
										const double											mjd_,
										const double											ra_,
										const double											dec_,
										const string&											radeSys_,
										const float												equinox_,
										const double											exptime_,
										const string&											origFile_,
										const string&											projID_,
										const int													scanNum_,

										const double											siteLong_,
										const double											siteLat_,
										const float												siteElev_,
										const float												diameter_,
										const string&											obsID_,
										const double											lst_,
										const double											utc2Ut1_,
										const double											tai2Utc_,
										const double											etUtc_,
										const double											gpsTai_,
										const string&											wcsName_,
										const string&											cType1_,
										const string&											cType2_,
										const double											crVal1_,
										const double											crVal2_,
										const string&											cType1n_,
										const string&											cType2n_,
										const double											lonPole_,
										const double											latPole_,
										const double											bLongObj_,
										const double											bLatObj_,
										const double											longObj_,
										const double											latObj_,
										const double											patLong_,
										const double											patLat_,
										const string&											calCode_,
										const bool												moveFram_,
										const double											periDate_,
										const double											periDist_,
										const double											longAsc_,
										const double											omega_,
										const double											inclinat_,
										const double											eccentr_,
										const double											orbEpoch_,
										const double											orbEqnox_,
										const double											distance_,
										const Scan&												scan_,
										const double											tranDist_,
										const double											tranFreq_,
										const double											tranFocu_,
										const bool												wobUsed_,
										const double											wobThrow_,
										const string&											wobDir_,
										const float												wobCycle_,
										const string&											wobMode_,
										const string&											wobPatt_,
										const MBFitsManager::String_v_t&	phases_,
										const int													nFebe_,
										const float												pDeltaIA_,
										const float												pDeltaCA_,
										const float												pDeltaIE_,
										const float												fDeltaIA_,
										const float												fDeltaCA_,
										const float												fDeltaIE_,
										const float												ia_,
										const float												ie_,
										const float												hasa_,
										const float												haca_,
										const float												hese_,
										const float												hece_,
										const float												hesa_,
										const float												hasa2_,
										const float												haca2_,
										const float												hesa2_,
										const float												heca2_,
										const float												haca3_,
										const float												heca3_,
										const float												hesa3_,
										const float												npae_,
										const float												ca_,
										const float												an_,
										const float												aw_,
										const float												hece2_,
										const float												hece6_,
										const float												hesa4_,
										const float												hesa5_,
										const float												hsca_,
										const float												hsca2_,
										const float												hssa3_,
										const float												hsca5_,
										const float												nrx_,
										const float												nry_,
										const float												hysa_,
										const float												hyse_,
										const double											setLinX_,
										const double											setLinY_,
										const double											setLinZ_,
										const double											setRotX_,
										const double											setRotY_,
										const double											setRotZ_,
										const double											moveFoc_,
										const double											focAmp_,
										const double											focFreq_,
										const double											focPhase_,

										const string&											dewCabin_,
										const string&											dewRtMod_,
										const float												dewUser_,
										const float												dewZero_,
										const string&											location_,
										const string&											optPath_,
										const int													nOptRefl_,
										const int													febeBand_,
										const int													febeFeed_,
										const int													nUseBand_,
										const string&											fdTypCod_,
										const float												feGain_,
										const string&											swtchMod_,
										const int													nPhases_,
										const float												frThrwLo_,
										const float												frThrwHi_,
										const float												tBlank_,
										const float												tSync_,
										const float												iaRx_,
										const float												ieRx_,
										const float												hasaRx_,
										const float												hacaRx_,
										const float												heseRx_,
										const float												heceRx_,
										const float												hesaRx_,
										const float												hasa2Rx_,
										const float												haca2Rx_,
										const float												hesa2Rx_,
										const float												heca2Rx_,
										const float												haca3Rx_,
										const float												heca3Rx_,
										const float												hesa3Rx_,
										const float												npaeRx_,
										const float												caRx_,
										const float												anRx_,
										const float												awRx_,
										const float												hece2Rx_,
										const float												hece6Rx_,
										const float												hesa4Rx_,
										const float												hesa5Rx_,
										const float												hscaRx_,
										const float												hsca2Rx_,
										const float												hssa3Rx_,
										const float												hsca5Rx_,
										const float												nRxRx_,
										const float												nRyRx_,
										const string&											sigOnln_,
										const string&											refOnln_,
										const string&											sigPol_,
										const string&											refPol_,

										const MBFitsManager::Long_v_t&		useBand_,
										const MBFitsManager::Long_v_t&		nUseFeed_,
										const MBFitsManager::Long_v_t&		useFeed_,
										const MBFitsManager::Long_v_t&		beSects_,
										const MBFitsManager::Long_v_t&		feedType_,
										const MBFitsManager::Double_v_t&	feedOffX_,
										const MBFitsManager::Double_v_t&	feedOffY_,
										const int													refFeed_,
										const string&											polTy_,
										const MBFitsManager::Float_v_t&		polA_,
										const MBFitsManager::Float_v_t&		aperEff_,
										const MBFitsManager::Float_v_t&		beamEff_,
										const MBFitsManager::Float_v_t&		etafss_,
										const MBFitsManager::Float_v_t&		hpbw_,
										const MBFitsManager::Float_v_t&		antGain_,
										const MBFitsManager::Float_v_t&		tCal_,
										const MBFitsManager::Float_v_t&		bolCalFc_,
										const MBFitsManager::Float_v_t&		beGain_,
										const MBFitsManager::Float_v_t&		flatFiel_,
										const MBFitsManager::Float_v_t&		boldCoff_,
										const MBFitsManager::Float_v_t&		gainImag_,
										const MBFitsManager::Float_v_t&		gainEle1_,
										const MBFitsManager::Float_v_t&		gainEle2_,
										const string& rxBa_40_,
										const string& rxBa_80_,
										const string& rxCh_40_,
										const string& rxCh_80_,
										const string& rxHor_40_,
										const string& rxHor_80_ );

		void endScan();

		void startSubScan( const MBFitsManager::FeBe_v_t& febeNames_,
											 const Baseband::Baseband_s_t&	baseBands_,
											 const int							scanNum_,
											 const int							subSNum_,
//											 const int							obsNum_,
											 const string&					dateObs_,
											 const string&					usrFrame_,

											 const double						lst_,
											 const string&					subsType_,
//											 const string&					obsType_,

											 const Scan&						scan_,

//											 const string&					cType1n_,
//											 const string&					cType2n_,

											 const bool							dpBlock_,
											 const int 							nInts_,
											 const bool							wobCoord_,
											 const MBFitsManager::String_v_t&	phaseN_,
											 const float						dewAng_,
											 const float						dewExtra_,

											 const int							channels_,
											 const double						freqRes_,
											 const string&					molecule_,
											 const string&					transiti_,
											 const double						restFreq_,
											 const string&					sideBand_,
											 const double						sbSep_,
											 const string&					_2ctyp2_,
											 const int							_2crpx2_,
											 const int							_2crvl2_,
											 const int							_21cd2a_,

											 // Next we assign to the specral axis a frequency and velocity description relative to the rest frame
											 // But other alternatives axes descriptions could also be given
											 const SpectralAxis& spectralAxisRestFrameMainSideband_,
											 const SpectralAxis& spectralAxisRestFrameImageSideband_,
											 const SpectralAxis& spectralAxisSkyFrameMainSideband_,
											 const SpectralAxis& spectralAxisSkyFrameImageSideband_,
											 const SpectralAxis& spectralAxisRestFrameVelocity_,

											 const float						_1vsou2r_,
											 const float						_1vsys2r_ );

		void endSubScan( MBFitsManager::FeBe_v_t frontendBackendNames_ );

		void integration( const string&										febeName_,
											const long											basebandID_,
											const double										mjd_,
											const MBFitsManager::Float_v_t&	data_ );

		void integrationParameters( const string& febeName_,
																const double	mjd_,
																const double	lst_,
																const double	integTim_,
																const int			phase_,
																const double	longOff_,
																const double	latOff_,
																const double	azimuth_,
																const double	elevatio_,
																const double	ra_,
																const double	dec_,
																const double	parAngle_,
																const double	cBasLong_,
																const double	cBasLat_,
																const double	basLong_,
																const double	basLat_,
																const double	rotAngle_,
																const double	mCRVal1_,
																const double	mCRVal2_,
																const double	mLonPole_,
																const double	mLatPole_,
																const double	dFocus_x_,
																const double	dFocus_y_,
																const double	dFocus_z_,
																const double	dPhi_x_,
																const double	dPhi_y_,
																const double	dPhi_z_,
																const double	wobDisLN_,
																const double	wobDisLT_ );

		void monitor( const double	mjd_,
									const string& referenceKeyword_,
									const MBFitsManager::Double_v_t& value_,
									const string& keyword_ = string() );

	private:
		MBFitsManager();																												// Not implemented
		MBFitsManager( const MBFitsManager& mbFitsManager_ );										// Not implemented

		virtual bool operator==( const MBFitsManager& mbFitsManager_ ) const;		// Not implemented
		virtual bool operator!=( const MBFitsManager& mbFitsManager_ ) const;		// Not implemented
		MBFitsManager& operator=( const MBFitsManager& mbFitsManager_ );				// Not implemented

		void createPrimaryHeader( const string&											telescop_,
															const string&											origin_,
															const string&											creator_,
															const string&											mbftsVer_,

//															const string&											comment_,

//															const string&											hierarchEsoDprCatg_,
//															const string&											hierarchEsoDprType_,
//															const string&											hierarchEsoDprTech_,
//															const string&											hierarchEsoObsProgID_,
//															const long												hierarchEsoObsID_,
//															const double											hierarchEsoTelAirmStart_,
//															const double											hierarchEsoTelAirmEnd_,
//															const double											hierarchEsoTelAlt_,
//															const double											hierarchEsoTelAz_,

															const string&											instrume_,
															const MBFitsManager::String_v_t&	febes_,
															const MBFitsManager::Double_v_t&	restFreqencies_,
															const MBFitsManager::String_v_t&	lines_,
															const MBFitsManager::Double_v_t&	bandwidths_,
															const string&											object_,
															const string&											timeSys_,
															const double											mjdObs_,
															const string&											dateObs_,
															const double											mjd_,
															const double											ra_,
															const double											dec_,
															const string&											radecsys_,
															const float												equinox_,
															const double											exptime_,
															const string&											origFile_,
															const string&											projID_,

															const long												scanNum_ );

		void updatePrimaryHeader( const double integTim_ );

		void saveGroupingBinTableData( // const long			member_position_,
																	 const string&	member_location_,
																	 const string&	member_uri_type_,
																	 const string&	extName_,
																	 const long			subsNum_,
																	 const string&	febe_,
																	 const long			baseband_ );

		void createScanHeader( const string&										telescop_,
													 const double											siteLong_,
													 const double											siteLat_,
													 const float											siteElev_,
													 const float											diameter_,
													 const string&										projID_,
													 const string&										obsID_,
													 const long												scanNum_,
													 const string&										timeSys_,
													 const string&										dateObs_,
													 const double											mjd_,
													 const double											lst_,
													 const long												nObs_,
													 const long												nSubs_,
													 const double											utc2Ut1_,
													 const double											tai2Utc_,
													 const double											etUtc_,
													 const double											gpsTai_,
													 const string&										wcsName_,
													 const string&										radeSys_,
													 const float											equinox_,
													 const string&										cType1_,
													 const string&										cType2_,
													 const double											crVal1_,
													 const double											crVal2_,
													 const string&										cType1n_,
													 const string&										cType2n_,
													 const double											lonPole_,
													 const double											latPole_,
													 const string&										object_,
													 const double											bLongObj_,
													 const double											bLatObj_,
													 const double											longObj_,
													 const double											latObj_,
													 const double											patLong_,
													 const double											patLat_,
													 const string&										calCode_,
													 const bool												moveFram_,
													 const double											periDate_,
													 const double											periDist_,
													 const double											longAsc_,
													 const double											omega_,
													 const double											inclinat_,
													 const double											eccentr_,
													 const double											orbEpoch_,
													 const double											orbEqnox_,
													 const double											distance_,
													 const Scan&											scan_,
													 const double											tranDist_,
													 const double											tranFreq_,
													 const double											tranFocu_,
													 const bool												wobUsed_,
													 const double											wobThrow_,
													 const string&										wobDir_,
													 const float											wobCycle_,
													 const string&										wobMode_,
													 const string&										wobPatt_,
													 const MBFitsManager::String_v_t&	phases_,
													 const int												nFebe_,
													 const float											pDeltaIA_,
													 const float											pDeltaCA_,
													 const float											pDeltaIE_,
													 const float											fDeltaIA_,
													 const float											fDeltaCA_,
													 const float											fDeltaIE_,
													 const float											ia_,
													 const float											ie_,
													 const float											hasa_,
													 const float											haca_,
													 const float											hese_,
													 const float											hece_,
													 const float											hesa_,
													 const float											hasa2_,
													 const float											haca2_,
													 const float											hesa2_,
													 const float											heca2_,
													 const float											haca3_,
													 const float											heca3_,
													 const float											hesa3_,
													 const float											npae_,
													 const float											ca_,
													 const float											an_,
													 const float											aw_,
													 const float											hece2_,
													 const float											hece6_,
													 const float											hesa4_,
													 const float											hesa5_,
													 const float											hsca_,
													 const float											hsca2_,
													 const float											hssa3_,
													 const float											hsca5_,
													 const float											nrx_,
													 const float											nry_,
													 const float											hysa_,
													 const float											hyse_,
													 const double											setLinX_,
													 const double											setLinY_,
													 const double											setLinZ_,
													 const double											setRotX_,
													 const double											setRotY_,
													 const double											setRotZ_,
													 const double											moveFoc_,
													 const double											focAmp_,
													 const double											focFreq_,
													 const double											focPhase_ );

		void updateScanHeader( const long			nObs_,
													 const long			nSubs_ );

		void saveScanBinTableData( const string& febe_ );

		void createFebeParHeader( CMBFitsWriterTable* const febeParTable_p_,
															const string&	febe_,
															const int			scanNum_,
															const string&	dateObs_,
															const string&	dewCabin_,
															const string&	dewRtMod_,
															const float		dewUser_,
															const float		dewZero_,
															const string&	location_,
															const string&	optPath_,
															const int			nOptRefl_,
															const int			febeBand_,
															const int			febeFeed_,
															const int			nUseBand_,
															const string&	fdTypCod_,
															const float		feGain_,
															const string&	swtchMod_,
															const int			nPhases_,
															const float		frThrwLo_,
															const float		frThrwHi_,
															const float		tBlank_,
															const float		tSync_,
															const float		iaRx_,
															const float		ieRx_,
															const float		hasaRx_,
															const float		hacaRx_,
															const float		heseRx_,
															const float		heceRx_,
															const float		hesaRx_,
															const float		hasa2Rx_,
															const float		haca2Rx_,
															const float		hesa2Rx_,
															const float		heca2Rx_,
															const float		haca3Rx_,
															const float		heca3Rx_,
															const float		hesa3Rx_,
															const float		npaeRx_,
															const float		caRx_,
															const float		anRx_,
															const float		awRx_,
															const float		hece2Rx_,
															const float		hece6Rx_,
															const float		hesa4Rx_,
															const float		hesa5Rx_,
															const float		hscaRx_,
															const float		hsca2Rx_,
															const float		hssa3Rx_,
															const float		hsca5Rx_,
															const float		nRxRx_,
															const float		nRyRx_,
															const string&	sigOnln_,
															const string&	refOnln_,
															const string&	sigPol_,
															const string&	refPol_,
															const string&	rxBa_40_,
															const string&	rxBa_80_,
															const string&	rxCh_40_,
															const string&	rxCh_80_,
															const string&	rxHor_40_,
															const string&	rxHor_80_ );

		void saveFebeParBinTableData( CMBFitsWriterTable* const	febeParTable_p_,
																	const MBFitsManager::Long_v_t&		useBand_,
																	const MBFitsManager::Long_v_t&		nUseFeed_,
																	const MBFitsManager::Long_v_t&		useFeed_,
																	const MBFitsManager::Long_v_t&		beSects_,
																	const MBFitsManager::Long_v_t&		feedType_,
																	const MBFitsManager::Double_v_t&	feedOffX_,
																	const MBFitsManager::Double_v_t&	feedOffY_,
																	const int													refFeed_,
																	const string&											polTy_,
																	const MBFitsManager::Float_v_t&		polA_,
																	const MBFitsManager::Float_v_t&		aperEff_,
																	const MBFitsManager::Float_v_t&		beamEff_,
																	const MBFitsManager::Float_v_t&		etafss_,
																	const MBFitsManager::Float_v_t&		hpbw_,
																	const MBFitsManager::Float_v_t&		antGain_,
																	const MBFitsManager::Float_v_t&		tCal_,
																	const MBFitsManager::Float_v_t&		bolCalFc_,
																	const MBFitsManager::Float_v_t&		beGain_,
																	const MBFitsManager::Float_v_t&		flatFiel_,
																	const MBFitsManager::Float_v_t&		boldCoff_,
																	const MBFitsManager::Float_v_t&		gainImag_,
																	const MBFitsManager::Float_v_t&		gainEle1_,
																	const MBFitsManager::Float_v_t&		gainEle2_ );

		void createMonitorHeader( const int			scanNum_,
															const int			subSNum_,
//															const int			obsNum_,
															const string&	dateObs_,
															const string&	usrFrame_
															);

		void createDataParHeader( CMBFitsWriterTable* const dataParTable_p_,
															const int 						scanNum_,
															const int 						subsNum_,
//															const int 						obsNum_,
															const string&					dateObs_,
															const string&					febe_,
															const double					lst_,
															const string&					subsType_,
//															const string&					obsType_,

															const Scan&						scanReference_,
															const Scan&						scan_,

//															const string&					cType1n_,
//															const string&					cType2n_,

															const bool						dpBlock_,
															const int 						nInts_,
															const string&					obStatus_,
															const bool						wobCoord_,
															const vector<string>&	phaseN_,
															const float						dewAng_,
															const float						dewExtra_ );

		void updateDataParHeader( CMBFitsWriterTable* const dataParTable_p_,
															const string& obStatus_ );

		void saveDataParBinTableData( CMBFitsWriterTable* const dataParTable_p_,
																	const double	mjd_,
																	const double	lst_,
																	const double	integTim_,
																	const int			phase_,
																	const double	longOff_,
																	const double	latOff_,
																	const double	azimuth_,
																	const double	elevatio_,
																	const double	ra_,
																	const double	dec_,
																	const double	parAngle_,
																	const double	cBasLong_,
																	const double	cBasLat_,
																	const double	basLong_,
																	const double	basLat_,
																	const double	rotAngle_,
																	const double	mcrVal1_,
																	const double	mcrVal2_,
																	const double	mLonPole_,
																	const double	mLatPole_,
																	const double	dFocus_x_,
																	const double	dFocus_y_,
																	const double	dFocus_z_,
																	const double	dPhi_x_,
																	const double	dPhi_y_,
																	const double	dPhi_z_,
																	const double	wobDisLn_,
																	const double	wobDisLt_ );

		void createArrayDataHeader( CMBFitsWriterTable* const arrayDataTable_p_,
																const string&	febe_,
																const int			baseBand_,
																const int			scanNum_,
																const int			subsNum_,
//																const int			obsNum_,
																const string&	dateObs_,
																const int			channels_,
																const int			nUseFeed_,
																const double	freqRes_,
																const double	bandWid_,
																const string&	molecule_,
																const string&	transiti_,
																const double	restFreq_,
																const double	skyFreq_,
																const string&	sideBand_,
																const double	sbSep_,
																const string&	_2ctyp2_,
																const int			_2crpx2_,
																const int			_2crvl2_,
																const int			_21cd2a_,

																// Next we assign to the specral axis a frequency and velocity description relative to the rest frame
																// But other alternatives axes descriptions could also be given
																const SpectralAxis& spectralAxisRestFrameMainSideband_,
																const SpectralAxis& spectralAxisRestFrameImageSideband_,
																const SpectralAxis& spectralAxisSkyFrameMainSideband_,
																const SpectralAxis& spectralAxisSkyFrameImageSideband_,
																const SpectralAxis& spectralAxisRestFrameVelocity_,

																const float		_1vsou2r_,
																const float		_1vsys2r_ );

		void saveArrayDataBinTableData( CMBFitsWriterTable* const				arrayDataTable_p_,
																		const double										mjd_,
																		const MBFitsManager::Float_v_t&	data_ );

		typedef map<string, CMBFitsWriterTable*>									FeBe_MBFitsWriter_p_m_t;
		typedef FeBe_MBFitsWriter_p_m_t::iterator									FeBe_MBFitsWriter_p_i_m_t;
		typedef FeBe_MBFitsWriter_p_m_t::const_iterator						FeBe_MBFitsWriter_p_ci_m_t;

		typedef map<long, CMBFitsWriterTable*>										Baseband_MBFitsWriter_p_m_t;
		typedef Baseband_MBFitsWriter_p_m_t::iterator							Baseband_MBFitsWriter_p_i_m_t;
		typedef Baseband_MBFitsWriter_p_m_t::const_iterator				Baseband_MBFitsWriter_p_ci_m_t;

		typedef map<string, Baseband_MBFitsWriter_p_m_t>					FeBe_Baseband_MBFitsWriter_p_m_t;
		typedef FeBe_Baseband_MBFitsWriter_p_m_t::iterator				FeBe_Baseband_MBFitsWriter_p_i_m_t;
		typedef FeBe_Baseband_MBFitsWriter_p_m_t::const_iterator	FeBe_Baseband_MBFitsWriter_p_ci_m_t;
		typedef pair<FeBe_Baseband_MBFitsWriter_p_i_m_t, bool>		FeBe_Baseband_MBFitsWriter_p_insert_m_t;

		typedef vector<CFitsWriter::TFeedHeader>									Feed_p_v_t;
		typedef Feed_p_v_t::iterator															Feed_p_i_v_t;
		typedef Feed_p_v_t::const_iterator												Feed_p_ci_v_t;

		static string																			m_lastVersion;

		bool																							m_isGrouping;

		bool                                              m_scanStarted;

		string																						m_path;
		string																						m_mbFitsVersion;
		string																						m_datasetName;
		Scan																							m_scan;

		CMBFitsWriterTable*																m_primaryHeaderGroupingTable_p;
		CMBFitsWriterTable*																m_scanTable_p;
		MBFitsManager::FeBe_MBFitsWriter_p_m_t						m_febeParTables;
		CMBFitsWriterTable*																m_monitorTable_p;
		MBFitsManager::FeBe_MBFitsWriter_p_m_t						m_dataParTables;
		MBFitsManager::FeBe_Baseband_MBFitsWriter_p_m_t 	m_arrayDataTables;
};

#endif // __H__MBFitsManager__
