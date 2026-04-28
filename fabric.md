The following are the registers available on the fabric

These are read-only
FABRIC_ID (8 bytes): 
FABRIC_VENDOR (8 bytes): Creator/maker of the Fabric (Writes to this register are ignored)
FABRIC_VERSION (8 bytes): Version (Writes to this register are ignored)

ECAM_BASE_ADDR (8 bytes): Base Address of ECAM
ECAM_SIZE_MASK (8 bytes): Size mask for ECAM Region

I2C_BASE_ADDR (8 bytes): Base Address of I2C
I2C_SIZE_MASK (8 bytes): Size mask for I2C

ROM_BASE_ADDR (8 bytes): Base Address of ROM
ROM_SIZE_MASK (8 bytes): Size mask for ROM region

RAM_BASE_ADDR (8 bytes): Base Address for RAM
RAM_SIZE_ADDR (8 bytes): Size mask for RAM

STATUS:  (2 bytes): Read here to fetch status
COMMAND: (2 bytes): Command register. Write here to send commands

STATUS flags
bit 0 = BUSY - Set for busy, clear for ready
bit 1 = ERROR - Set for error, clear for no error

COMMAND:
COMMAND flags
bit 0 = ECAM - Set to enable routing to ECAM
bit 1 = I2C - Set to enable routing to I2C
bit 2 = RAM - Set to enable routing to RAM


Each route entry in the routing table has:
Base Address // Base address of the region
Size Mask // Size mask
Read-Only // The region is readonly
enabled  // Routing is enabled to this region