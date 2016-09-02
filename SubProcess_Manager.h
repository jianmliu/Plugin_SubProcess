/* ----------------------------------------------------------------- */
/*           SubProcess plugin for MMDAgent                          */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2016-2016  Jianming Liu                            */
/*  Copyright (c) 2011-2012  S. Irie                                 */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* 1. Redistributions of source code must retain the above copyright */
/*    notice, this list of conditions and the following disclaimer.  */
/* 2. Redistributions in binary form must reproduce the above        */
/*    copyright notice, this list of conditions and the following    */
/*    disclaimer in the documentation and/or other materials         */
/*    provided with the distribution.                                */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR             */
/* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT  */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF  */
/* USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED   */
/* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT       */
/* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN */
/* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE   */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* SubProcess_Link: cell of subprocess list */
typedef struct _SubProcess_Link {
   SubProcess_Thread proc;
   struct _SubProcess_Link *next;
} SubProcess_Link;

/* SubProcess_Manager: multi thread manager for subprocesses */
class SubProcess_Manager
{
private:

   MMDAgent *m_mmdagent;

   GLFWmutex m_mutex;  /* mutual exclusion for message queue */
   GLFWmutex m_mutex2; /* mutual exclusion for sub-thread list */
   GLFWcond m_cond;
   GLFWthread m_thread;

   bool m_kill;

   SubProcess_Queue m_queue; /* queue of input message */
   SubProcess_Link *m_procs; /* list of subprocesses */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* SubProcess_Manager: thread constructor */
   SubProcess_Manager();

   /* ~SubProcess_Manager: thread destructor */
   ~SubProcess_Manager();

   /* loadAndStart: start thread manager*/
   void loadAndStart(MMDAgent *mmdagent);

   /* stopAndRelease: stop threads and release */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* startProcess: start subprocess by creating socketpair */
   void startProcess(const char *str);

   /* stopProcess: stop subprocess and close socketpair */
   void stopProcess(const char *str);

   /* enqueueBuffer: enqueue buffer to send */
   void enqueueBuffer(const char *type, const char *args);
};
