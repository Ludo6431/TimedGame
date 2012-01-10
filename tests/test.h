#ifndef _TEST_H
#define _TEST_H

#define test(cond) if(!(cond)) { fprintf(stderr, "ERR, condition \""#cond"\" failed !\n"); exit(1); }

#endif

