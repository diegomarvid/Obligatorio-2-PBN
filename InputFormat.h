#ifndef INPUTFORMAT_H
#define INPUTFORMAT_H

int clean_stdin(void);

pid_t readPID(char msg[]);

int readInt( int min , int max );

char * readCMD(void);

#endif