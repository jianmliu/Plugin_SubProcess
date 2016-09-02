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

/* headers */

#include "MMDAgent.h"
#include <poll.h>
#include <signal.h>

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include "SubProcess_Thread.h"

/* association list of PID */
struct pid_assoc
{
    FILE *stream;
    pid_t pid;
    struct pid_assoc *next;
} *pids = NULL;

/* spawn subprocess with socketpair connected */
FILE *spopen(const char *command)
{
    int sv[2], saved_errno;
    pid_t pid;

    if(command == NULL) {
        errno = EINVAL;
        return NULL;
    }

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1)
        return NULL;

    switch(pid = fork()) {
    case -1: /* error */
        saved_errno = errno;
        close(sv[0]);
        close(sv[1]);
        errno = saved_errno;

        return NULL;

    case 0: /* child */
    {
        int fd1, fd2;
        char *buff = NULL;

        close(sv[0]); /* unused */

        fd1 = dup2(sv[1], 0); /* socketpair(in)  -> stdin */
        fd2 = dup2(sv[1], 1); /* socketpair(out) -> stdout */

        close(sv[1]);

        if(fd1 >= 0 && fd2 >= 0) {
            buff = (char *) malloc(sizeof(char) * (strlen(command) + 5 + 1));

            if(buff != NULL) {
                strcpy(buff, "exec "); /* 5 characters */
                strcat(buff, command);
                execl("/bin/sh", "sh", "-c", buff, (char *) NULL);
            }
        }

        /* error */
        _exit(1);
    }
    default: /* parent */
    {
        struct pid_assoc *assoc;

        close(sv[1]); /* unused */

        assoc = (struct pid_assoc *) malloc(sizeof(struct pid_assoc));

        if(assoc != NULL) {
            assoc->stream = fdopen(sv[0], "r+");

            if(assoc->stream != NULL) {
                assoc->pid = pid;

                /* insert assoc to head of list */
                if(pids == NULL)
                    assoc->next = NULL;
                else
                    assoc->next = pids;

                pids = assoc;

                return assoc->stream;
            }
            else
                saved_errno = errno;

            /* error */
            free(assoc);
        }
        else
            saved_errno = ENOMEM;

        /* error */
        close(sv[0]);
        errno = saved_errno;
        return NULL;
    }
    }
}

/* close socketpair and wait for subprocess to stop */
int spclose(FILE *stream)
{
    pid_t pid;
    int status;
    struct pid_assoc *assoc, *prev = NULL;

    if(stream == NULL)
        return -1;

    /* close streams */
    fclose(stream);

    /* remember PID */
    for(assoc = pids; assoc != NULL; assoc = assoc->next) {
        if(assoc->stream == stream) {
            /* wait for child process to stop */
            /* (ignore SIGCHLD when the other child process stops) */
            while((pid = waitpid(assoc->pid, &status, 0)) == -1 && errno == EINTR);

            /* remove assoc from list */
            if(prev == NULL)
                pids = assoc->next;
            else
                prev->next = assoc-> next;

            free(assoc);

            /* return exit status of child process */
            return (pid == -1) ? -1 : status;
        }

        prev = assoc;
    }

    return -1;
}

/* get PID of subprocess that socketpair stream is bound to */
pid_t spgetpid(FILE *stream)
{
    struct pid_assoc *assoc;

    for(assoc = pids; assoc != NULL; assoc = assoc->next) {
        if(assoc->stream == stream)
            return assoc->pid;
    }

    return -1;
}

/* getArgFromString: get argument from string using separators */
static int getArgFromString(const char *str, int *index, char *buff)
{
   char c;
   int i, beg, end, pos = 0;

   /* skip white spaces */
   c = str[*index];
   while(c == ' ' || c == '\t' || c == '\n' || c == '\r')
      c = str[++(*index)];

   beg = *index;

   /* search separator */
   while(c != SUBPROCESSTHREAD_SEPARATOR && c != '\0' && c != '\n' && c != '\r')
      c = str[++(*index)];

   end = *index;

   if(c == SUBPROCESSTHREAD_SEPARATOR)
      ++(*index);

   /* trim trailing white spaces */
   for(; end > 0; --end) {
      c = str[end - 1];
      if(c != ' ' && c != '\t')
         break;
   }

   /* copy the argument to the place specified by buff */
   for(i = beg; i < end; ++i)
      buff[pos++] = str[i];

   buff[pos] = '\0';
   return pos;
}

/* mainThread: main thread */
static void mainThread(void *param)
{
   SubProcess_Thread *subprocess_thread = (SubProcess_Thread *) param;
   subprocess_thread->run();
}

/* SubProcess_Thread::initialize: initialize thread */
void SubProcess_Thread::initialize()
{
   m_mmdagent = NULL;

   m_thread = -1;

   m_name = NULL;
   m_commandLine = NULL;
   m_stream = NULL;
}

/* SubProcess_Thread::clear: free thread */
void SubProcess_Thread::clear()
{
   /* stop subprocess */
   if(m_stream != NULL) {
      kill(spgetpid(m_stream), SIGHUP);
      spclose(m_stream);
   }

   /* stop thread */
   if(m_thread >= 0) {
      glfwWaitThread(m_thread, GLFW_WAIT);
      glfwDestroyThread(m_thread);
   }

   /* free */
   free(m_name);
   free(m_commandLine);

   initialize();
}

/* SubProcess_Thread::SubProcess_Thread: thread constructor */
SubProcess_Thread::SubProcess_Thread()
{
   initialize();
}

/* SubProcess_Thread::~SubProcess_Thread: thread destructor */
SubProcess_Thread::~SubProcess_Thread()
{
   clear();
}

/* loadAndStart: load program and start thread */
void SubProcess_Thread::loadAndStart(MMDAgent *mmdagent, const char *args)
{
   int len, idx = 0;
   char *buff;

   clear();

   if(mmdagent == NULL)
      return;

   buff = (char *) malloc(sizeof(char) * (MMDAgent_strlen(args) + 1));

   /* get alias */
   if(getArgFromString(args, &idx, buff) == 0) {
      free(buff);
      return;
   }
   m_name = MMDAgent_strdup(buff);

   m_mmdagent = mmdagent;

   /* get command */
   len = getArgFromString(args, &idx, buff);
   if(len == 0) {
      free(buff);
      clear();
      return;
   }

   /* add an argument to command line if given */
   if(args[idx] != '\0') {
      buff[len] = ' ';
      strcpy(&buff[len + 1], &args[idx]);
   }
   m_commandLine = MMDAgent_strdup(buff);

   free(buff);

   /* start subprocess */
   m_stream = spopen(m_commandLine);
   if(m_stream == NULL){
      clear();
      return;
   }

   /* start thread */
   m_thread = glfwCreateThread(mainThread, this);
   if(m_thread < 0) {
      clear();
      return;
   }

   m_mmdagent->sendMessage(SUBPROCESSTHREAD_EVENTSTART, "%s", m_name);
}

/* SubProcess_Thread::stopAndRelease: stop thread and release */
void SubProcess_Thread::stopAndRelease()
{
   MMDAgent *mmdagent = m_mmdagent;
   char *name = MMDAgent_strdup(m_name);

   clear();
   mmdagent->sendMessage(SUBPROCESSTHREAD_EVENTSTOP, "%s", name);

   free(name);
}

/* SubProcess_Thread::run: main loop */
void SubProcess_Thread::run()
{
   int pos, idx;
   char c;
   char buff[MMDAGENT_MAXBUFLEN];
   char type[MMDAGENT_MAXBUFLEN];
   pollfd pfd;

   pfd.fd = fileno(m_stream);
   pfd.events = POLLIN;

   /* main loop */
   while(poll(&pfd, 1, SUBPROCESSTHREAD_TIMEOUT) >= 0) {
      if(pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
         if(pfd.revents & (POLLHUP | POLLERR))
            /* subprocess stopped */
            m_mmdagent->sendMessage(SUBPROCESSTHREAD_EVENTSTOP, "%s", m_name);

         break;
      } else if(pfd.revents & POLLIN) {
         /* receive message from subprocess */
         if(fgets(buff, MMDAGENT_MAXBUFLEN, m_stream) != NULL) {
            /* discard trailing newlines */
            for(pos = MMDAgent_strlen(buff) - 1; pos > 1; --pos) {
               c = buff[pos];
               if(c != '\n' && c != '\r') {
                  /* forward the message to main program */
				idx = 0;
				if(getArgFromString(buff, &idx, type) > 0)
                    m_mmdagent->sendMessage(type, "%s", &buff[idx]); 
                break;
               }
            }
         }
      }
   }
}

/* SubProcess_Thread::isRunning: check running */
bool SubProcess_Thread::isRunning()
{
   if (m_stream == NULL || m_thread < 0
       || glfwWaitThread(m_thread, GLFW_NOWAIT) == GL_TRUE)
      return false;
   else
      return true;
}

/* SubProcess_Thread::checkName: check thread name */
bool SubProcess_Thread::checkName(const char *args)
{
   int idx = 0;
   char *name = (char *) malloc(sizeof(char) * (MMDAgent_strlen(args) + 1));
   bool retval;

   getArgFromString(args, &idx, name);
   retval = MMDAgent_strequal(m_name, name);

   free(name);
   return retval;
}

/* SubProcess_Thread::puts: write a string and a trailing newline to subprocess */
int SubProcess_Thread::puts(const char *str)
{
   pollfd pfd;

   if(m_stream == NULL)
      return EOF;

   pfd.fd = fileno(m_stream);
   pfd.events = POLLOUT;

   /* check if output buffer is ready or full */
   if(poll(&pfd, 1, 0) < 1)
      return EOF;

   fputs(str, m_stream);
   fputc('\n', m_stream);

   return fflush(m_stream);
}
