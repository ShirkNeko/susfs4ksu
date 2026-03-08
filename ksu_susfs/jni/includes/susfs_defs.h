#ifndef SUSFS_DEFS_H
#define SUSFS_DEFS_H

/*************************
 ** Define Const Values **
 *************************/
#define TAG "ksu_susfs"
#define KSU_INSTALL_MAGIC1 0xDEADBEEF
#define SUSFS_MAGIC 0xFAFAFAFA

#define SUSFS_MAX_LEN_PATHNAME 256

/******************
 ** Define Macro **
 ******************/
#define ERR_CMD_NOT_SUPPORTED 126
#define log(fmt, msg...) printf(fmt, ##msg);
#define PRT_MSG_IF_CMD_NOT_SUPPORTED(x, cmd) if (x == ERR_CMD_NOT_SUPPORTED) log("[-] CMD: '0x%x', SUSFS operation not supported, please enable it in kernel\n", cmd)

#endif // #ifndef SUSFS_DEFS_H
