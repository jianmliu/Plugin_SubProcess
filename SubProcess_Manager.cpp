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

/* headers */

#include "MMDAgent.h"

#include "SubProcess_Queue.h"
#include "SubProcess_Thread.h"
#include "SubProcess_Manager.h"

/* mainThread: main thread */
static void mainThread(void *param)
{
   SubProcess_Manager *subprocess_manager = (SubProcess_Manager *) param;
   subprocess_manager->run();
}

/* SubProcess_Manager::initialize: initialize thread */
void SubProcess_Manager::initialize()
{
   m_mmdagent = NULL;

   m_kill = false;

   m_mutex = NULL;
   m_mutex2 = NULL;
   m_cond = NULL;
   m_thread = -1;

   m_procs = NULL;
}

/* SubProcess_Manager::clear: free thread */
void SubProcess_Manager::clear()
{
   SubProcess_Link *link, *next;

   m_kill = true;

   /* wait */
   if(m_cond != NULL)
      glfwSignalCond(m_cond);

   /* stop thread & close mutex */
   if(m_mutex != NULL || m_mutex2 != NULL || m_cond != NULL || m_thread >= 0) {
      if(m_thread >= 0) {
         glfwWaitThread(m_thread, GLFW_WAIT);
         glfwDestroyThread(m_thread);
      }
      if(m_cond != NULL)
         glfwDestroyCond(m_cond);
      if(m_mutex != NULL)
         glfwDestroyMutex(m_mutex);
      if(m_mutex2 != NULL)
         glfwDestroyMutex(m_mutex2);
      glfwTerminate();
   }

   /* free */
   m_queue.clear();

   for(link = m_procs; link != NULL; link = next) {
      next = link->next;
      delete link;
   }

   initialize();
}

/* SubProcess_Manager::SubProcess_Manager: thread constructor */
SubProcess_Manager::SubProcess_Manager()
{
   initialize();
}

/* SubProcess_Manager::~SubProcess_Manager: thread destructor */
SubProcess_Manager::~SubProcess_Manager()
{
   clear();
}

/* SubProcess_Manager::loadAndStart: start thread manager */
void SubProcess_Manager::loadAndStart(MMDAgent *mmdagent)
{
   clear();

   if(mmdagent == NULL)
      return;

   m_mmdagent = mmdagent;

   /* start thread */
   glfwInit();
   m_mutex = glfwCreateMutex();
   m_mutex2 = glfwCreateMutex();
   m_cond = glfwCreateCond();
   m_thread = glfwCreateThread(mainThread, this);
   if(m_mutex == NULL || m_mutex2 == NULL || m_cond == NULL || m_thread < 0) {
      clear();
      return;
   }
}

/* SubProcess_Manager::stopAndRelease: stop threads and release */
void SubProcess_Manager::stopAndRelease()
{
   clear();
}

/* SubProcess_Manager::run: main loop */
void SubProcess_Manager::run()
{
   char *type, *args, *buff;
   SubProcess_Link *link, *prev, *unused;

   while(m_kill == false) {
      glfwLockMutex(m_mutex);

      /* wait messages from main program */
      while(m_queue.isEmpty()) {
         glfwWaitCond(m_cond, m_mutex, GLFW_INFINITY);
         if(m_kill == true)
            return;
      }

      /* dequeue event */
      m_queue.dequeue(&type, &args);

      glfwUnlockMutex(m_mutex);

      /* send message to all subprocesses */
      buff = (char *) malloc(sizeof(char) * (MMDAgent_strlen(type)
                                             + MMDAgent_strlen(args) + 4));
      if(MMDAgent_strlen(args) > 0) {
		sprintf(buff, "%s|%s", type, args);
      } else {
		sprintf(buff, "%s", type);
      }

      glfwLockMutex(m_mutex2);

      prev = unused = NULL;
      for(link = m_procs; link != NULL;) {
         if(link->proc.isRunning() == true) {
            /* send message to thread */
            link->proc.puts(buff);
            prev = link;
            link = link->next;
         } else {
            /* discard link of thread not running */
            if(prev == NULL) {
               m_procs = link->next;
               link->next = unused;
               unused = link;
               link = m_procs;
            } else {
               prev->next = link->next;
               link->next = unused;
               unused = link;
               link = prev->next;
            }
         }
      }

      glfwUnlockMutex(m_mutex2);

      for(link = unused; link != NULL; link = link->next) {
         delete link;
      }

      free(type);
      free(args);
      free(buff);
   }
}

/* SubProcess_Manager::isRunning: check running */
bool SubProcess_Manager::isRunning()
{
   if (m_kill == true || m_mutex == NULL || m_mutex2 == NULL || m_cond == NULL || m_thread < 0)
      return false;
   else
      return true;
}

/* SubProcess_Manager::startProcess: start subprocess by creating socketpair */
void SubProcess_Manager::startProcess(const char *str)
{
   SubProcess_Link *newlink, *link, *prev = NULL;

   newlink = new SubProcess_Link;
   newlink->proc.loadAndStart(m_mmdagent, str);
   if(newlink->proc.isRunning() == false) {
      delete newlink;
      return;
   }

   glfwLockMutex(m_mutex2);

   for(link = m_procs; link != NULL; link = link->next) {
      if(link->proc.checkName(str) == true)
         break;
      prev = link;
   }

   if(link != NULL)
      /* replace existing thread if name is already used */
      newlink->next = link->next;
   else
      /* add new thread to the end of the list */
      newlink->next = NULL;

   if(prev == NULL)
      m_procs = newlink;
   else
      prev->next = newlink;

   glfwUnlockMutex(m_mutex2);

   if(link != NULL)
      delete link;
}

/* SubProcess_Manager::stopProcess: stop subprocess and close socketpair */
void SubProcess_Manager::stopProcess(const char *str)
{
   SubProcess_Link *link, *prev = NULL;

   glfwLockMutex(m_mutex2);

   for(link = m_procs; link != NULL; link = link->next) {
      if(link->proc.checkName(str) == true) {
         /* remove link from thread list */
         if(prev == NULL)
            m_procs = link->next;
         else
            prev->next = link->next;

         break;
      }
      prev = link;
   }

   glfwUnlockMutex(m_mutex2);

   if (link != NULL) {
      link->proc.stopAndRelease();
      delete link;
   }
}

/* SubProcess_Manager::enqueueBuffer: enqueue buffer to send */
void SubProcess_Manager::enqueueBuffer(const char *type, const char *args)
{
   glfwLockMutex(m_mutex);

   /* enqueue event */
   m_queue.enqueue(type, args);

   /* start message dispatcher thread */
   glfwSignalCond(m_cond);

   glfwUnlockMutex(m_mutex);
}
