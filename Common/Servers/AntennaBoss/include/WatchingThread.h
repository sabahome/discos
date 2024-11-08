#ifndef _WATCHINGTHREAD_H_
#define _WATCHINGTHREAD_H_

/* ************************************************************************************************************* */
/* IRA Istituto di Radioastronomia                                                                               */
/* $Id: WatchingThread.h,v 1.2 2008-03-07 17:12:52 a.orlati Exp $										         */
/*                                                                                                               */
/* This code is under GNU General Public Licence (GPL).                                                          */
/*                                                                                                               */
/* Who                                            when             What                                                       */
/* Andrea Orlati(aorlati@ira.inaf.it)  28/01/2008      Creation                                                  */

#include <acsThread.h>
#include <IRA>
#include <BossCore.h>

/**
 * This class implements a watching thread. This thread is in charge of update all the obseved coordinates. It is started when the component
 *  goes up and it conitnues to run until the component dies. 
*/
class CWatchingThread : public ACS::Thread
{
public:
	/**
     * Constructor().
     * @param name thread name
     * @param responseTime thread's heartbeat response time in 100ns unit. Default value is 1s.
     * @param sleepTime thread's sleep time in 100ns unit. Default value is 100ms.
    */
	CWatchingThread(const ACE_CString& name,CBossCore *param,
			const ACS::TimeInterval& responseTime=ThreadBase::defaultResponseTime,const ACS::TimeInterval& sleepTime=ThreadBase::defaultSleepTime);

	/**
	 * Destructor.
    */
    ~CWatchingThread();

     /**
     * This method is executed once when the thread starts.
    */
    virtual void onStart();

    /**
     * This method is executed once when the thread stops.
     */
     virtual void onStop();

     /**
      * This method overrides the thread implementation class. The method is executed in a loop until the thread is alive.
      * The thread can be exited by calling ACS::ThreadBase::stop or ACS::ThreadBase::exit command.
     */
     virtual void runLoop();
private:
	CBossCore *boss;
};

#endif /*WATCHINGTHREAD_H_*/

