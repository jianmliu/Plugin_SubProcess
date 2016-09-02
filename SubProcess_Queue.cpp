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

/* SubProcess_Queue::clear: clear queue  */
void SubProcess_Queue::clear()
{
   if(m_last != NULL) {
      Cell *cell = m_last, *next;

      do {
         next = cell->next;

         /* free unused events */
         free(cell->type);
         free(cell->args);
         delete cell;

         cell = next;
      } while(cell != m_last);

      m_last = NULL;
   }
}

/* SubProcess_Queue::SubProcess_Queue: constructor */
SubProcess_Queue::SubProcess_Queue()
{
   m_last = NULL;
}

/* SubProcess_Queue::~SubProcess_Queue: destructor */
SubProcess_Queue::~SubProcess_Queue()
{
   clear();
}

/* SubProcess_Queue::enqueue: enqueue */
void SubProcess_Queue::enqueue(const char *type, const char *args)
{
   Cell *cell = new Cell;

   cell->type = MMDAgent_strdup((type != NULL) ? type : "");
   cell->args = MMDAgent_strdup((args != NULL) ? args : "");

   if(m_last == NULL)
      cell->next = cell;
   else {
      cell->next = m_last->next;
      m_last->next = cell;
   }

   m_last = cell;
}

/* SubProcess_Queue::dequeue: dequeue */
void SubProcess_Queue::dequeue(char **type, char **args)
{
   if(m_last == NULL) {
      *type = MMDAgent_strdup("");
      *args = MMDAgent_strdup("");
   }
   else {
      Cell *top = m_last->next;

      *type = top->type;
      *args = top->args;

      if(m_last == top)
         m_last = NULL;
      else
         m_last->next = top->next;

      delete top;
   }
}

/* SubProcess_Queue::isEmpty: check empty */
bool SubProcess_Queue::isEmpty()
{
   return (m_last == NULL) ? true : false;
}
