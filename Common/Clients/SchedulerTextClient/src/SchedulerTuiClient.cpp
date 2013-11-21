#include "SchedulerTuiClient.h"

#define _GET_ACS_PROPERTY(TYPE,NAME) TYPE##_var NAME; \
{	\
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,"Trying to get property "#NAME"...")); \
	NAME=component->NAME(); \
	if (NAME.ptr()!=TYPE::_nil()) { \
		ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_DEBUG,"OK, reference is: %x",NAME.ptr())); \
	} \
	else { \
		_EXCPT(ClientErrors::CouldntAccessPropertyExImpl,impl,MODULE_NAME"::Main()"); \
		impl.setPropertyName(#NAME); \
		impl.log(); \
		goto ErrorLabel; \
	} \
}

#define _INSTALL_MONITOR(COMP,TRIGGERTIME) { \
	if (!COMP->installAutomaticMonitor(GUARDINTERVAL)) { \
		_EXCPT(ClientErrors::CouldntPerformActionExImpl,impl,MODULE_NAME"::Main()"); \
		impl.setAction("Install monitor"); \
		impl.setReason((const char*)COMP->getLastError()); \
		impl.log(); \
		ACE_OS::sleep(1); \
		goto ErrorLabel; \
	} \
	COMP->setTriggerTime(TRIGGERTIME); \
}

#define COMPONENT_INTERFACE COMPONENT_IDL_MODULE::COMPONENT_IDL_INTERFACE
#define COMPONENT_DECLARATION COMPONENT_IDL_MODULE::COMPONENT_SMARTPOINTER

#define TEMPLATE_4_ROTBOOLEAN  Management::ROTBoolean_ptr,ACS::Monitorpattern,ACS::Monitorpattern_var,_TW_CBpattern,ACS::CBpattern_var
#define TEMPLATE_4_ROTSYSTEMSTATUS  Management::ROTSystemStatus_ptr,ACS::Monitorpattern,ACS::Monitorpattern_var,_TW_CBpattern,ACS::CBpattern_var

using namespace TW;

static bool terminate;

void quintHandler(int sig)
{
	terminate=true;
}

IRA::CString trackingFormat(const baci::BACIValue& value,const void* arg)
{
	IRA::CString out="";
	BACIuLongLong app=0;
	app=value.uLongLongValue();
	if (Management::MNG_TRUE==(Management::TBoolean)app) {
		out="1";
	}
	else {
		out="0";
	}
	return out;
}

int main(int argc, char *argv[]) {
	bool loggedIn=false;
	//int fieldCounter;
	maci::SimpleClient client;
	ACE_Time_Value tv(1);
	//IRA::CString fields[MAXFIELDNUMBER];
	//char formatString[20];
	IRA::CString inputCommand;
	
	// Component declaration 
	COMPONENT_DECLARATION component=COMPONENT_INTERFACE::_nil();

	maci::ComponentInfo_var info;
	CORBA::Object_var obj;
		
	
	// Add frame controls declaration
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING> *projectCode_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING> *schedule_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)> *scanID_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)> *subScanID_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING> *currentBackend_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING> *currentRecorder_field;
	TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)> *currentDevice_field;
	TW::CPropertyStatusBox<TEMPLATE_4_ROTSYSTEMSTATUS,Management::TSystemStatus> * status_box;
	TW::CPropertyLedDisplay<TEMPLATE_4_ROTBOOLEAN> * tracking_display;
	// *******************************
	TW::CLabel *output_label;
	TW::CInputCommand *userInput;

	terminate=false;
	// mainframe 
	CFrame window(CPoint(0,0),CPoint(WINDOW_WIDTH,WINDOW_HEIGHT),'|','|','-','-');
	
	// disable ctrl+C
	signal(SIGINT,SIG_IGN);
	signal(SIGUSR1,quintHandler);
	
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::MANAGER_LOGGING"));
	try {
		if (client.init(argc,argv)==0) {
			_EXCPT(ClientErrors::CouldntInitExImpl,impl,MODULE_NAME"::Main()");
			impl.log();		
			goto ErrorLabel;
		}
		else {
			if (client.login()==0) {
				_EXCPT(ClientErrors::CouldntLoginExImpl,impl,MODULE_NAME"::Main()");
				impl.log();		
				goto ErrorLabel;
			}
			loggedIn=true;
		}	
	}
	catch(...) {
		_EXCPT(ClientErrors::UnknownExImpl,impl,MODULE_NAME"::Main()");
		IRA::CString Message;
		impl.log();
		goto ErrorLabel;
	}
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::DONE"));	
	// Window initialization
	window.initFrame();
	window.setTitle(APPLICATION_TITLE);
	// Change the style of the main frame 
	window.setTitleStyle(CStyle(TITLE_COLOR_PAIR,TITLE_STYLE));
	
	if (window.colorReady()) {
		window.defineColor(GRAY_COLOR,255,255,255);		
		window.defineColorPair(BLUE_GRAY,CColor::BLUE,GRAY_COLOR);
		window.defineColorPair(GRAY_BLUE,GRAY_COLOR,CColor::BLUE);		
	}
	else {
		window.defineColorPair(BLUE_GRAY,CColor::BLUE,CColor::WHITE);
		window.defineColorPair(GRAY_BLUE,CColor::WHITE,CColor::BLUE);	
		window.defineColorPair(BLACK_RED,CColor::BLACK,CColor::RED);	
		window.defineColorPair(BLACK_GREEN,CColor::BLACK,CColor::GREEN);	
		window.defineColorPair(BLACK_YELLOW,CColor::BLACK,CColor::YELLOW);	
		window.defineColorPair(BLACK_BLUE,CColor::BLACK,CColor::BLUE);		
		window.defineColorPair(BLACK_MAGENTA,CColor::BLACK,CColor::MAGENTA);	
		window.defineColorPair(BLACK_WHITE,CColor::BLACK,CColor::WHITE	);			
	}
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::FRAME_INITIALIZED"));
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::GET_DEFAULT_COMPONENENT: %s",COMPONENT_INTERFACE_TPYE));
	try {
		info=client.manager()->get_default_component(client.handle(),COMPONENT_INTERFACE_TPYE);
		obj=info->reference;
		if (CORBA::is_nil(obj.in())) {
			_EXCPT(ClientErrors::CouldntAccessComponentExImpl,impl,MODULE_NAME"::Main()");
			impl.setComponentName(COMPONENT_INTERFACE_TPYE);
			impl.log();
			goto ErrorLabel;
		}
	}
	catch (ACSErr::ACSbaseExImpl& E) {
		_ADD_BACKTRACE(ClientErrors::CouldntAccessComponentExImpl,impl,E,MODULE_NAME"::Main()");
		impl.setComponentName(COMPONENT_INTERFACE_TPYE);
		impl.log();
		goto ErrorLabel;
	}
	catch(CORBA::SystemException &E) {
		_EXCPT(ClientErrors::CORBAProblemExImpl,impl,MODULE_NAME"::Main()");
		impl.setName(E._name());
		impl.setMinor(E.minor());
		impl.log();
		goto ErrorLabel;
	}
	catch(...) {
		_EXCPT(ClientErrors::UnknownExImpl,impl,MODULE_NAME"::Main()");
		impl.log();
		goto ErrorLabel;
	}
	component=COMPONENT_INTERFACE::_narrow(obj.in());
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_DEBUG,"OK, reference is: %d",component.ptr()));
	ACE_OS::sleep(1);
	try {

		// Add all component properties here
		_GET_ACS_PROPERTY(ACS::ROstring,projectCode);
		_GET_ACS_PROPERTY(ACS::ROlong,scanID);
		_GET_ACS_PROPERTY(ACS::ROlong,subScanID);
		_GET_ACS_PROPERTY(ACS::ROstring,scheduleName);
		_GET_ACS_PROPERTY(ACS::ROstring,currentBackend);
		_GET_ACS_PROPERTY(ACS::ROstring,currentRecorder);
		_GET_ACS_PROPERTY(ACS::ROlong,currentDevice);
		_GET_ACS_PROPERTY(Management::ROTBoolean,tracking);
		_GET_ACS_PROPERTY(Management::ROTSystemStatus,status);
		// *********************************
		ACE_OS::sleep(1);
		
		// Frame controls creation

		projectCode_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING>(projectCode.in());
		schedule_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING>(scheduleName.in());
		scanID_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)>(scanID.in());
		subScanID_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)>(subScanID.in());
		currentDevice_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_RO(long)>(currentDevice.in());
		tracking_display=new TW::CPropertyLedDisplay<TEMPLATE_4_ROTBOOLEAN>(tracking.in());
		status_box=new TW::CPropertyStatusBox<TEMPLATE_4_ROTSYSTEMSTATUS,Management::TSystemStatus>(status.in(),Management::MNG_OK) ;
		currentBackend_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING>(currentBackend.in());
		currentRecorder_field=new TW::CPropertyText<_TW_PROPERTYCOMPONENT_T_ROSTRING>(currentRecorder.in());

		#if USE_OUTPUT_FIELD >=1 
			output_label=new TW::CLabel("");
		#else
			output_label=NULL;
		#endif
		userInput=new TW::CInputCommand();
		
		// setting up the properties of the components of the frame controls
		_TW_SET_COMPONENT(projectCode_field,22,0,24,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		_TW_SET_COMPONENT(schedule_field,22,1,20,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		_TW_SET_COMPONENT(scanID_field,22,2,4,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		char frm[8];
		strncpy(frm,"%04ld",8);
		scanID_field->setFormatFunction(CFormatFunctions::integerFormat,static_cast<const void *>(frm));
		scanID_field->setWAlign(CFrameComponent::RIGHT);
		_TW_SET_COMPONENT(subScanID_field,27,2,4,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		subScanID_field->setFormatFunction(CFormatFunctions::integerFormat,static_cast<const void *>(frm));
		_TW_SET_COMPONENT(currentBackend_field,22,3,24,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		_TW_SET_COMPONENT(currentRecorder_field,22,4,24,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		_TW_SET_COMPONENT(currentDevice_field,22,5,7,1,CColorPair::WHITE_BLACK,CStyle::BOLD,output_label);
		tracking_display->setPosition(CPoint(22,6));
		tracking_display->setOrientation(TW::CPropertyLedDisplay<TEMPLATE_4_ROTBOOLEAN>::HORIZONTAL);
		tracking_display->setFormatFunction(trackingFormat,NULL);
		tracking_display->setLedStyle(0,TW::CStyle(CColorPair::GREEN_BLACK,0),TW::CStyle(CColorPair::RED_BLACK,0));
		_TW_SET_COMPONENT(status_box,22,7,10,1,BLACK_GREEN,CStyle::BOLD,output_label);
		status_box->setStatusLook(Management::MNG_OK,CStyle(BLACK_GREEN,CStyle::BOLD));
		status_box->setStatusLook(Management::MNG_WARNING,CStyle(BLACK_YELLOW,CStyle::BOLD));
		status_box->setStatusLook(Management::MNG_FAILURE,CStyle(BLACK_RED,CStyle::BOLD));

		// ******************************************************************
		_TW_SET_COMPONENT(userInput,0,WINDOW_HEIGHT-6,WINDOW_WIDTH-1,1,USER_INPUT_COLOR_PAIR,USER_INPUT_STYLE,NULL);
		#if USE_OUTPUT_FIELD >=1 
			_TW_SET_COMPONENT(output_label,0,WINDOW_HEIGHT-(OUTPUT_FIELD_HEIGHT+1),WINDOW_WIDTH-1,OUTPUT_FIELD_HEIGHT,OUTPUT_FIELD_COLOR_PAIR,OUTPUT_FIELD_STYLE,NULL);	
		#endif 
		
		// Monitors
		ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::MONITORS_INSTALLATION"));
		// Add all required monitor installation here
		_INSTALL_MONITOR(schedule_field,1000);
		_INSTALL_MONITOR(scanID_field,1000);
		_INSTALL_MONITOR(subScanID_field,1000);
		_INSTALL_MONITOR(projectCode_field,1000);
		_INSTALL_MONITOR(currentDevice_field,1000);
		_INSTALL_MONITOR(tracking_display,200);
		_INSTALL_MONITOR(currentBackend_field,1000);
		_INSTALL_MONITOR(currentRecorder_field,1000);

		//tracking_display->setValueTrigger(1L,true);
		_INSTALL_MONITOR(status_box,3000);
		status_box->setValueTrigger(1L,true);

		ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::DONE"));
		
		// Add all the static labels
		_TW_ADD_LABEL("Project Code   :",0,0,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Schedule       :",0,1,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Scan/SubScan   :",0,2,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("/",26,2,1,1,CColorPair::WHITE_BLACK,0,window);
		_TW_ADD_LABEL("Backend        :",0,3,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Recorder       :",0,4,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Device         :",0,5,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Tracking       :",0,6,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		_TW_ADD_LABEL("Status         :",0,7,18,1,CColorPair::WHITE_BLACK,CStyle::UNDERLINE,window);
		// *************************
		
		// Add all required association: components/Frame
		window.addComponent((CFrameComponent*)schedule_field);
		window.addComponent((CFrameComponent*)scanID_field);
		window.addComponent((CFrameComponent*)subScanID_field);
		window.addComponent((CFrameComponent*)projectCode_field);
		window.addComponent((CFrameComponent*)currentDevice_field);
		window.addComponent((CFrameComponent*)tracking_display);
		window.addComponent((CFrameComponent*)currentBackend_field);
		window.addComponent((CFrameComponent*)currentRecorder_field);
		window.addComponent((CFrameComponent*)status_box);
		// **********************************************
		window.addComponent((CFrameComponent*)userInput);		
		#if USE_OUTPUT_FIELD >=1 
			window.addComponent((CFrameComponent*)output_label);
		#endif

	}
	catch(CORBA::SystemException &E) {		
		_EXCPT(ClientErrors::CORBAProblemExImpl,impl,MODULE_NAME"::Main()");
		impl.setName(E._name());
		impl.setMinor(E.minor());
		impl.log();
		goto ErrorLabel;
	}
	catch(...) {
		_EXCPT(ClientErrors::UnknownExImpl,impl,MODULE_NAME"::Main()");
		IRA::CString Message;
		impl.log();
		goto ErrorLabel;
	}
	// now it is possible to show the frame
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::START"));
	window.showFrame();
	while(!terminate) {
		if (userInput->readCommand(inputCommand)) {
			if (inputCommand=="exit") terminate=true;
			else if (component->_is_a("IDL:alma/Management/CommandInterpreter:1.0")) {
				try {
					char * outputAnswer;
					component->command((const char *)inputCommand,outputAnswer);
					output_label->setValue(outputAnswer);
					CORBA::string_free(outputAnswer);
					output_label->Refresh();
				}
				catch (CORBA::SystemException& ex) {
					_EXCPT(ClientErrors::CORBAProblemExImpl,impl,"Main()");
					impl.setName(ex._name());
					impl.setMinor(ex.minor());
					IRA::CString Message;
					_EXCPT_TO_CSTRING(Message,impl);
					output_label->setValue(Message);
					output_label->Refresh();					
					impl.log(LM_ERROR);
				}
				catch(...) {
					_EXCPT(ClientErrors::CouldntPerformActionExImpl,impl,"Main()");
					impl.setAction("command()");
					impl.setReason("communication error to component server");
					IRA::CString Message;
					_EXCPT_TO_CSTRING(Message,impl);
					output_label->setValue(Message);
					output_label->Refresh();
	                impl.log(LM_ERROR); 					
				}
			}
			else {
				output_label->setValue("not enabled");
				output_label->Refresh();				
			}
		}
		client.run(tv);
		tv.set(0,MAINTHREADSLEEPTIME*1000);
	}
		
	window.closeFrame();
	
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::RELEASING"));
	goto CloseLabel;
ErrorLabel:
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::ABORTING"));	
CloseLabel:
	window.Destroy();
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::FRAME_CLOSED"));
	ACE_OS::sleep(1);	
	try {
		if (!CORBA::is_nil(component.in())) {
			client.releaseComponent(component->name());
		}
	}
	catch (maciErrType::CannotReleaseComponentExImpl& E) {
		E.log();
	}
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::COMPONENT_RELEASED"));
	if (loggedIn) client.logout();
	ACS_LOG(LM_FULL_INFO,MODULE_NAME"::Main()",(LM_INFO,MODULE_NAME"::SHUTTING_DOWN"));		
	signal(SIGINT,SIG_DFL);
	signal(SIGUSR1,SIG_DFL);
	ACE_OS::sleep(1);
	return 0;
}

