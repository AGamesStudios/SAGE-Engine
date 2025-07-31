#ifndef SAGE_GF_ERRORS_H
#define SAGE_GF_ERRORS_H

#define GF_FAC_CORE    0x01
#define GF_FAC_FORMULA 0x02
#define GF_FAC_STREAM  0x03
#define GF_FAC_FORMAT  0x04
#define GF_FAC_CLI     0x05

#define GF_MOD_MATH 0x01
#define GF_MOD_VM   0x02
#define GF_MOD_IPC  0x03
#define GF_MOD_SFS  0x04

#define GF_E(e) (e)
#define GF_ERR(f,m,c) (((f)<<24)|((m)<<16)|(c))

enum {
    GF_OK      = 0,
    GF_EINVAL  = 0x0001,
    GF_ENOMEM  = 0x0002,
    GF_EIO     = 0x0003,
    GF_ECRC    = 0x0004,
    GF_ECAP    = 0x0005,
    GF_ESTATE  = 0x0006,
    GF_EFORM   = 0x0007,
    GF_EPARSE  = 0x0008,
    GF_EOVER   = 0x0009,
    GF_ETIME   = 0x000A
};

#endif // SAGE_GF_ERRORS_H
