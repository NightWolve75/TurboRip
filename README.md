# TurboRip
The PC Engine/TurboGrafx-16 CD-ROM ISO/WAV/CUE Ripper - A Win32 command line tool to rip PC Engine/TurboGrafx-16/PC-FX game discs to ISO/WAV/CUE image filesets on any Microsoft® Windows® OS.
<div align="center"><h1>- The Demo Shots -</h1></div>

<img width="100%" src="https://github.com/user-attachments/assets/ac6030fd-58cf-41c6-82de-3b5221c5acd3" /><br/>
<img width="100%" src="https://github.com/user-attachments/assets/3db27727-91a9-431e-ac03-05f5d1bcd620" /><br/>
<img width="100%" src="https://github.com/user-attachments/assets/fb9c57a3-f3fd-473c-a8ef-256ebef796df" /><br/>
<img width="100%" src="https://github.com/user-attachments/assets/d85ed92c-a744-4d40-9180-271cc1eb7d00" /><br/>
<img width="100%" src="https://github.com/user-attachments/assets/b9edab3d-eedf-4e8a-9e8b-c92ed2d42e3c" /><br/>
<img width="100%" src="https://github.com/user-attachments/assets/abdcfb48-e9fb-4a73-a7d1-04c2c39cab71" /><br/>

<div align="center"><h1>- The Introduction -</h1></div>

   I got the idea for this program some time after working on my translation
   project for "Ys IV: The Dawn of Ys" and releasing the first patch. It was
   based on many factors, one of them being that a project member was planning
   on creating English dubs for the Japanese audio tracks used in the game. Now
   to replace those audio tracks with English replacements, one would need to
   rip their Ys IV disc in a special way. The required image format is what's
   referred to as ISO/WAV/CUE. In this format, unlike formats such as CDRWIN's
   BIN/CUE, Nero's NRG, or Alcohol 120%'s MDS, etc., every track file is ripped
   to a separate file when dealing with a mixed mode CD (which is what all
   PC Engine/TG-16 CDs are). This has many advantages, since audio tracks are
   ripped to wave files, they can individually be compressed by special audio
   compression programs to greatly reduce the storage space required.

   So anyway, the purpose of this program is to rip/extract any PC Engine/TG-16
   CD-ROM that you might have directly into an ISO/WAV/CUE image format. It
   will guarantee the same size of every track file across all CD-ROM devices
   because it will enforce the PRE/POST-GAP rules for transitions between track
   types, something even GoldenHawk's CDRWIN program will not do, nor any other
   professional software of that nature. I wanted a perfect extraction of every
   track so every ISO and WAV file will come out to be the same size on every
   machine out there. So eliminating size inconsistencies in ripping results is
   another reason why I felt this program needed to exist.

   While this program is intended for use with PC Engine/TG-16 discs, it can
   also detect and rip MODE2/2352 discs such as CD-I/PlayStation ones. So it is
   a general purpose command-line ripping program for various discs. But here's
   the neat thing that makes it unique to PC Engine/TG-16 discs: This program
   is compiled with a TOC Database of all PC Engine/TG-16 discs, so when you
   insert an original PC Engine/TG-16 disc, it'll detect and verify its
   authenticity and use the default title stored within to name the track
   files. You'll see what I mean after you try the program and it correctly
   names the image based on the disc you inserted.

   So, what else can TurboRip do for you? It can produce an ISO/MP3 image
   archive that is ready for use on Sony's PSP portable system with the PCEP
   emulator or even X-BOX's HUGO-X emulator. If you have an audio CD, you can
   just use the /mp3 parameter, set the bitrate and a few other controls, and
   it'll rip just audio tracks. It should be faster than anything commercial
   software has to offer for audio extraction to boot! I also added support for
   the Monkey's Audio codec for lossless image backups too. If the initial
   release of this application is well received by the community, I will add
   support for more audio codecs such as OGG, and possibly FLAC.

   Well, good luck, and do lemme know how this program works out for you!

   - NightWolve

<div align="center"><h1>- The Changelog/Version History -</h1></div>

 Version 1.45 (10/18/2025):

 + Update: New parameter option: /redump to dump in the redump.org bin image
   format (BIN/BIN/CUE format).

 + Update: No more spam, version-check browser popup removed.

 + Update: The basic /normalize feature was revisited and improved. The main
   loop for processing a WAVE file was rewritten in pure x86 Assembly, the
   volume multiplier precision was increased from a 4-byte float variable to
   an 8-byte double, switched to faster float-to-int rounding, and even more
   speed was achieved by skipping zero/null/silent 16-bit samples (an obvious
   no-brainer, previously every 16-bits of a WAVE were read/written blindly).

 + Bug fix: The /xbox option produced a CUE file with wrong MP3 filenames...
   Thanks to pceslayer @ PCEngine-FX.com forums for the bug report!
   (The CUE format is matched to the old cddissect app by Xport which most
   XBOX PCE emulators expect I figure, so that's what I used for reference.)

 + Bug fix: Compatibility improvements. More CD/DVD devices under Win7++ work!
   Some Windows® 7 versions can't read CD data sectors using SPTI that lack an
   an ISO9660 file system like all previous NT versions and the security level
   varies... It's confusing, but I had to rewrite the function that reads data
   sectors to try using the normal method of building a SCSI command packet
   with the SCSI_READ10 (0x28) opcode first, and if that fails, try the
   Microsoft SPTI method to let the OS mostly handle it. The prescribed SPTI
   ReadFile() method is no longer enforced as other Windows® 7 builds required
   AND it stopped working on PC Engine game CDs as they have no file system...

   The other issue is the CD device rejecting opcode 0x28 to read data sectors.
   As a result, I increased compatibility by adding the MMC1 0xBE opcode as a
   3rd option when the other 2 fail! That newer opcode is used for reading
   sectors raw, but by setting the appropriate flag in the command packet, it
   will return only the "cooked" user data just as the READ10 opcode does.
   Anyway, thanks to Chris Covell again for another bug report!

 + Bug fix: A 64-bit version of Windows® creates a separate registry view for a
   32-bit app under the "Wow6432Node" key that prevented the "AllocateCDRoms=1"
   string from being properly set. Thanks to ImgBurn for this tip/info! If that
   registry value can be set, it may allow CD/DVD reading to occur under User
   Rights, but you need to merge a .reg file with the contents below:

   e.g. "CDReadRightsNT.reg"

     [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon]
     "AllocateCDRoms"="1"

   Note: Following investigation of virustotal.com and false virus-flagging, I
   no longer compile TurboRip to set this HKLM registry value to reduce 1 false
   virus report, so I include 2 .reg files to do it yourself...

 + Bug: CTRL+BREAK/C not working 100% correctly (still fixing this...).


 Version 1.41 (10/15/2016):

 + Update: New parameter option: /auto which simply scans all drives for the
   first game CD detected in the TOC database and rips it without prompt! ^_^
   Try: "/auto /turbo" for max, automatic ripping of the first Turbo CD found.

 + Update: New parameter option: /normalize which will auto-normalize/amplify
   wave files afer ripping them. Given low preamp levels in some CDs making 'em
   very hard to play loudly on your sound system, this will be useful to some!

 + Update: I switched to the TOC CDDB checksum standard for identifying game
   discs. This reduced TurboRip's size by 200KB, and allowed me to use the much
   quicker binary search algorithm to find a CD title whereas before I used a
   basic linear search. This also prepares TurboRip for a future upgrade, to
   look up the CDDB ID at www.freedb.org to conveniently title music discs.

 + Update: Alignment of 64KB SCSI buffer. Alignmask of CD/DVD drive processed.
   Switched from dynamic memory allocation to static - this more guarantees no
   memory fragmentation or trouble. I noticed more general stability this way!

 + Update: Error messages are now printed in red for fun, also some yellow is
   used for status/info messages. :)

 + Update: MNKyDeth has an original copy of "Dungeon Master - Theron's Quest"
   that wasn't in the PCE/TG-16 TOC database, so it appears another factory
   pressing was discovered. I added it under its catalog/reference #TGXCD1041.

 + Update: Full [re]testing on Windows® 95/98/ME/NT4/2000/XP/Vista/7/8/10 was
   performed to 100% verify TurboRip's compatibility, no assumptions this time!

 + Update: TurboRip now automatically centers its working CMD window.

 + Bug fix: Fixed total epic failure for Windows® 7 that required major rewrite!
   TurboRip is now officially certified for Windows® 7 support even though I
   said it was prior to this release without actually testing it on!!! :)
 
   *Better trapping of SPTI errors after SCSI command packet sent.
   *Using prescribed SPTI method for raw read of audio sectors - Win7 required!
   *Using prescribed SPTI ReadFile method for data sector reads, the ONLY Win7
    permissible way for MODE1 sectors due to security/policing of SCSI reads!
   *Win7 security changes led to this!
   Audio sectors - check!!!
   Data sectors - check!!!
 
   Thanks to johnnykonami for the bug report which also led me to do complete
   genuine Windows® platform tests and find/fix the bug for NT 4.0 as well.

 + Bug fix: Setting thread priority to 'highest' on Windows® 95/98/ME caused
   buggy input handling at the Command Prompt, sometimes even crashes! It took
   hours of testing on Windows® 95 to figure this out, removed one unneeded
   line of code to fix... Quite pesky...

 + Bug fix: Getting a SPTI CD/DVD drive handle on Windows® NT 4.0 (SP6) failed!
   Now works both with User or Administrator Rights! Why bother making sure it
   can work all the way back to such old Windows versions ?? Just because! :)
   I started my software developer career on Windows® NT4 Workstations and it's
   just nice to know that TurboRip can work on them. It lets me know the code
   is simple enough and most appropriately makes use of the Windows API as well
   as handle some of its quirks where one or two lines of code or flags can
   make or break software between different OS versions.

 + Bug fix: Wrongly displayed US TurboGrafx CD type for Japanese PC Engine CDs.
   This may have actually caused crashes for some people. Bug Report: It did!
   tbone3969 @ PCEngine-FX.com reported crashing on his Windows 10 machine which
   was subsequently solved by this fix! :)


 Version 1.40 (8/7/2015 - 8/19/2015):

 + Update: Now using native NT SCSI library (SPTI) for NT/2K/XP/Vista/7/8/10+
   Windows® platforms!!! No more dependency on an ASPI DLL and less of the
   problems associated with it!!! You will at times need Administrator Rights
   for sure with Operating Systems like Windows® 7.

 + Update: Certified, tested, approved for use all the way back to Windows®
   95/98/ME for the "retro" in you AND all the way to the present with the new
   arrival of Windows® 10!!!! Even I find it hard to believe it works on all
   of them!!!

 + Update: Improved skipping of unreadable data sectors near postgap area
   transition. If you had trouble reading some PC Engine/TG-16 discs, this
   should help!

 + Update: Major Command Console interface enhancement: TurboRip can now be
   fully run by double-clicking it anywhere in Windows Explorer! The user no
   longer has to first open a Command Console window and then CD to the folder
   where it's located in order to enter parameters! User will be prompted for
   parameters if none were detected when TurboRip is executed/double-clicked!!
   A troll on the Ancient Land of Ys forums once barked, "How do you use this
   F#!K1NG! PROGRAM?!?!?!?!" having never encountered command-line apps before
   and not knowing how to first open a Command Console before using it...
   Short of a future Windows GUI version, this enhancement solves that issue!
   Of course, you can't fix stupid nor those who can't be bothered to read
   instructions...

 + Update: By request, the TOC CD data of NEC PC-FX videogames (50 out of 80)
   were added to coincide with the release of the "Tyoushin Heiki Zeroigar"
   English fan translation patch on 8/8/2015!! Only games such as Zeroigar
   (a shooter) were added that "OBEY" the standard mixed-mode CD-ROM rules
   which can accurately be ripped by TurboRip (future version will have all!).
   Teaser trailer of "God-Fighter" Zeroigar link below:
   https://www.youtube.com/watch?v=GnUtZkT7pVM

 + Update: Added CD-TEXT support! Some music audio discs can have track file
   names and other info such as artist, composer, genre, etc. burned into the
   "Lead-in" area of the disc. Unfortunately, it's rare to find discs using
   this feature because it arrived towards the end of the CD format's life, but
   I thought I'd add support for it nonetheless! You still have CDDB of course.
   (You can think of this feature as CDDB BTW, but that the album data/info is
   actually burned into the disc so you don't need a program to do a CDDB
   look-up over the Internet to obtain it.)

 + Update: Added CD-EXTRA support! If your music audio disc carries the label
   of "Enhanced CD," then it's really a multi-session disc that comes in 3
   possible forms, both with audio tracks and a MODE2 data track that can
   include music videos, interviews with the band, whatever, etc. So, I've
   added support for ripping the one format that I know of for now. I happen
   to have some of these discs, so I wanted to make TurboRip work with them.
   Unlike with CD-TEXT, music audio discs such as these are more common.

 + Update: Various performance enhancements in interface, behavior, coding were
   made to really make this app as great as it should be!! However, index/gap
   detection by reading the "Q" subchannel failed to make it in this version.
   Stay tuned!

 + Update: To reduce the size of TurboRip, all third-party components for MP3
   and APE are now zipped within TurboRip and extracted/unzipped on demand!
   TurboRip is now 4096 aligned to the preferences of Win98® as is the APE DLL.
   4K alignment normally makes an EXE/DLL bigger, but with some reduction in
   the way CD TOC naming data was stored, the EXE wound up smaller than ever!

 + Update: When using the /APE parameter for perfect CD backups, support files
   are now extracted to easily decompress APE files back to WAVE when needed!
   Simply double-clicking the "APE_TO_WAVE.bat" batch file will do the trick!

 + Update: The default 3.9.9.0 (2004) Monkey’s Audio (APE) codec no longer
   requires the Unicode layer for Win95/98/ME®, so TurboRip fully works with
   both its APE and MP3 codecs on a clean install of all Windows® flavors!!! :)
   Beyond minimizing code in the DLL needed just for compression, I also fixed
   a slight bug where the file handles to newly written APE files weren't being
   closed. Only after TurboRip exits was it left to the OS to close them... As
   such, I don't recommend ever upgrading to the 2015 DLL from the MACSDK... So
   if using the /APE option, just leave it all to TurboRip! You can install the
   latest Monkey's Audio GUI app to deal with the APE files produced however,
   but don't bother with upgrading TurboRip's internal DLL.

 + Bug fix: I noticed on Windows® XP that as TurboRip read sectors from CD/DVD
   drives connected via USB or FireWire, it could actually lose the connection!
   This could happen consistently when you ALT+TAB'ed to switch to another app!
   I found the solution was to raise the Process Priority of TurboRip to HIGH!
   Normally, disc burning software will raise the Process Priority to HIGH as
   well when *burning* a disc which makes sense, and so with all that, I now
   ALWAYS set TurboRip's priority to HIGH to avoid the bug with external drives
   AND speed up the ripping time - it might as well live up to its name!! :)

 + Bug fix: Fixed a drive selection bug in Windows® 98SE when selecting a drive
   number from the prompt menu.

 + Fixed issue that caused "Linda ³ (J)" to be excluded from the TOC database.
   Squaresoft74's use of the fancy superscript "3" was the culprit...

 + Change: If all CD tracks are audio, it's a music CD (not videogame!) so
   messages like "Note: No videogame CD-ROM was detected." were eliminated.

 + Change: The track file-naming style was changed to put the track # (01-99)
   first. This is better for sorting and viewing the CD image file set I think.

 + Change: TurboRip no longer installs its own ASPI layer for Win9X/ME, it just
   relies on the default Adaptec ASPI layer. Compatibility is still maintained!
   It appears TurboRip works best with the 1995 Adaptec layers found on Win98
   and WinME by default, and I do not recommend upgrading to Adaptec 4.71 or
   anything else... Upgrading would just break support in my tests and TurboRip
   worked fine with new installations of Win98/ME, so I think your best bet is
   to take the "If it ain't broke, don't try to fix it" route.

 + Update: TurboRip can rip a CD-ROM with only Guest Rights under Windows®
   Vista and User Rights under some Windows® NT/2K/XP versions. However, if you
   do have trouble in newer Operating Systems, try right-clicking TurboRip, and
   selecting "Run as administrator." But yeah, you might be able to access/read
   a CD/DVD drive *without* Administrator Rights... On Windows® 7 you'll likely
   need Administrator Rights given the security changes there which were then
   reduced for Windows® 10 for whatever reason on Microsoft's part.

 + Update: TurboRip sets the TOP_MOST flag 'on' of the Command Prompt window so
   it can never be hidden behind other windows until it's closed or minimized.
   If it's minimized while ripping, it'll restore and flash itself when done.

 + Bug fix: Fixed a minor bug that caused the /name parameter to be overwritten
   when the last parameter specified was /useaspi or if any other text was
   mistakenly typed at the Command Prompt.

 + Bug fix: Fixed another minor bug with the /name parameter where all letters
   were forcibly lowercased. Casing is now properly preserved.

 + Change: Options /pcep and /hugox are now /psp and /xbox respectively.
   Shorter and easier to go by the gaming platform of those emulators I figure.

 + Change: Other options that changed: /rs to /mrs, /br to /mbr, /mbr to /mmbr,
   /vbr to /mvbr. For help, it now can either be /?, /h, or /help.

 + Update: Added a shortcut /turbo option for /speed=max - Adding /turbo at the
   prompt for parameters is a quicker way to set the drive's reading speed to
   maximum.
 
 + Update: When using /?, /h, or /help for a parameter list, TurboRip no longer
   exits and forces you to restart it - it will list the parameters, then let
   you enter what you want to use and resume! The nice thing is you'll still be
   able to see many parameters as you decide what to use. You shouldn't avoid
   reading the ReadMe to understand everything, but it's a nice shortcut!


 Version 1.00 (4/19/2006):

 + Integration with the PC Engine/TG-16 TOC database so as to detect a valid,
   original PC Engine/TG-16 disc if inserted, and warn you if not.

 + Can detect CD/DVD devices connected to your system via Firewire/USB!

 + Independent executable that provides its own copy of components if missing.
   You'll only ever need the executable and not have to worry about other
   missing dependent files as they'll be extracted on demand.

 + As of this release, LAME MP3 Library version 1.32 [Engine 3.98] (2/19/2006)
   was compiled with the executable for all your MP3 encoding needs. If you have
   a better/newer DLL, you can always replace LAME_ENC.DLL in the same folder
   as the executable and it'll use that instead.

 + As of this release, MAC APE Library version 3.9.9.0 (2004) was compiled
   with the executable for all your APE encoding needs. If you have a
   better/newer DLL, you can always replace MACDll.dll in the same folder as
   the executable and it'll use that instead. MAC=Monkey's Audio Codec is
   lossless audio encoder. Use this if you want a perfect backup of your CD!

 + Can rip your disc directly into an ISO/MP3 format that is usable by PCEP
   (a PC Engine/TG-16 emulator for Sony's portable PSP system) or HUGO-X (a
   PC Engine/TG-16 emulator for Microsoft's X-BOX console system).

 + Generates a default reliable CUE file for use with Daemon Tools or any CDRW
   burning software that supports CDRWIN's CUE format.

 + Generates a TOC dump in PCEP usable format.

 + Support for three data track modes (MODE1/2048, MODE1/2352, & MODE2/2352).
   That means you could rip a game disc from other systems such as Sega CD,
   NeoGeo CD, PlayStation, etc. PlayStation games can only be ripped in RAW
   mode so your drive must support reading RAW sectors.

 + Code is highly optimized - The default VC++ runtime engine is not used...
   Instead, I use a small custom runtime engine I prefer to compile with that
   results in a much smaller and quicker executable, along with the fact that I
   sacrifice ANSI portability by calling Windows® APIs directly for more speed.

 + I would argue TurboRip is probably faster than any audio ripper out there
   when used to rip a regular audio disc. Many of the libraries commercial
   software use are quite bloated while TurboRip was written with the bare
   minimum of what's needed to read from a MMC-Capable device. Their advantage
   of course is more compatibility with specific drives, etc. and support for
   Jitter Correction if your drive isn't "CD-DA Stream Accurate." TurboRip
   being command-line and not having to deal with controlling a Windows® GUI
   also provides a speed advantage.

 + Accurate track file sizes with ALL CD/DVD-ROM devices, ensuring consistency
   in ripping results. When dealing with mixed-mode discs, TurboRip follows the
   standard set forth by the industry as follows (verbatim from a MMC document):


   ****  6.2.11.6. Pre-gap ****
   If a Data track is preceded by a different mode of track (such as an audio
   track) or if the mode number of CD-ROM changes, this Data track starts with
   an extended pre-gap. A pre-gap is placed at the head of a Data track, also
   is belonging to the Data track. A pre-gap does not contain actual user data.
   The pre-gap is encoded as "pause."

   An extended pre-gap is divided into 2 parts. The first part of the
   extended pre-gap has a minimum 1 second of data, and it is encoded according
   to the data structure of previous track. The second part has a minimum 2
   seconds data, and this data track is encoded according to the same data
   structure as the other parts.

   ****  6.2.11.7. Post-gap ****
   If a Data track is followed by another kind of track (such as an audio
   track), this Data track ends with a post-gap. A post-gap is placed at the
   end of a Data track, and is part of the Data Track. A post-gap does not
   contain actual user data. The minimum length of post-gap is 2 seconds. The
   drive does not perform any action for a Post-gap.


   What this means is that if a Data track is followed by an Audio track, the
   CUE file will have a "PREGAP 00:02:00" line to in effect cause a post-gap of
   2 seconds for the prior Data track. If an Audio track is followed by a Data
   track, you will see a "PREGAP 00:03:00" line. Now because I follow these
   rules and assume the factory where the disc was burned/made follows them, I
   will subtract off sectors accordingly to avoid these gap/pause sectors.
   This is why TurboRip will work with a PC Engine/TG-16 disc without errors.
   This general rule should apply to many mixed-mode discs as well, but there's
   clearly no universal rule that's followed, so no guarantees...

<div align="center"><h1>- Project History -</h1></div>

   In retracing my steps I found that I queried for information on how to
   create such a program as far back as January 29, 2005 when I asked David
   Michel - the MagicEngine emulator author - how to read a CD-ROM sector. I
   decided I wanted a program like this soon after I had released my Ys IV
   patch as I mentioned in the Introduction. That was Saturday, December 25,
   2004. For the reasons already stated there was a real need for such a
   program given no commercial software could properly produce an ISO/WAV/CUE
   image file set when it comes to mixed-mode discs such as PC Engine/TG-16s!
   I basically wanted an easy/free way for someone to take their original
   Ys IV CD, pop it in their CD/DVD drive, run a program, and have it rip an
   ISO/WAV/CUE image file set for them with the greatest of ease. This wasn't
   possible yet, so I up and decided I was gonna change that......

   I worked on the idea for a bit in 2005, but I gave up... ASPI documentation
   was rather lacking, and dealing with the process of reading sectors directly
   from CDROM devices was a daunting task. It seemed too hard for little ole me
   to handle, so I gave up... I was intimidated, scared, discouraged... I gave
   up... They have a word for that: it's called quitter! That's right! Quitter!
   Proud to be one when it suits me! No, actually, I'm not... As such, I hadn't
   forgotten about my dream to make this program a year later.

   It was February 28, 2006, around the time I suddenly became possessed to
   finish the job I had once started. By this time, I had released "TocFixer"
   which was based on Squaresoft74's TOC database of PC Engine/TG-16 CDROMs. I
   had the experience in my belt of having created that application and that's
   partly what motivated me to try once again. I had thought of another cool
   use for the TOC data!! How about a ripping program that detects an original
   disc using that TOC data and names the track files accordingly, I thought?
   How awesome would that be?! Thus with my newly found fervor, I decided to
   try my luck once again!

   The search began anew! I scoured the Internet for as much sample code of
   ASPI commands that I could find. I looked at pre-existing code from bloated
   audio ripper components. I looked at comments by others in forums. Then, one
   comment eventually led me to the t10 group. "MMC" was thrown around. What
   was this "MMC" and what did it have to do with CD/DVD drives? I had to dig
   some more. Eventually, I did learn what it was after finding the relevant
   documentation. It's a specification of a universal means of communication
   with SCSI/ATAPI I/O drives/devices. It specifically deals with optical
   devices such as CD/DVD readers/writers, etc. Anyhow, I was faced with having
   to read very high level technical documentation, and I was slowly but surely
   learning what the hell was going on... Thus, I produced the first working
   version of TurboRip 3 weeks later!

   After the first 3 weeks I got in touch with Squaresoft74 to give him a copy
   to test with. Unfortunately, TurboRip didn't behave the same on his drives
   as it did on all of mine in many tests! So we worked a good deal on figuring
   out the problems... It was a back'n'forth approach and sometimes I couldn't
   get the feedback as fast as I wanted, but we did it in the end. I took about
   a week off anything computer-related, but after I resumed working on it I
   fixed all the problems he was having. TurboRip, some 7 weeks later, April 19,
   was finally something I was comfortable releasing for mass distribution. It
   only got better with each day. Hell, it was only hours ago before this first
   release that I added support for the Monkey's Audio codec (APE)!

   I should note that 2 similar programs, pcerip and cddissect, also offered
   some inspiration to me in creating TurboRip. However, pcerip only rips data
   tracks and while cddissect was very close to what I wanted, it would only
   output an audio track directly to MP3 with no option to allow for WAVE
   output... I was thinking of asking the creator for the source code and had
   that option been available I probably wouldn't have pursued this project
   interestingly enough...

   Alas, here we are. A very exquisite command-line program that should be very
   useful to the PC Engine/TG-16 community as well as to others is before you.
   Go forth, my children, and use the gift I leave on to you wisely. Courage...

   -NightWolve
