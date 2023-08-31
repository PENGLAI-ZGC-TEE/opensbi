#ifndef SM_PRINT_H
#define SM_PRINT_H

#include <sbi/sbi_console.h>
#include <sbi/sbi_string.h>

#define PENGLAI_DEBUG

#define debug(M, ...) sbi_printf("[DEBUG] (%s:%d,%s) \n " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define log_err(M, ...) sbi_printf("[ERROR] (%s:%d,%s) \n " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define log_info(M, ...) sbi_printf("[INFO] (%s:%d,%s) \n " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define log_warn(M, ...) sbi_printf("[WARN] (%s:%d,%s) \n " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef PENGLAI_DEBUG
#define printm(...) debug(__VA_ARGS__)
#else
#define printm(...)
#endif

//For report error messages, always enabled
#define printm_err(...) log_err(__VA_ARGS__)

#endif
