#ifndef _ERROR_H_
#define _ERROR_H_

const char* get_error_string (int code);

// Like, global error (variable)
extern int gerror;

#define ERRNOMEM 1
#define ERRWRONGSIZE 2
#define ERRNOACC 3
#define ERRUNKNOWN 4
#define ERRMAX 4

#endif
