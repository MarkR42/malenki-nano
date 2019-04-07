// Write a string
void diag_puts(const char *str);

// Use the gcc extension to check format strings.
void diag_println(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
void diag_print(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));
