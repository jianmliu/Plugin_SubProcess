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

/* definitions */

#define SUBPROCESSTHREAD_TIMEOUT    10000
#define SUBPROCESSTHREAD_EVENTSTART "SUBPROC_EVENT_START"
#define SUBPROCESSTHREAD_EVENTSTOP  "SUBPROC_EVENT_STOP"
#define SUBPROCESSTHREAD_SEPARATOR  '|'

/* SubProcess_Thread: thread for popen() */
class SubProcess_Thread
{
private:

   MMDAgent *m_mmdagent;

   GLFWthread m_thread;

   char *m_name;        /* name of thread */
   char *m_commandLine; /* command line string to invoke subprocess */
   FILE *m_stream;      /* I/O stream (NULL means not running) */

   /* initialize: initialize thread */
   void initialize();

   /* clear: free thread */
   void clear();

public:

   /* SubProcess_Thread: thread constructor */
   SubProcess_Thread();

   /* ~SubProcess_Thread: thread destructor */
   ~SubProcess_Thread();

   /* loadAndStart: load program and start thread */
   void loadAndStart(MMDAgent *mmdagent, const char *args);

   /* stopAndRelease: stop thread and release */
   void stopAndRelease();

   /* run: main loop */
   void run();

   /* isRunning: check running */
   bool isRunning();

   /* checkName: check thread name */
   bool checkName(const char *args);

   /* puts: write a string and a trailing newline to subprocess */
   int puts(const char *str);
};
