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
#ifndef _D_METALINK_PCTRL_H_
#define _D_METALINK_PCTRL_H_

#include "metalink_types.h"
#include "list.h"

typedef struct metalink_pctrl_t {
  int error;

  metalink_t* metalink;

  list_t/* <metalink_file_t*> */ * files;

  metalink_file_t* temp_file;

  list_t/* <metalink_resource_t*> */ * resources;

  metalink_resource_t* temp_resource;

  list_t/* <metalink_checksum_t*> */ * checksums;

  metalink_checksum_t* temp_checksum;

  metalink_chunk_checksum_t* temp_chunk_checksum;

  list_t/* <metalink_piece_hash_t*> */ * piece_hashes;

  metalink_piece_hash_t* temp_piece_hash;

} metalink_pctrl_t;

metalink_pctrl_t* new_metalink_pctrl();

void delete_metalink_pctrl(metalink_pctrl_t* ctrl);

/**
 * detach metalink member: return ctrl->metalink and set NULL to
 * ctrl->metalink.
 */
metalink_t* metalink_pctrl_detach_metalink(metalink_pctrl_t* ctrl);

/* Set error code */
void metalink_pctrl_set_error(metalink_pctrl_t* ctrl, int error);

int metalink_pctrl_get_error(metalink_pctrl_t* ctrl);

/* metalink manipulation functions */
int metalink_pctrl_metalink_accumulate_files(metalink_pctrl_t* ctrl);

/* transaction functions */
metalink_file_t* metalink_pctrl_new_file_transaction(metalink_pctrl_t* ctrl);

int metalink_pctrl_commit_file_transaction(metalink_pctrl_t* ctrl);

metalink_resource_t* metalink_pctrl_new_resource_transaction(metalink_pctrl_t* ctrl);

int metalink_pctrl_commit_resource_transaction(metalink_pctrl_t* ctrl);

metalink_checksum_t* metalink_pctrl_new_checksum_transaction(metalink_pctrl_t* ctrl);

int metalink_pctrl_commit_checksum_transaction(metalink_pctrl_t* ctrl);

metalink_chunk_checksum_t* metalink_pctrl_new_chunk_checksum_transaction(metalink_pctrl_t* ctrl);

int metalink_pctrl_commit_chunk_checksum_transaction(metalink_pctrl_t* ctrl);

metalink_piece_hash_t* metalink_pctrl_new_piece_hash_transaction(metalink_pctrl_t* ctrl);

int metalink_pctrl_commit_piece_hash_transaction(metalink_pctrl_t* ctrl);

/* file manipulation functions*/
int metalink_pctrl_file_set_name(metalink_pctrl_t* ctrl, const char* name);

void metalink_pctrl_file_set_size(metalink_pctrl_t* ctrl, long long int size);

int metalink_pctrl_file_set_version(metalink_pctrl_t* ctrl, const char* version);

int metalink_pctrl_file_set_language(metalink_pctrl_t* ctrl, const char* language);

int metalink_pctrl_file_set_os(metalink_pctrl_t* ctrl, const char* os);

void metalink_pctrl_file_set_maxconnections(metalink_pctrl_t* ctrl, int maxconnections);

/* resource manipulation functions */
int metalink_pctrl_resource_set_type(metalink_pctrl_t* ctrl, const char* type);

int metalink_pctrl_resource_set_location(metalink_pctrl_t* ctrl,
					 const char* location);

void metalink_pctrl_resource_set_preference(metalink_pctrl_t* ctrl,
					    int preference);

void metalink_pctrl_resource_set_maxconnections(metalink_pctrl_t* ctrl,
						int maxconnections);

int metalink_pctrl_resource_set_url(metalink_pctrl_t* ctrl, const char* url);

/* checksum manipulation functions */
int metalink_pctrl_checksum_set_type(metalink_pctrl_t* ctrl, const char* type);

int metalink_pctrl_checksum_set_hash(metalink_pctrl_t* ctrl, const char* hash);

/* piece hash manipulation functions */
void metalink_pctrl_piece_hash_set_piece(metalink_pctrl_t* ctrl, int piece);

int metalink_pctrl_piece_hash_set_hash(metalink_pctrl_t* ctrl, const char* hash);

/* chunk checksum manipulation functions */
int metalink_pctrl_chunk_checksum_set_type(metalink_pctrl_t* ctrl, const char* type);

void metalink_pctrl_chunk_checksum_set_length(metalink_pctrl_t* ctrl, int length);

/* Unlike other mutator functions, this function doesn't create copy of
   piece_hashes. So don't free piece_hashes manually after this call.*/
void metalink_pctrl_chunk_checksum_set_piece_hashes(metalink_pctrl_t* ctrl,
						    metalink_piece_hash_t** piece_hashes);

#endif // _D_METALINK_PCTRL_H_