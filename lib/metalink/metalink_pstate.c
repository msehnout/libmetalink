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
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "metalink_pstate.h"
#include "metalink_pstm.h"
#include "metalink_error.h"

metalink_pstate_t* new_metalink_pstate()
{
  metalink_pstate_t* state;
  state = malloc(sizeof(metalink_pstate_t));
  if(state) {
    memset(state, 0, sizeof(metalink_pstate_t));
  }
  return state;
}

void delete_metalink_pstate(metalink_pstate_t* state)
{
  free(state);
}

const char* get_attribute_value(const char** attrs, const char* name)
{
  const char** p;
  const char* cname;

  if(attrs == NULL) {
    return NULL;
  }

  p = attrs;
  while(*p) {
    cname = *p++;
    if(*p == 0) {
      break;
    }
    if(strcmp(cname, name) == 0) {
      break;
    } else {
      ++p;
    }
  }
  return *p;
}

/**
 * set error code to metalink_pctrl and transit to null state, where no further
 * state transition takes place.
 */
void error_handler(metalink_pstm_t* stm, int error)
{
  metalink_pctrl_set_error(stm->ctrl, error);
  metalink_pstm_enter_null_state(stm);
}


/* null handler doing nothing */
void null_state_start_fun(metalink_pstm_t* stm,
			  const char* name,
			  const char** attrs) {}

void null_state_end_fun(metalink_pstm_t* stm,
			const char* name,
			const char* characters) {}

/* initial state */
void initial_state_start_fun(metalink_pstm_t* stm,
			     const char* name, const char** attrs)
{
  if(strcmp("metalink", name) == 0) {
    metalink_pstm_enter_metalink_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
}

void initial_state_end_fun(metalink_pstm_t* stm,
			   const char* name, const char* characters) {}

/* metalink state <metalink> */
void metalink_state_start_fun(metalink_pstm_t* stm,
			      const char* name, const char** attrs)
{
  if(strcmp("files", name) == 0) {
    metalink_pstm_enter_files_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
}

void metalink_state_end_fun(metalink_pstm_t* stm,
			    const char* name, const char* characters)
{
  metalink_pstm_enter_fin_state(stm);
}

/* files state <files> */
void files_state_start_fun(metalink_pstm_t* stm,
			   const char* name, const char** attrs)
{
  int r;
  if(strcmp("file", name) == 0) {
    const char* fname;
    metalink_file_t* file;

    fname = get_attribute_value(attrs, "name");
    if(!fname) {
      /* name is required attribute. Skip this entry. */
      metalink_pstm_enter_skip_state(stm);
      return;
    }

    file = metalink_pctrl_new_file_transaction(stm->ctrl);
    if(!file) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    r = metalink_pctrl_file_set_name(stm->ctrl, fname);
    if(r != 0) {
      error_handler(stm, r);
      return;
    }
    metalink_pstm_enter_file_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
}

void files_state_end_fun(metalink_pstm_t* stm,
			 const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_metalink_accumulate_files(stm->ctrl);
  if(r == 0) {
    metalink_pstm_enter_metalink_state(stm);
  } else {
    error_handler(stm, r);
  }
}

/* file state <file> */
void file_state_start_fun(metalink_pstm_t* stm,
			  const char* name, const char** attrs)
{
  if(strcmp("size", name) == 0) {
    metalink_pstm_enter_size_state(stm);
  } else if(strcmp("version", name) == 0) {
    metalink_pstm_enter_version_state(stm);
  } else if(strcmp("language", name) == 0) {
    metalink_pstm_enter_language_state(stm);
  } else if(strcmp("os", name) == 0) {
    metalink_pstm_enter_os_state(stm);
  } else if(strcmp("verification", name) == 0) {
    metalink_pstm_enter_verification_state(stm);
  } else if(strcmp("resources", name) == 0) {
    const char* value;
    int maxconnections = 0;

    value = get_attribute_value(attrs, "maxconnections");
    if(value) {
      maxconnections = strtol(value, 0, 10);
      if((errno == ERANGE && maxconnections == LONG_MAX) || maxconnections < 0) {
	/* error, maxconnection is not positive integer. */
	maxconnections = 0;
      }
    }
    metalink_pctrl_file_set_maxconnections(stm->ctrl, maxconnections);

    metalink_pstm_enter_resources_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
}

void file_state_end_fun(metalink_pstm_t* stm,
			const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_commit_file_transaction(stm->ctrl);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  metalink_pstm_enter_files_state(stm);
}

/* size state <size> */
void size_state_start_fun(metalink_pstm_t* stm,
			  const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void size_state_end_fun(metalink_pstm_t* stm,
			const char* name, const char* characters)
{
  long long int size = 0;

  /* TODO evaluate endptr(2nd argument) */
  size = strtoll(characters, 0, 10);
  if((errno == ERANGE && size == LLONG_MAX) || size < 0) {
    /* overflow or parse error or negative integer detected. */
    /* current Metalink specification does not require size. */
    size = 0;
  }
  metalink_pctrl_file_set_size(stm->ctrl, size);

  metalink_pstm_enter_file_state(stm);
}

/* version state <version> */
void version_state_start_fun(metalink_pstm_t* stm,
			     const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void version_state_end_fun(metalink_pstm_t* stm,
			   const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_file_set_version(stm->ctrl, characters);
  if(r == 0) {
    metalink_pstm_enter_file_state(stm);
  } else {
    error_handler(stm, r);
  }
}

/* language state <language> */
void language_state_start_fun(metalink_pstm_t* stm,
			      const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void language_state_end_fun(metalink_pstm_t* stm,
			    const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_file_set_language(stm->ctrl, characters);
  if(r == 0) {
    metalink_pstm_enter_file_state(stm);
  } else {
    error_handler(stm, r);
  }
}

/* os state <os> */
void os_state_start_fun(metalink_pstm_t* stm,
			const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void os_state_end_fun(metalink_pstm_t* stm,
		      const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_file_set_os(stm->ctrl, characters);
  if(r == 0) {
    metalink_pstm_enter_file_state(stm);
  } else {
    error_handler(stm, r);
  }
}

/* resources state <resources> */
void resources_state_start_fun(metalink_pstm_t* stm,
			       const char* name, const char** attrs)
{
  int r;
  if(strcmp("url", name) == 0) {
    const char* type;
    const char* location;
    const char* value;
    int preference = 0;
    int maxconnections = 0;
    metalink_resource_t* resource;

    resource = metalink_pctrl_new_resource_transaction(stm->ctrl);
    if(!resource) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }

    type = get_attribute_value(attrs, "type");
    if(!type) {
      /* type attribute is required, but not found. Skip current url tag. */
      metalink_pstm_enter_skip_state(stm);
      return;
    }
    r = metalink_pctrl_resource_set_type(stm->ctrl, type);
    if(r != 0) {
      error_handler(stm, r);
      return;
    }

    location = get_attribute_value(attrs, "location");
    if(location) {
      r = metalink_pctrl_resource_set_location(stm->ctrl, location);
      if(r != 0) {
	error_handler(stm, r);
	return;
      }
    }

    value = get_attribute_value(attrs, "preference");
    if(value) {
      preference = strtol(value, 0, 10);
      if((errno == ERANGE && preference == LONG_MAX) || preference < 0) {
	/* error, preference is not positive integer. */
	preference = 0;
      }
    }
    metalink_pctrl_resource_set_preference(stm->ctrl, preference);

    value = get_attribute_value(attrs, "maxconnections");
    if(value) {
      maxconnections = strtol(value, 0, 10);
      if((errno == ERANGE && maxconnections == LONG_MAX) || maxconnections < 0) {
	/* error, maxconnections is not positive integer. */
	maxconnections = 0;
      }
    }
    metalink_pctrl_resource_set_maxconnections(stm->ctrl, maxconnections);

    metalink_pstm_enter_url_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }

}

void resources_state_end_fun(metalink_pstm_t* stm,
			     const char* name, const char* characters)
{
  metalink_pstm_enter_file_state(stm);
}

/* url state <url> */
void url_state_start_fun(metalink_pstm_t* stm,
			 const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void url_state_end_fun(metalink_pstm_t* stm,
		       const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_resource_set_url(stm->ctrl, characters);
  if(r != 0) {
    /* TODO clear intermidiate resource transaction. */
    error_handler(stm, r);
    return;
  }
  r = metalink_pctrl_commit_resource_transaction(stm->ctrl);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  metalink_pstm_enter_resources_state(stm);
}

/* verification state <verification> */
void verification_state_start_fun(metalink_pstm_t* stm,
				  const char* name, const char** attrs)
{
  int r;
  if(strcmp("hash", name) == 0) {
    const char* type;
    metalink_checksum_t* checksum;

    type = get_attribute_value(attrs, "type");
    if(!type) {
      /* type is required attribute, if not specified, then skip this tag */
      metalink_pstm_enter_skip_state(stm);
      return;
    }
    checksum = metalink_pctrl_new_checksum_transaction(stm->ctrl);
    if(!checksum) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    r = metalink_checksum_set_type(checksum, type);
    if(r != 0) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    metalink_pstm_enter_hash_state(stm);
  } else if(strcmp("pieces", name) == 0) {
    const char* type;
    const char* value;
    int length;
    metalink_chunk_checksum_t* chunk_checksum;

    type = get_attribute_value(attrs, "type");
    if(!type) {
      /* type is required attribute, so if not specified, then skip this tag. */
      metalink_pstm_enter_skip_state(stm);
      return;
    }
    
    value = get_attribute_value(attrs, "length");
    if(value) {
      length = strtol(value, 0, 10);
      if((errno == ERANGE && length == LONG_MAX) || length < 0) {
	/* error, length is not positive integer. */
	/* length is required attribute, so if not specified, then skip this tag*/
	metalink_pstm_enter_skip_state(stm);
	return;
      }
    } else {
      /* length is required attribute, so if not specified, then skip this tag*/
      metalink_pstm_enter_skip_state(stm);
      return;
    }

    chunk_checksum = metalink_pctrl_new_chunk_checksum_transaction(stm->ctrl);
    if(!chunk_checksum) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    r = metalink_chunk_checksum_set_type(chunk_checksum, type);
    if(r != 0) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    metalink_chunk_checksum_set_length(chunk_checksum, length);

    metalink_pstm_enter_pieces_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
}

void verification_state_end_fun(metalink_pstm_t* stm,
				const char* name, const char* characters)
{
  metalink_pstm_enter_file_state(stm);
}

/* hash state <hash> */
void hash_state_start_fun(metalink_pstm_t* stm,
			 const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void hash_state_end_fun(metalink_pstm_t* stm,
			const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_checksum_set_hash(stm->ctrl, characters);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  r = metalink_pctrl_commit_checksum_transaction(stm->ctrl);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  metalink_pstm_enter_verification_state(stm);
}

/* pieces state <pieces> */
void pieces_state_start_fun(metalink_pstm_t* stm,
			    const char* name, const char** attrs)
{
  if(strcmp("hash", name) == 0) {
    const char* value;
    int piece;
    metalink_piece_hash_t* piece_hash;

    value = get_attribute_value(attrs, "piece");
    if(value) {
      piece = strtol(value, 0, 10);
      if((errno == ERANGE && piece == LONG_MAX) || piece < 0) {
	/* error, piece is not positive integer. */
	/* piece is required attribute, but it is missing. Skip this tag. */
	metalink_pstm_enter_skip_state(stm);
	return;      
      }
    } else {
      /* value is required attribute, but it is missing. Skip this tag. */
      metalink_pstm_enter_skip_state(stm);
      return;      
    }
    
    piece_hash = metalink_pctrl_new_piece_hash_transaction(stm->ctrl);
    if(!piece_hash) {
      error_handler(stm, METALINK_ERR_BAD_ALLOC);
      return;
    }
    metalink_pctrl_piece_hash_set_piece(stm->ctrl, piece);

    metalink_pstm_enter_piece_hash_state(stm);
  } else {
    metalink_pstm_enter_skip_state(stm);
  }
  
}

void pieces_state_end_fun(metalink_pstm_t* stm,
			  const char* name, const char* characters)
{
  int r;
  r = metalink_pctrl_commit_chunk_checksum_transaction(stm->ctrl);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  metalink_pstm_enter_verification_state(stm);
}

/* piece hash state <hash> inside of <pieces> */
void piece_hash_state_start_fun(metalink_pstm_t* stm,
				const char* name, const char** attrs)
{
  metalink_pstm_enter_skip_state(stm);
}

void piece_hash_state_end_fun(metalink_pstm_t* stm,
			      const char* name, const char* characters)
{
  int r;
  metalink_pctrl_piece_hash_set_hash(stm->ctrl, characters);
  r = metalink_pctrl_commit_piece_hash_transaction(stm->ctrl);
  if(r != 0) {
    error_handler(stm, r);
    return;
  }
  metalink_pstm_enter_pieces_state(stm);
}

/* fin state */
void fin_state_start_fun(metalink_pstm_t* stm,
			 const char* name, const char** attrs) {}

void fin_state_end_fun(metalink_pstm_t* stm,
		       const char* name, const char* characters) {}

/* skip state */
void skip_state_start_fun(metalink_pstm_t* stm,
			  const char* name, const char** attrs)
{
  ++stm->state->skip_depth;
}

void skip_state_end_fun(metalink_pstm_t* stm,
			const char* name, const char* characters)
{
  if(--stm->state->skip_depth == 0) {
    metalink_pstm_exit_skip_state(stm);
  }
}