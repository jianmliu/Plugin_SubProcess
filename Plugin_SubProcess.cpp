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

#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif /* _WIN32 */

#define PLUGINSUBPROCESS_NAME         "SubProcess"
#define PLUGINSUBPROCESS_STARTCOMMAND "SUBPROC_START"
#define PLUGINSUBPROCESS_STOPCOMMAND  "SUBPROC_STOP"

/* headers */

#include "MMDAgent.h"

#include "SubProcess_Queue.h"
#include "SubProcess_Thread.h"
#include "SubProcess_Manager.h"

/* variables */

static SubProcess_Manager subprocess_manager;
static bool enable;

/* extAppStart: start thread */
EXPORT void extAppStart(MMDAgent *mmdagent)
{
   subprocess_manager.loadAndStart(mmdagent);

   enable = true;
   mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINSUBPROCESS_NAME);
}

/* extProcMessage: process event/command message */
EXPORT void extProcMessage(MMDAgent *mmdagent, const char *type, const char *args)
{
   if(enable == true) {
      if (subprocess_manager.isRunning()) {
         /* start or stop subprocess */
         if (MMDAgent_strequal(type, PLUGINSUBPROCESS_STARTCOMMAND)) {
            subprocess_manager.startProcess(args);
         } else if (MMDAgent_strequal(type, PLUGINSUBPROCESS_STOPCOMMAND)) {
            subprocess_manager.stopProcess(args);
         }
         /* enqueue message */
		subprocess_manager.enqueueBuffer(type, args);
      }
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINDISABLE)) {
         if(MMDAgent_strequal(args, PLUGINSUBPROCESS_NAME)) {
            enable = false;
            mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINDISABLE, "%s", PLUGINSUBPROCESS_NAME);
         }
      }
   } else {
      if(MMDAgent_strequal(type, MMDAGENT_COMMAND_PLUGINENABLE)) {
         if(MMDAgent_strequal(args, PLUGINSUBPROCESS_NAME)) {
            enable = true;
            mmdagent->sendMessage(MMDAGENT_EVENT_PLUGINENABLE, "%s", PLUGINSUBPROCESS_NAME);
         }
      }
   }
}

/* extAppEnd: stop and free thread */
EXPORT void extAppEnd(MMDAgent *mmdagent)
{
   subprocess_manager.stopAndRelease();
}
