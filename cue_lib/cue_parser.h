#pragma once

#include <stdio.h>

struct cue_sheet;
struct line_reader;
struct line_writer;

struct cue_sheet* cue_sheet_parse_file(FILE *fid);
struct cue_sheet* cue_sheet_parse_filename(char const *filename);
struct cue_sheet* cue_sheet_parse(struct line_reader* reader);
void cue_sheet_write_file(struct cue_sheet* sheet, FILE* fid);
void cue_sheet_write_filename(struct cue_sheet* sheet, char const *filename);
void cue_sheet_write(struct cue_sheet* sheet, struct line_writer *writer);
