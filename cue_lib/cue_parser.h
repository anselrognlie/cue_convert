#pragma once

#include <stdio.h>

#include "object_vector.h"

struct cue_sheet;
struct line_reader;
struct line_writer;

struct cue_sheet_parse_result;

struct cue_sheet* cue_sheet_parse_file(FILE *fid, struct cue_sheet_parse_result *out);
struct cue_sheet* cue_sheet_parse_filename(char const *filename, struct cue_sheet_parse_result* out);
struct cue_sheet* cue_sheet_parse(struct line_reader* reader, struct cue_sheet_parse_result* out);
void cue_sheet_write_file(struct cue_sheet* sheet, FILE* fid);
void cue_sheet_write_filename(struct cue_sheet* sheet, char const *filename);
void cue_sheet_write(struct cue_sheet* sheet, struct line_writer *writer);
