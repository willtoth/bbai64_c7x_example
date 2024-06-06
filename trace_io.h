#include <file.h>
#include <stdio.h>

#pragma once

#pragma RETAIN(gDebugMemLog)
#define DebugP_MEM_LOG_SIZE 2048
__attribute__((section (".log_shared_mem"))) char gDebugMemLog[DebugP_MEM_LOG_SIZE];

int TRACE_open(const char *path, unsigned flags, int fno) { return 0; }
int TRACE_close(int fno) { return 0; }
int TRACE_read(int fno, char *buffer, unsigned count) { return -1; }

// retarget stdout to remoteproc trace buffer
int TRACE_write(int fno, const char *data, unsigned size) {
    static int idx = 0;
    int count;

    for (count = 0; count < size; count++)
    {
        if (idx > DebugP_MEM_LOG_SIZE)
            idx = 0;

        gDebugMemLog[idx++] = data[count];
    }
    //CacheP_wb(gDebugMemLog, DebugP_MEM_LOG_SIZE, CacheP_TYPE_L1D);

    return count;
}

off_t TRACE_lseek(int fno, off_t offset, int origin) { return 0; }
int TRACE_unlink(const char *path) { return 0; }
int TRACE_rename(const char *old_name, const char *new_name) { return 0; }

void TRACE_add() {
    add_device("trace_device", _MSA, 
        TRACE_open, TRACE_close, 
        TRACE_read, TRACE_write, 
        TRACE_lseek, TRACE_unlink, TRACE_rename);

    freopen("trace_device:stdoutfile", "w", stdout);
    setvbuf(stdout, NULL, _IONBF, 0);
    // freopen("trace_device:stderrfile", "w", stderr);
    // setvbuf(stderr, NULL, _IONBF, 0);
}
