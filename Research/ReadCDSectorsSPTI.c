/* https://support.microsoft.com/en-us/kb/138434
The Windows NT CD-ROM File System (CDFS) currently supports compact discs
formatted with either the ISO 9660 or High Sierra file systems. CDFS does not
support other file systems, so Win32-based applications cannot read sectors
from discs formatted with file systems other than ISO 9660 or High Sierra.

Steps to Read Sectors from Compact Discs

Win32-based applications can read sectors in cooked mode from compact disks
in the same way they read sectors from floppy and hard disks. Because the
reads are in cooked mode, no error correcting codes are included in the sector
data that the application reads. Follow these three basic steps: 

Use CreateFile to open a CD-ROM drive with the syntax \\.\X: where X is the
letter of the CD-ROM drive.

Use ReadFile or ReadFileEx to read sectors.

Use CloseHandle to close the CD-ROM drive.

As with floppy and hard disks, all sector reads must start on sector boundaries
on the compact disc and must be an integral number of sectors long. Furthermore,
the buffers used for the reads must be aligned on addresses that fall on sector
boundaries. For example, because a sector on a compact disc is normally 2048 bytes,
the buffer that receives the sector data must be a multiple of 2048 and must
start on an address that is a multiple of 2048. An easy way to guarantee the
that the buffer will start on a multiple of 2048 is to allocate it with
VirtualAlloc. Finally, although sectors on compact discs are normally 2048
bytes, you should use the DeviceIoControl IOCTL_CDROM_GET_DRIVE_GEOMETRY
command to return the sector size to avoid hard-coded limits.

Issuing DeviceIoControl IOCTL Commmands to CD-ROM Drives

Whenf reading sectors from compact discs, applications usually need to use a
few support functions that provide capabilities such as determining the
characteristics of the media and locking the media in the drive so that it
can't be removed accidentally. These functions are provided by DeviceIoControl
IOCTL commands. 

To issue IOCTL commands to CD-ROM drives, Win32-based applications must use the
IOCTL compact disc commands defined in Ntddcdrm.h instead of the IOCTL disk
commands defined in Winioctl.h. The IOCTL disk commands will fail if issued
for compact discs. Documentation for the IOCTL compact disc commands is located
in the Windows NT DDK. The Ntddcdrm.h header file is located in the Windows NT
DDK in the \Ddk\Src\Storage\Inc directory.

Example Code that Reads Sectors from a Compact Disc

The following code demonstrates how to read sectors from a compact disc from a Win32-based application running on Windows NT. 
*/

#include <windows.h>
#include <winioctl.h>  // From the Win32 SDK \Mstools\Include
#include "ntddcdrm.h"  // From the Windows NT DDK \Ddk\Src\Storage\Inc

/*
   This code reads sectors 16 and 17 from a compact disc and writes
   the contents to a disk file named Sector.dat
*/ 

{
   HANDLE  hCD, hFile;
   DWORD   dwNotUsed;

   //  Disk file that will hold the CD-ROM sector data.
   hFile = CreateFile ("sector.dat",
                       GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL, NULL);

   // For the purposes of this sample, drive F: is the CD-ROM
   // drive.
   hCD = CreateFile ("\\\\.\\F:", GENERIC_READ,
                     FILE_SHARE_READ|FILE_SHARE_WRITE,
                     NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                     NULL);

   // If the CD-ROM drive was successfully opened, read sectors 16
   // and 17 from it and write their contents out to a disk file.
   if (hCD != INVALID_HANDLE_VALUE)
   {
      DISK_GEOMETRY         dgCDROM;
      PREVENT_MEDIA_REMOVAL pmrLockCDROM;

      // Lock the compact disc in the CD-ROM drive to prevent accidental
      // removal while reading from it.
      pmrLockCDROM.PreventMediaRemoval = TRUE;
      DeviceIoControl (hCD, IOCTL_CDROM_MEDIA_REMOVAL,
                       &pmrLockCDROM, sizeof(pmrLockCDROM), NULL,
                       0, &dwNotUsed, NULL);

      // Get sector size of compact disc
      if (DeviceIoControl (hCD, IOCTL_CDROM_GET_DRIVE_GEOMETRY,
                           NULL, 0, &dgCDROM, sizeof(dgCDROM),
                           &dwNotUsed, NULL))
      {
         LPBYTE lpSector;
         DWORD  dwSize = 2 * dgCDROM.BytesPerSector;  // 2 sectors

         // Allocate buffer to hold sectors from compact disc. Note that
         // the buffer will be allocated on a sector boundary because the
         // allocation granularity is larger than the size of a sector on a
         // compact disk.
         lpSector = VirtualAlloc (NULL, dwSize,
                                  MEM_COMMIT|MEM_RESERVE,
                                  PAGE_READWRITE);

         // Move to 16th sector for something interesting to read.
         SetFilePointer (hCD, dgCDROM.BytesPerSector * 16,
                         NULL, FILE_BEGIN);

         // Read sectors from the compact disc and write them to a file.
         if (ReadFile (hCD, lpSector, dwSize, &dwNotUsed, NULL))
            WriteFile (hFile, lpSector, dwSize, &dwNotUsed, NULL);

         VirtualFree (lpSector, 0, MEM_RELEASE);
      }

      // Unlock the disc in the CD-ROM drive.
      pmrLockCDROM.PreventMediaRemoval = FALSE;
      DeviceIoControl (hCD, IOCTL_CDROM_MEDIA_REMOVAL,
                       &pmrLockCDROM, sizeof(pmrLockCDROM), NULL,
                       0, &dwNotUsed, NULL);

      CloseHandle (hCD);
      CloseHandle (hFile);
   }
}
