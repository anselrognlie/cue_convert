#include "cue_convert.h"

#include <stdio.h>

#include "cue_options.h"
#include "err_helpers.h"
#include "mem_helpers.h"
#include "format_helpers.h"
#include "cue_traverse.h"
#include "directory_traversal.h"
#include "file_line_writer.h"
#include "null_line_writer.h"
#include "cue_traverse_report.h"
#include "cue_traverse_report_writer.h"
#include "path.h"

errno_t cue_convert(struct cue_options* opts, cue_convert_env_t* env) {
  errno_t err = 0;
  cue_traverse_visitor_t visitor = { 0 };
  file_line_writer_t file_writer = { 0 };
  file_line_writer_t out_writer = { 0 };
  null_line_writer_t null_writer = { 0 };
  line_writer_i *selected_writer = 0;
  cue_traverse_report_writer_t report_file_writer = { 0 };
  cue_traverse_report_writer_t report_out_writer = { 0 };
  cue_traverse_report_t *report = 0;
  cue_traverse_visitor_opts_t visitor_opts;
  
  ERR_REGION_BEGIN() {
    if (opts->generate_report) {
      ERR_REGION_ERROR_CHECK(file_line_writer_init_path(&file_writer, opts->report_path), err);
      ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_init_params(
        &report_file_writer,
        &file_writer.line_writer), err);
    }

    ERR_REGION_ERROR_CHECK(null_line_writer_init(&null_writer), err);

    if (env->out) {
      file_line_writer_init_fid(&out_writer, env->out);
      selected_writer = &out_writer.line_writer;
    }
    else {
      selected_writer = &null_writer.line_writer;
    }

    if (opts->quiet) {
      selected_writer = &null_writer.line_writer;
    }

    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_init_params(
      &report_out_writer,
      selected_writer), err);

    memset(&visitor_opts, 0, sizeof(visitor_opts));
    visitor_opts.target_path = opts->target_dir;
    visitor_opts.source_path = opts->source_dir;
    visitor_opts.report_only = opts->test_only;
    visitor_opts.writer = selected_writer;

    ERR_REGION_ERROR_CHECK(cue_traverse_visitor_init(
      &visitor,
      &visitor_opts), err);

    traverse_dir_path(opts->source_dir, &visitor.handler_i);

    cue_traverse_report_t* report = visitor.report;

    if (opts->generate_report) {
      ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_write(&report_file_writer, report), err);
    }

    ERR_REGION_ERROR_CHECK(cue_traverse_report_writer_write(&report_out_writer, report), err);

  } ERR_REGION_END()

  cue_traverse_report_writer_uninit(&report_file_writer);
  cue_traverse_report_writer_uninit(&report_out_writer);
  cue_traverse_visitor_uninit(&visitor);
  file_line_writer_uninit(&file_writer);
  file_line_writer_uninit(&out_writer);
  null_line_writer_uninit(&null_writer);

  return err;
}

errno_t cue_convert_with_args(int argc, char const** argv, cue_convert_env_t* env) {
  errno_t err = 0;
  cue_options_t *opts = 0;
  short show_help = 0;

  ERR_REGION_BEGIN() {
    // convert the command line to options
    ERR_REGION_NULL_CHECK(opts = cue_options_alloc(), err);
    ERR_REGION_ERROR_CHECK_CODE(err = cue_options_load_from_args(opts, argc - 1, argv + 1), show_help, 1);
    
    ERR_REGION_ERROR_CHECK(cue_convert(opts, env), err);

  } ERR_REGION_END()

  SAFE_FREE_HANDLER(opts, cue_options_free);

  if (show_help) {
    char const *bin = 0;
    short free_bin = 1;

    bin = path_file_part(argv[0]);
    if (!bin) {
      free_bin = 0;
      bin = argv[0];
    }

    safe_fprintf(env->out, "%s ", bin);
    safe_fprintf(env->out, "%s", cue_options_get_help());

    if (free_bin) {
      SAFE_FREE(bin);
    }
  }

  return err;
}
