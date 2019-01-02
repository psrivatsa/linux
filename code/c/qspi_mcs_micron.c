// ============================================================================
// In System Programming to program MICRON DUAL SPI Flash with MCS files
// ----------------------------------------------------------------------------
// Authour : psrivatsa 
// Date    : Sep 3, 2018
// 
// ============================================================================

#define _GNU_SOURCE
#if __STDC_VERSION__ >= 201112L
#   define _XOPEN_SOURCE 700
#elif __STDC_VERSION__ >= 199901L
#   define _XOPEN_SOURCE 600
#else
#   define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "gdf_glb.h"

// ============================================================================
// Defines for QSPI Address Locations
// ============================================================================
#define QSPI_SPISRR_ADDR    0x2040 // SPI Software Reset Register
#define QSPI_SPICR_ADDR     0x2060 // SPI Control Register
#define QSPI_SPISR_ADDR     0x2064 // SPI Status Register
#define QSPI_SPIDTR_ADDR    0x2068 // SPI Data Transmit Register
#define QSPI_SPIDRR_ADDR    0x206C // SPI Data Receive Register
#define QSPI_SPISSR_ADDR    0x2070 // SPI Slave Select Register
#define QSPI_SPITFOR_ADDR   0x2074 // SPI Transmit FIFO Occupancy Register
#define QSPI_SPIRFOR_ADDR   0x2078 // SPI Receive FIFO Occupancy Register

#define RD_STATUS_REG       0x05   // READ STATUS REGISTER
#define RD_FLAG_STATUS_REG  0x70   // READ FLAG STATUS REGISTER
#define RD_EXT_ADDR_REG     0xC8   // READ EXTENDED ADDRESS REGSITER

#define MAX_SECTORS         256    // Current design fits in 256 sectors
// ============================================================================
// Global Variable. 
// ============================================================================
int fd_dev = 0;
FILE *fptr_mcs0;
FILE *fptr_mcs1;

// ============================================================================
// Print LINE Function
// ============================================================================
print_line () {
  printf("=============================================================\n");
}

// ============================================================================
// Function to read FLASH DEVICE ID (9Eh)
// ============================================================================
read_device_id (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;

// ============================================================================
  // Read Device ID
// ============================================================================
  print_line();
  if (id == 0) {
    printf(" -->> Reading DEVICE ID of Primary Flash (Slave0)\n");
  } else {
    printf(" -->> Reading DEVICE ID of Secondary Flash (Slave1)\n");
  }
  print_line();

  // Software Reset of QSPI Controller
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISRR_ADDR, 0xA);

  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load Device ID READ Command with 3 Dummy's
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x9E);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    // printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);

  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

// ============================================================================
  // Read Data from RX FIFO
// ============================================================================
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  // Device Manufacturer
  if(vr == 0x20){
    printf(" -> DEVICE ID[0] = 0x%X; MICRON DEVICE\n", vr);
  } else {
    printf(" -> DEVICE ID[0] = 0x%X; UNKNOWN DEVICE\n", vr);
    exit(1);
  }
  
  // Device Voltage
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  if(vr == 0xBB){
    printf(" -> DEVICE ID[1] = 0x%X; 1.8V\n", vr);
  } else {
    printf(" -> DEVICE ID[1] = 0x%X; 3V\n", vr);
  }

  // Device Size
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  if(vr == 0x21){
    printf(" -> DEVICE ID[2] = 0x%X; 1Gb MICRON DEVICE\n", vr);
  } else {
    printf(" -> DEVICE ID[2] = 0x%X; UNKNOWN SIZE MICRON DEVICE\n", vr);
  }
}
// ============================================================================
// Function to read Registers (05h/70h/C8h)
// 1. STATUS REGISTER
// 2. FLAG STATUS REGISTER
// 3. EXTENDED ADDRESS REGISTER
// ============================================================================
uint32_t read_register (int id, uint32_t RAddr, char msg[], int PrintVal) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;

// ============================================================================
  // Read Status Register ID
// ============================================================================
  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load Device READ Status Register Command with 1 Dummy
  // vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x05);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, RAddr);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    // printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);

  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

// ============================================================================
  // Read Data from RX FIFO
// ============================================================================
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  // Status Register
  if (PrintVal == 1) {
    printf("%-30s = 0x%X; \n", msg, vr);
  }
  
  return (vr);
}
// ============================================================================
// Function to Enable Writes (06h)
// ============================================================================
write_enable (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;

// ============================================================================
  // Write Enable to be set
// ============================================================================
  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load WRITE_ENABLE Register Command with 1 Dummy
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x06);

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    //printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);

  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

}  
// ============================================================================
// Function to Disable Writes (04h)
// ============================================================================
write_disable (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;

// ============================================================================
  // Write Enable to be set
// ============================================================================
  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load WRITE_DISABLE Register 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x04);

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    //printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);

  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

}

// ============================================================================
// Function to do DIE ERASE Sector by Sector
// BULK ERASE (C4h) is not supported in the QUAD SPI IP
// ============================================================================
erase (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;
  uint32_t sector;

  if (id == 0) {
    print_line();
    printf(" -->> ERASE Primary Flash Sector by Sector \n");
    print_line();
  } else {
    print_line();
    printf(" -->> ERASE Secondary Flash Sector by Sector \n");
    print_line();
  }

// ============================================================================
  // DIE ERASE Command
// ============================================================================
  for (sector = 0; sector < MAX_SECTORS; sector++) { // Restrict to 256 Sectors

    // ============================================================================
      // Write Enable to be set
    // ============================================================================
    write_enable(id);
    printf(" -> Erasing SECTOR %04d .. ", sector);

    // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
    vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
          
    // Load Sector ERASE Command 
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0xD8);
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, sector);
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x00);
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x00);
  
    // Enable Slave 
    if (id == 0) {
      vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
    } else {
      vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
    }
  
    // Remove Master Inhibit to start Transaction
    vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);
  
    // Wait for SPI Status Register to indicate Tx Empty
    while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
      //printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
      usleep(1);
    }
  
    // Disable Slave 0 
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);
  
    // Enable Master Inhibit to stop Transaction
    vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);
  
    // Wait for DIE TO be ERASED. Poll 
    while((read_register(id, RD_STATUS_REG, "STATUS_REGISTER", 0) & 0x3) == 0x3) {
      // printf("Waiting for DIE ERASE ... ");
      usleep(1000); // wait for 1ms. Given in us
    }
    // Check Flag Status Register
    vr = read_register(id, RD_FLAG_STATUS_REG , "FLAG STATUS REGISTER", 0);
    if (vr != 0x80) {
      printf(" --->>> FATAL :: ERROR ERASING FLASH MEMORY\n");
      exit(1);
    }
    printf("Done\n");
    // ============================================================================
      // Write Disable after ERASE
    // ============================================================================
    write_disable(id);

  } // end for
}

// ============================================================================
// Function to do FAST READ 0Bh for 'n' bytes from Given Address
// ============================================================================
read_fast_nbytes (uint32_t RAddr, int n, int id, uint32_t *RData) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t vr = 0;
  int k;

  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load FAST READ (0Bh) Command with n Dummy's
  // Command
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0B);
  // 24-bit Addr
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (RAddr & 0xFF0000) >> 16);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (RAddr & 0xFF00) >> 8);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (RAddr & 0xFF));
  for (k = 0; k < n; k++) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);
  }
  // Extra dummy to account for 8 dummy cycles
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x0);

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    //printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);
  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

  // Read Data from RX FIFO
  // Ignore First 4 bytes
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  // 8 clock cycles dummy data. So, ignore 1 more byte
  vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
  // Actual Data
  for (k = 0; k < n; k++) {
    vr = gdfglb_ioread(fd_dev, QSPI_SPIDRR_ADDR);
    *RData++ = vr;
//      if (k%16 == 0) {
//        printf("0x%08X : ", RAddr + k);
//      }
//      printf("%02X", vr);
//      if (k%16 == 15) {
//        printf("\n");
//      }
  } // end for

}

// ============================================================================
// Function to do PROGRAM 02h for 'n' bytes from Given Address
// ============================================================================
program_nbytes (uint32_t WAddr, int n, int id, uint32_t *WData) {

  // ============================================================================
    // Local Variables
  // ============================================================================
  uint32_t vr = 0;
  int k;

  // ============================================================================
    // Write Enable to be set
  // ============================================================================
  write_enable(id);

  // Reset Rx/Tx FIFO. Set CPOL/CPHA = 11, Set IP Enable
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x1FE);
        
  // Load PROGRAM (02h) Command with n Data bytes
  // Command
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, 0x02);
  // 24-bit Addr
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (WAddr & 0xFF0000) >> 16);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (WAddr & 0xFF00) >> 8);
  vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, (WAddr & 0xFF));
  for (k = 0; k < n; k++) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPIDTR_ADDR, *WData++); // Data
  }

  // Enable Slave 
  if (id == 0) {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x2); // Primary
  } else {
    vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x1); // Secondary
  }

  // Remove Master Inhibit to start Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x9E);

  // Wait for SPI Status Register to indicate Tx Empty
  while((gdfglb_ioread(fd_dev, QSPI_SPISR_ADDR) & 0x4) == 0x0) {
    //printf("Waiting for Tx_Empty => Addr: 0x%X: readback 0x%X\n", QSPI_SPISR_ADDR, vr);
    usleep(1);
  }

  // Disable Slave 0 
  vr = gdfglb_iowrite(fd_dev, QSPI_SPISSR_ADDR, 0x3);
  // Enable Master Inhibit to stop Transaction
  vr = gdfglb_iowrite(fd_dev, QSPI_SPICR_ADDR, 0x19E);

  // Wait for PROGRAM to complete. Poll 
  while((read_register(id, RD_STATUS_REG, "STATUS_REGISTER", 0) & 0x3) == 0x3) {
    //printf("Waiting for PROGRAM COMPLETION ... ");
    usleep(1); // wait for 1us. Given in us
  }
  // Check Flag Status Register
  vr = read_register(id, RD_FLAG_STATUS_REG , "FLAG STATUS REGISTER", 0);
  if (vr != 0x80) {
    printf("  --->>> FATAL :: ERROR ERASING FLASH MEMORY\n");
    exit(1);
  }
  // ============================================================================
    // Write Disable after PROGRAM
  // ============================================================================
  write_disable(id);

}

// ============================================================================
// Function to do BLANK Check
// ============================================================================
blank_check (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t BAddr;                   // Base Address or Sector Number (64KB Sectors)
  uint32_t Addr;                    // Address for Data line
  uint32_t RAddr;                   // Final Address for READ command
  uint32_t lines;                   // Each line is 16-bytes
  uint32_t sector;                  // 1Gb(128MB) memory has 2K, 64KB sectors
  uint32_t RSize;
  uint32_t RData[16];
  int k;

// ============================================================================
  // Read And Verify Sector by Sector
// ============================================================================
  print_line();
  if (id == 0) {
    printf(" -->> BLANK CHECK Primary Flash !!! \n");
  } else {
    printf(" -->> BLANK CHECK Secondary Flash !!! \n");
  }  
  print_line();

  // ==========================================================================
  // Read and Compare sector-by-sector and line-by-line
  // ==========================================================================
  BAddr = 0x0; // Start Address init
  Addr  = 0x0; // Address init
  RSize = 16;  // Always 16 bytes read
  for (sector = 0; sector < MAX_SECTORS; sector++) { // Currently 256 Sectors
   
    // Set Base Address 
    BAddr = (sector) << 16;
    // ========================================================================
      // Line by Line check of 16-bytes
    // ========================================================================
    printf(" -> Blank check SECTOR %04d; Start Address 0x%08X .. ", sector, BAddr);
    // These should be sector Data as all FF bytes
    for (lines = 0; lines < 4*1024; lines++) { // 4K lines
      // Compute Final Read Address
      RAddr = BAddr + Addr;
      // Read n bytes
      read_fast_nbytes(RAddr, RSize, id, &RData[0]); // Each line is 16 bytes
      for (k = 0; k < RSize; k++) {
          if (RData[k] != 0xFF) {
            printf("\n --->>> FATAL :: BLANK CHECK Failed : Actual Data = 0x%X", RData[k]);
            printf("\n --->>> FATAL :: BLANK CHECK Failed : Address= 0x%X ; index = %d\n", RAddr, k);
            exit(1);
          }
      } // end for 
    } // end for lines
    printf("Sector Verified\n");
  } // end for sector

} // end function

// ============================================================================
// Function to do PROGRAM Flash content with MCS file
// ============================================================================
program (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t BAddr;                   // Base Address or Sector Number (64KB Sectors)
  uint32_t Addr;                    // Address for Data line
  uint32_t WAddr;                   // Final Address for Program command
  int lines;                        // Each line is 16-bytes
  int sector;                       // 1Gb(128MB) memory has 2K, 64KB sectors
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char ELA[5];
  char ELAHdr[10];                           // Header Part
  const char ELAHdrConst[10] = ":02000004";  // Constant Header Part
  char WSize_s[3];
  uint32_t WSize;
  char Addr_s[5];
  char Cmd_s[3];
  uint32_t Cmd;
  uint32_t WData[16];
  int k;
  char WData_s[3];
  int EOF_MCS = 0;

// ============================================================================
  // Program Sector by Sector
// ============================================================================
  print_line();
  if (id == 0) {
    printf(" -->> PROGRAM Primary Flash !!! \n");
    fseek(fptr_mcs0, 0, SEEK_SET); // Reset to beginning
  } else {
    printf(" -->> PROGRAM Secondary Flash !!! \n");
    fseek(fptr_mcs1, 0, SEEK_SET); // Reset to beginning
  }  
  print_line();

  // ==========================================================================
  // Program sector-by-sector and line-by-line
  // ==========================================================================
  BAddr = 0x0; // Start Address init
  Addr  = 0x0; // Address init
  for (sector = 0; sector < MAX_SECTORS; sector++) { // Currently 256 Sectors
    if (id == 0) {
      if ((read = getline(&line, &len, fptr_mcs0)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
      } else {
        printf(" --->>> FATAL :: ERROR Reading Primary MCS file\n");
        exit(1);
      }
    } else {
      if ((read = getline(&line, &len, fptr_mcs1)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
      } else {
        printf(" --->>> FATAL :: ERROR Reading Secondary MCS file\n");
        exit(1);
      }
    }
    // ========================================================================
      // Get the ELAHdr
    // ========================================================================
    memcpy(ELAHdr, line, 9);
    ELAHdr[9] = '\0';
    // ========================================================================
      // Get Extended Linear Address (2 Bytes/4 Chars)
    // ========================================================================
    if (strcmp(ELAHdr, ELAHdrConst) == 0) {
      memcpy(ELA, line+9, 4);
      ELA[4] = '\0';
      BAddr = (strtoul(ELA, NULL, 16)) << 16;
    } else {
      printf(" --->>> FATAL :: ERROR in MCS File ELA line \n");
      exit(1);
    }
    // ========================================================================
      // Line by Line Program of 16-bytes
    // ========================================================================
    printf(" -> Programming SECTOR %04d; Start Address 0x%08X .. ", sector, BAddr);
    // These should be sector Data content with some EOF somewhere
    for (lines = 0; lines < 4*1024; lines++) { // 4K lines
      if (id == 0) {
        if ((read = getline(&line, &len, fptr_mcs0)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        } else {
          printf(" --->>> FATAL :: ERROR Reading Primary MCS file\n");
          exit(1);
        }
      } else {
        if ((read = getline(&line, &len, fptr_mcs1)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        } else {
          printf(" --->>> FATAL :: ERROR Reading Secondary MCS file\n");
          exit(1);
        }
      }
    // ========================================================================
      // Get the DataSize
    // ========================================================================
      memcpy(WSize_s, line+1, 2);
      WSize_s[2] = '\0';
      WSize = strtoul(WSize_s, NULL, 16);
    // ========================================================================
      // Get the Addr
    // ========================================================================
      memcpy(Addr_s, line+3, 4);
      Addr_s[4] = '\0';
      Addr = strtoul(Addr_s, NULL, 16);
    // ========================================================================
      // Get the Command
    // ========================================================================
      memcpy(Cmd_s, line+7, 2);
      Cmd_s[2] = '\0';
      Cmd = strtoul(Cmd_s, NULL, 16);
    // ========================================================================
      // Perform Tasks based on Command
    // ========================================================================
      if (Cmd == 0) { // Data Program
        // Compute Final Read Address
        WAddr = BAddr + Addr;
        // Get Program Data
        for (k = 0; k < WSize; k++) {
          memcpy(WData_s, line+9+2*k, 2);
          WData_s[2] = '\0';
          WData[k] = strtoul(WData_s, NULL, 16);
        } // end for 
        // Program n bytes
        program_nbytes(WAddr, WSize, id, &WData[0]); // Each line is 16 bytes
      } else if (Cmd == 1) { // EOF
        EOF_MCS = 1; // exit(1);
        break;
      } else {
        printf(" --->>> FATAL :: UNKNOWN COMMAND \n");
        exit(1);
      }
    } // end for lines
    printf("Sector Programmed\n");
    if (EOF_MCS == 1) {
      EOF_MCS = 0; // reset
      print_line();
      if (id == 0) {
        printf(" -->> PROGRAM Complete. EOF for Primary MCS File Reached \n");
      } else {
        printf(" -->> PROGRAM Complete. EOF for Secondary MCS File Reached \n");
      }
      print_line();
      break;
    }
  } // end for sector

} // end function

// ============================================================================
// Function to do READ and VERIFY Flash content with MCS file
// ============================================================================
verify (int id) {

// ============================================================================
  // Local Variables
// ============================================================================
  uint32_t BAddr;                   // Base Address or Sector Number (64KB Sectors)
  uint32_t Addr;                    // Address for Data line
  uint32_t RAddr;                   // Final Address for READ command
  int lines;                        // Each line is 16-bytes
  int sector;                       // 1Gb(128MB) memory has 2K, 64KB sectors
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  char ELA[5];
  char ELAHdr[10];                           // Header Part
  const char ELAHdrConst[10] = ":02000004";  // Constant Header Part
  char RSize_s[3];
  uint32_t RSize;
  char Addr_s[5];
  char Cmd_s[3];
  uint32_t Cmd;
  uint32_t RData[16];
  uint32_t EData;
  int k;
  char EData_s[3];
  int EOF_MCS = 0;

// ============================================================================
  // Read And Verify Sector by Sector
// ============================================================================
  print_line();
  if (id == 0) {
    printf(" -->> READ and VERIFY Primary Flash !!! \n");
    fseek(fptr_mcs0, 0, SEEK_SET); // Reset to beginning
  } else {
    printf(" -->> READ and VERIFY Secondary Flash !!! \n");
    fseek(fptr_mcs1, 0, SEEK_SET); // Reset to beginning
  }  
  print_line();

  // ==========================================================================
  // Read and Compare sector-by-sector and line-by-line
  // ==========================================================================
  BAddr = 0x0; // Start Address init
  Addr  = 0x0; // Address init
  for (sector = 0; sector < MAX_SECTORS; sector++) { // Currently 256 Sectors
    if (id == 0) {
      if ((read = getline(&line, &len, fptr_mcs0)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
      } else {
        printf(" --->>> FATAL :: ERROR Reading Primary MCS file\n");
        exit(1);
      }
    } else {
      if ((read = getline(&line, &len, fptr_mcs1)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
      } else {
        printf(" --->>> FATAL :: ERROR Reading Secondary MCS file\n");
        exit(1);
      }
    }
    // ========================================================================
      // Get the ELAHdr
    // ========================================================================
    memcpy(ELAHdr, line, 9);
    ELAHdr[9] = '\0';
    // ========================================================================
      // Get Extended Linear Address (2 Bytes/4 Chars)
    // ========================================================================
    if (strcmp(ELAHdr, ELAHdrConst) == 0) {
      memcpy(ELA, line+9, 4);
      ELA[4] = '\0';
      BAddr = (strtoul(ELA, NULL, 16)) << 16;
    } else {
      printf(" --->>> FATAL :: ERROR in MCS File ELA line \n");
      exit(1);
    }
    // ========================================================================
      // Line by Line check of 16-bytes
    // ========================================================================
    printf(" -> Verifying SECTOR %04d; Start Address 0x%08X .. ", sector,BAddr);
    // These should be sector Data content with some EOF somewhere
    for (lines = 0; lines < 4*1024; lines++) { // 4K lines
      if (id == 0) {
        if ((read = getline(&line, &len, fptr_mcs0)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        } else {
          printf(" --->>> FATAL :: ERROR Reading Primary MCS file\n");
          exit(1);
        }
      } else {
        if ((read = getline(&line, &len, fptr_mcs1)) != -1) {
        //printf("Retrieved line of length %zu :\n", read);
        //printf("%s", line);
        } else {
          printf(" --->>> FATAL :: ERROR Reading Secondary MCS file\n");
          exit(1);
        }
      }
    // ========================================================================
      // Get the DataSize
    // ========================================================================
      memcpy(RSize_s, line+1, 2);
      RSize_s[2] = '\0';
      RSize = strtoul(RSize_s, NULL, 16);
    // ========================================================================
      // Get the Addr
    // ========================================================================
      memcpy(Addr_s, line+3, 4);
      Addr_s[4] = '\0';
      Addr = strtoul(Addr_s, NULL, 16);
    // ========================================================================
      // Get the Command
    // ========================================================================
      memcpy(Cmd_s, line+7, 2);
      Cmd_s[2] = '\0';
      Cmd = strtoul(Cmd_s, NULL, 16);
    // ========================================================================
      // Perform Tasks based on Command
    // ========================================================================
      if (Cmd == 0) { // Data read and compare
        // Compute Final Read Address
        RAddr = BAddr + Addr;
        // Read n bytes
        read_fast_nbytes(RAddr, RSize, id, &RData[0]); // Each line is 16 bytes
        for (k = 0; k < RSize; k++) {
          // Get Expected Data
          memcpy(EData_s, line+9+2*k, 2);
          EData_s[2] = '\0';
          EData = strtoul(EData_s, NULL, 16);
          if (EData != RData[k]) {
            printf("\nREAD Verify Failed : Expected Data = 0x%X \n", EData);
            printf("READ Verify Failed : Actual Data   = 0x%X \n", RData[k]);
            printf("READ Verify Failed : Address       = 0x%X \n", RAddr);
            printf("READ Verify Failed : index         = %d \n", k);
            exit(1);
          }
        } // end for 
      } else if (Cmd == 1) { // EOF
        EOF_MCS = 1; // exit(1);
        break;
      } else {
        printf(" --->>> FATAL :: UNKNOWN COMMAND \n");
        exit(1);
      }
    } // end for lines
    printf("Sector Verified\n");
    if (EOF_MCS == 1) {
      EOF_MCS = 0; // reset
      print_line();
      if (id == 0) {
        printf(" -->> READ Verify Complete. EOF for Primary MCS File Reached \n");
      } else {
        printf(" -->> READ Verify Complete. EOF for Secondary MCS File Reached \n");
      }
      print_line();
      break;
    }
  } // end for sector

} // end function

// ============================================================================
// Main Function
// ============================================================================
int main(int argc, char **argv) {
  
  // ============================================================================
    // Local Variables
  // ============================================================================
  uint32_t RegVal; 
  char *PrimaryMCS;
  char *SecondaryMCS;

  // ============================================================================
    // Try opening Device
  // ============================================================================
  fd_dev = gdfglb_open(0); 
  if (fd_dev < 0) {
    print_line();
    printf(" --->>> FATAL :: Error opening device !!!\n");
    print_line();
    exit(1);
  } else {
    print_line();
    printf("*****   Device Opened Successfully!!!   ***** \n");
    print_line();
  }

  // ============================================================================
    // Try opening Primary and Secondary MCS Files
  // ============================================================================
  PrimaryMCS   = argv[1];
  SecondaryMCS = argv[2];
  printf("Number of input Arguments ARGC = %d\n", argc);
  printf("Executable Name ARGV[0] = %s\n", argv[0]);
  printf("Primary MCS File path ARGV[1] = %s\n", argv[1]);
  printf("Secondary MCS File path ARGV[2] = %s\n", argv[2]);
  if (argc != 3) {
    print_line();
    printf(" --->>> FATAL :: Wrong usage of command\n");
    printf("Ex : ./gdf_isp ../tool/isp/jcv0_33161109_primariy.mcs ../tool/isp/jcv0_33161109_secondary.mcs\n");
    print_line();
    exit(1);
  }
  //fptr_mcs0 = fopen("/home/atpl/hambi/acceletrade-fpga.gdf/sw/tool/isp/jcv0_32150709_primary.mcs", "r");
  fptr_mcs0 = fopen(PrimaryMCS, "r");
  if (fptr_mcs0 == NULL) {
    printf(" --->>> FATAL :: ERROR opening PRIMARY MCS File\n");
    exit(1);
  }
  //fptr_mcs1 = fopen("/home/atpl/hambi/acceletrade-fpga.gdf/sw/tool/isp/jcv0_32150709_secondary.mcs", "r");
  fptr_mcs1 = fopen(SecondaryMCS, "r");
  if (fptr_mcs1 == NULL) {
    printf(" --->>> FATAL :: ERROR opening SECONDARY MCS File\n");
    exit(1);
  }

  // ============================================================================
    // Read Device ID of Primary and Secondary Flash
  // ============================================================================
  read_device_id(0);
  read_device_id(1);
  // ============================================================================
    // Read Flash Registers state
  // ============================================================================
  print_line();
  printf(" -->> Reading REGISTER CONFIGURATION of Primary Flash \n");
  print_line();
  RegVal = read_register(0, RD_STATUS_REG , "STATUS REGISTER", 1);
  RegVal = read_register(0, RD_FLAG_STATUS_REG , "FLAG STATUS REGISTER", 1);
  RegVal = read_register(0, RD_EXT_ADDR_REG , "EXTENDED ADDRESS REGISTER", 1);
  print_line();
  printf(" -->> Reading REGISTER CONFIGURATION of Secondary Flash \n");
  print_line();
  RegVal = read_register(1, RD_STATUS_REG , "STATUS REGISTER", 1);
  RegVal = read_register(1, RD_FLAG_STATUS_REG , "FLAG STATUS REGISTER", 1);
  RegVal = read_register(1, RD_EXT_ADDR_REG , "EXTENDED ADDRESS REGISTER", 1);
  //===========================================================================
    // ERASE Flash Content
  // ============================================================================
  erase(0);
  erase(1);
  //===========================================================================
    // Verify ERASE : Blank check
  // ============================================================================
  blank_check(0);
  blank_check(1);
  // ============================================================================
    // Program
  // ============================================================================
  program(0);
  program(1);
  // ============================================================================
    // Read and Verify Flash Memory Content
  // ============================================================================
  verify(0);
  verify(1);

  // ============================================================================
    // Close MCS File Pointers
  // ============================================================================
  fclose(fptr_mcs0);
  fclose(fptr_mcs1);

  // ============================================================================
    //  Close Device  
  // ============================================================================
  gdfglb_close(fd_dev);
  print_line();
  printf("*****   Device Closed Successfully!!!   *****\n");
  print_line();
}

