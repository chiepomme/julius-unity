#pragma once

#if _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

#include "julius/juliuslib.h"

typedef short*(*audio_read_callback_func_type)(int, int*);
typedef void(*debug_log_func_type)(const char*);
typedef void(*result_func_type)(const char*, int);

EXPORT_API void set_audio_callback(audio_read_callback_func_type callback);
EXPORT_API void set_debug_log_func(debug_log_func_type debug_log_func);
EXPORT_API void set_result_func(result_func_type result_func);
EXPORT_API void set_log_to_file(const char* path);
EXPORT_API void set_log_to_stdout(BOOL use_stderr_instead);

EXPORT_API void start(char* jconf_path);
EXPORT_API void finish();

void clean_up_if_exists();
BOOL create_engine();

static void write_result(Recog * recog, void * dummy);
