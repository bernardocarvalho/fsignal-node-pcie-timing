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
   a Mutex semafore. This is a copy of the JET - PPCC BaseLib2's linux semaphore
*/


#ifndef MUTEX_SEM
#define MUTEX_SEM

#include <math.h>
#include <sys/timeb.h>
#include <pthread.h>

/** Auxiliary class needed to implement the MutexSem class under
    Solaris and Linux. */
class PrivateMutexSemStruct{
    /**  Mutex Handle */
    pthread_mutex_t       mutexHandle;
    /** Mutex Attributes */
    pthread_mutexattr_t   mutexAttributes;
public:
    /** */
    PrivateMutexSemStruct(){}
    /** */
    ~PrivateMutexSemStruct(){}

    /** */
    bool Init(){
        if(pthread_mutexattr_init(&mutexAttributes) != 0)                         return false;
        if(pthread_mutexattr_settype(&mutexAttributes,PTHREAD_MUTEX_RECURSIVE)!=0)return false;
        if(pthread_mutex_init(&mutexHandle,&mutexAttributes)!=0)                  return false;
        return true;
    }

    /** */
    bool Close(){
        if(!pthread_mutexattr_destroy(&mutexAttributes))                          return false;
        if(!pthread_mutex_destroy(&mutexHandle))                                  return false;
        return true;
    }

    /** */
    bool Lock(long msecTimeout = -1){
        if(msecTimeout == -1){
            if(pthread_mutex_lock(&mutexHandle) != 0)                 return false;
        }else{
            
            struct timespec timesValues;
            timeb tb;
            ftime( &tb );
            double sec        = ((msecTimeout + tb.millitm)*1e-3 + tb.time);
            double roundValue = floor(sec);
            timesValues.tv_sec  = (int)roundValue;
            timesValues.tv_nsec = (int)((sec-roundValue)*1E9);
            int err = 0;
            if((err = pthread_mutex_timedlock(&mutexHandle, &timesValues)) != 0){
                return false;
            }
        } 
        return true;
    }

    /** */
    bool UnLock(){
        return (pthread_mutex_unlock(&mutexHandle)==0);
    }

    /** */
    bool TryLock(){
        return (pthread_mutex_trylock(&mutexHandle)==0);
    }


};

/** a mutual exclusion semaphore */
class MutexSem {

public:
    /** open the semafore with a given initial state */
    bool Create(bool locked=false){
        if(semH != (void *)0) 
            delete (PrivateMutexSemStruct *)semH;
        // Create the Structure
        semH = (void *) new PrivateMutexSemStruct();
        if(semH == (void *)0) 
            return false;
        // Initialize the Semaphore
        bool ret = ((PrivateMutexSemStruct *)semH)->Init();
        if(!ret){
            delete (PrivateMutexSemStruct *)semH;
            semH = (void *)0;
            return false;
        }
        if(locked == true){
            ((PrivateMutexSemStruct *)semH)->Lock();
        }

        return true;
    };
    
    /** close the semafore handle */
    bool Close(){
        if (semH==(void *)0) 
            return true;        
        delete (PrivateMutexSemStruct *)semH;        
        semH=(void *)0;
        return true;
    };

    /** grab the semafore */
    bool Lock(long msecTimeout = -1){
        if(semH == (void *)0)
            return false;
        return ((PrivateMutexSemStruct *)semH)->Lock(msecTimeout);
    };

    /** returns the ownership */
    bool UnLock(void){
        if(semH == (void *)0)
            return false;
        return ((PrivateMutexSemStruct *)semH)->UnLock();
    };

    /** constructor */
    MutexSem(void * h){
        semH = h;
    }

    /** default constructor */
    MutexSem(){
    }

    /** destructor */
    ~MutexSem(){
        Close();
    }
    
private:
    /** A number associated to te semaphore. */
    void *semH = NULL;
};


#endif
