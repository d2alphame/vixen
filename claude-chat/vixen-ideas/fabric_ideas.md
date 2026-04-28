Offset  Name                Access   Description
──────────────────────────────────────────────────────────────────
0x000   FABRIC_ID           R/O      magic number 0x46425249
0x004   FABRIC_VERSION      R/O      version e.g. 0x00010000
0x008   FABRIC_STATUS       R/O      current status bits
0x00C   FABRIC_ERR_ADDR_LO  R/O      low 32 bits of fault address
0x010   FABRIC_ERR_ADDR_HI  R/O      high 32 bits of fault address
0x014   FABRIC_ERR_STATUS   R/W      fault details, write 1 to clear
0x018   FABRIC_TIMEOUT      R/W      bus timeout in cycles

0x020   FABRIC_RAM_BASE_LO  R/W      low 32 bits of RAM base
0x024   FABRIC_RAM_BASE_HI  R/W      high 32 bits of RAM base
0x028   FABRIC_RAM_SIZE_LO  R/W      low 32 bits of RAM size
0x02C   FABRIC_RAM_SIZE_HI  R/W      high 32 bits of RAM size
0x030   FABRIC_RAM_ENABLE   R/W      bit 0 = enable RAM routing

0x040   FABRIC_ECAM_BASE_LO R/W      low 32 bits of ECAM base
0x044   FABRIC_ECAM_BASE_HI R/W      high 32 bits of ECAM base
0x048   FABRIC_ECAM_SIZE    R/W      ECAM window size
0x04C   FABRIC_ECAM_ENABLE  R/W      bit 0 = enable ECAM routing