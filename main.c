#include <stdint.h>
#include <string.h>
#include <c7x.h>

#include "ipc_rpmsg_linux_resource_table.h"
#include "trace_io.h"

#pragma RETAIN(gRPMessage_linuxResourceTable)
const RPMessage_ResourceTable gRPMessage_linuxResourceTable __attribute__ ((section (".resource_table"), aligned (4096))) =
{
    {
        1U,         /* we're the first version that implements this */
        2U,         /* number of entries, MUST be 2 */
        { 0U, 0U, } /* reserved, must be zero */
    },
    /* offsets to the entries */
    {
        offsetof(RPMessage_ResourceTable, vdev),
        offsetof(RPMessage_ResourceTable, trace),
    },
    /* vdev entry */
    {
        RPMESSAGE_RSC_TYPE_VDEV, RPMESSAGE_RSC_VIRTIO_ID_RPMSG,
        0U, 1U, 0U, 0U, 0U, 2U, { 0U, 0U },
    },
    /* the two vrings */
    { RPMESSAGE_RSC_VRING_ADDR_ANY, 4096U, 256U, 1U, 0U },
    { RPMESSAGE_RSC_VRING_ADDR_ANY, 4096U, 256U, 2U, 0U },
    {
        (RPMESSAGE_RSC_TRACE_INTS_VER0 | RPMESSAGE_RSC_TYPE_TRACE),
        (uint32_t)gDebugMemLog, DebugP_MEM_LOG_SIZE,
        0, "trace:r5f0",
    },
};

int main() {
    TRACE_add();
    printf("Booting\n");
    runDhrystone();
    return 0;
}
