/* <!-- copyright */
/*
 * libmetalink
 *
 * Copyright (c) 2008 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
/* copyright --> */
#include <string.h>
#include <libxml/parser.h>
#include "metalink_parser.h"
#include "metalink_pstm.h"
#include "metalink_pstate.h"
#include "metalink_error.h"
#include "session_data.h"
#include "stack.h"
#include "string_buffer.h"

static void start_element_handler(void* user_data,
				  const xmlChar* name,
				  const xmlChar** attrs)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = new_string_buffer(128);

  /* TODO evaluate return value of stack_push; non-zero value is error. */
  stack_push(session_data->characters_stack, str_buf);

  session_data->stm->state->start_fun(session_data->stm,
				      (const char*)name,
				      (const char**)attrs);
}

static void end_element_handler(void* user_data, const xmlChar* name)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = stack_pop(session_data->characters_stack);
  
  session_data->stm->state->end_fun(session_data->stm,
				    (const char*)name,
				    string_buffer_str(str_buf));

  delete_string_buffer(str_buf);	     
}

static void characters_handler(void* user_data, const xmlChar* chars,
			       int length)
{
  session_data_t* session_data = (session_data_t*)user_data;
  string_buffer_t* str_buf = stack_top(session_data->characters_stack);

  string_buffer_append(str_buf, (const char*)chars, length);
}

static xmlSAXHandler mySAXHandler = {
  0, /* internalSubsetSAXFunc */
  0, /* isStandaloneSAXFunc */
  0, /* hasInternalSubsetSAXFunc */
  0, /* hasExternalSubsetSAXFunc */
  0, /* resolveEntitySAXFunc */
  0, /* getEntitySAXFunc */
  0, /* entityDeclSAXFunc */
  0, /* notationDeclSAXFunc */
  0, /* attributeDeclSAXFunc */
  0, /* elementDeclSAXFunc */
  0, /*   unparsedEntityDeclSAXFunc */
  0, /*   setDocumentLocatorSAXFunc */
  0, /*   startDocumentSAXFunc */
  0, /*   endDocumentSAXFunc */
  &start_element_handler, /*   startElementSAXFunc */
  &end_element_handler, /*   endElementSAXFunc */
  0, /*   referenceSAXFunc */
  &characters_handler, /*   charactersSAXFunc */
  0, /*   ignorableWhitespaceSAXFunc */
  0, /*   processingInstructionSAXFunc */
  0, /*   commentSAXFunc */
  0, /*   warningSAXFunc */
  0, /*   errorSAXFunc */
  0, /*   fatalErrorSAXFunc */
  0, /*   getParameterEntitySAXFunc */
  0, /*   cdataBlockSAXFunc */
  0, /*   externalSubsetSAXFunc */
  0, /*   unsigned int  initialized */
  0, /*   void *        _private */
  0, /*   startElementNsSAX2Func */
  0, /*   endElementNsSAX2Func */
  0, /*   xmlStructuredErrorFunc */
};

int metalink_parse_file(const char* filename, metalink_t** res)
{
  session_data_t* session_data;
  int r;
  int retval;

  session_data = new_session_data();
  
  r = xmlSAXUserParseFile(&mySAXHandler, session_data, filename);

  if(r == 0 && session_data->stm->ctrl->error == 0) {
    *res = metalink_pctrl_detach_metalink(session_data->stm->ctrl);
  }

  if(r != 0) {
    /* TODO more detailed error handling for parser is desired. */
    retval = METALINK_ERR_PARSER_ERROR;
  } else {
    retval = metalink_pctrl_get_error(session_data->stm->ctrl);
  }

  delete_session_data(session_data);

  return retval;
}