/***************************************************************************
 *   Copyright (C) 2006 by Andre Neto   *
 *   andre.neto@cfn.ist.utl.pt   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/**
   @file 
   A semaphore used to synchronise tasks. This is a copy of the JET - PPCC BaseLib2's linux semaphore
*/
#ifndef EVENT_SEM
#define EVENT_SEM

#include <math.h>
#include <sys/timeb.h>
#include <pthread.h>

/** Private structure used for solaris adn linux when using pThread. */
class PrivateEventSemStruct{
    // Mutex Handle
    pthread_mutex_t       mutexHandle;

    // Mutex Attributes
    pthread_mutexattr_t   mutexAttributes;

    // Conditional Variable
    pthread_cond_t        eventVariable;

    // boolean semaphore
    bool                  stop;

public:
    //
    PrivateEventSemStruct(){
	stop = true;
    }
    //
    ~PrivateEventSemStruct(){}

    bool Init(){
	stop = true;
        if (pthread_mutexattr_init(&mutexAttributes) != 0)        return false;
        if (pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0) return false;
        if (pthread_cond_init(&eventVariable,NULL)!=0)            return false;
        return true;
    }

    bool Close(){
	Post();
        if(!pthread_mutexattr_destroy(&mutexAttributes))          return false;
        if(pthread_mutex_destroy(&mutexHandle)  != 0)             return false;
        if(pthread_cond_destroy(&eventVariable) != 0)             return false;
        return true;
    }

    bool Wait(int msecTimeout){
        if(msecTimeout == -1){
            if(pthread_mutex_lock(&mutexHandle) != 0)                          return false;
	    if(stop == true){
                if(pthread_cond_wait(&eventVariable,&mutexHandle) != 0){
                     pthread_mutex_unlock(&mutexHandle);
                     return false;
                }
	    }
            if(pthread_mutex_unlock(&mutexHandle) != 0)                        return false;

        }else{
            struct timespec timesValues;
            timeb tb;
            ftime( &tb );
            
            double sec = ((msecTimeout + tb.millitm)*1e-3 + tb.time);
            
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            if(pthread_mutex_timedlock(&mutexHandle, &timesValues) != 0)                 return false;
	    if(stop == true){
                if(pthread_cond_timedwait(&eventVariable,&mutexHandle,&timesValues) != 0){
                    pthread_mutex_unlock(&mutexHandle);
                    return false;
                }
	    }
            if(pthread_mutex_unlock(&mutexHandle) != 0)                                  return false;

        } 
        
        return true;
    }

    bool Post(){
	if(pthread_mutex_lock(&mutexHandle) != 0)                                        return false;
        stop = false;
        if(pthread_mutex_unlock(&mutexHandle) != 0)                                      return false;
	return (pthread_cond_broadcast(&eventVariable) != 0);
    }

    bool Reset(){
	if(pthread_mutex_lock(&mutexHandle) != 0)                                        return false;
	stop = true;
        if(pthread_mutex_unlock(&mutexHandle) != 0)                                      return false;
        return true;
    }

};

/** Definition of an event shemaphore. */
class EventSem{
public:
    /** Creates the semafore */
    bool Create(){
        if(semH != (void *)NULL) 
            delete (PrivateEventSemStruct *)semH;
        // Create the Structure
        semH = (void *) 
            new PrivateEventSemStruct();
        if(semH == (void *)NULL) 
            return false;
        // Initialize the Semaphore
        bool ret = ((PrivateEventSemStruct *)semH)->Init();
        if(!ret){
            delete (PrivateEventSemStruct *)semH;
            semH = (void *)NULL;
            return false;
        }
        return true;
    }

    /** closes the semafore */
    bool Close(void){
        bool ret = true;
        if(semH == (void *)NULL)
            return true;
        delete (PrivateEventSemStruct *)semH;
        semH = (void *)NULL;
        return ret;
    }

    /** wait for an event */
    bool Wait(long msecTimeout = -1){
        if(semH == (void *)NULL)
            return false;
        return ((PrivateEventSemStruct *)semH)->Wait(msecTimeout);
    }

    /** resets the semafore and then waits*/
    bool ResetWait(long msecTimeout = -1){
        Reset();
        return Wait(msecTimeout);
    }

    /** Send an event to semafore */
    bool Post(void){
        if(semH == (void *)NULL)
            return false;
        return ((PrivateEventSemStruct *)semH)->Post();
    }

    /** reset the semafore to its unposted state */
    bool Reset(void){
        if(semH == (void *)NULL)
            return false;
        return ((PrivateEventSemStruct *)semH)->Reset();
    }

    /** */
    EventSem(void * h){
        semH = h;
    }

    /** copies semaphore and special infos as well. */
    void operator=(EventSem &s){
        *this = s;
    }

    /** */
    EventSem(){
    }

    /** */
    ~EventSem(){
        Close();
    }

private:
    void *semH = NULL;
};

#endif
