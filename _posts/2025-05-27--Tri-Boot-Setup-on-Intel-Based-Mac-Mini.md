---
layout: post
title:  "Tri-Boot Setup on Intel-Based Mac Mini"
date:   2025-05-27 10:00:00 +0100
---

how to setup macOS(v15.5), Ubuntu(24.04) and Windows(10) on an intel-based Mac Mini on the same internal SSD.

in order to test *Umfeld* better on 3 different Operating Systems ( macOS, Ubuntu Linux, and Windows ) i prepared a cup of coffee and took on the challenge to install all 3 onto a single intel-based Mac Mini. it took me a few detours but it sort of works now. the Operating Systems selection process is still a bit quirky … but they all three run natively.

and yes i did consider using virtual environments ( e.g [UTM](https://getutm.app) which is pretty nice ) but it turns out that even there the ARM architecture of my MacBook Pro M3 ( i.e *Apple Silicon* ) prooved to be stubborn … again. e.g no FFmpeg version beyond 4 is available with `apt` on Ubuntu and on Windows `MSYS UCRT64` installer does not even start …

well here is what i did to get it working on an intel-based Mac Mini:

# Tri-Boot Setup on Intel-Based Mac Mini

how to setup macOS(v15.5), Ubuntu(24.04) and Windows(10) on a single intel-based Mac Mini on the same internal SSD:

- clean macOS install
- boot into *Recovery Mode* ( `CMD+R` )
- disable protection in *Startup Security Utility* 
    - go to `Utilities > Startup Security Utility`
    - set `Secure Boot` to `No Security`
    - set `Allowed Boot Media` to `Allow booting from external…`
- disable *System Integrity Protection* (SIP)
    - go to `Utilities > Terminal` (  `CMD+SHIFT+T` )
    - run command `csrutil disable`
    - consider re-enabling this after installtion is complete
- re-boot into macOS
- download [Windows 10 (64-bit)](https://www.microsoft.com/en-en/software-download/windows10ISO)
- download [Ubuntu 24.04 Desktop (64-bit)](https://ubuntu.com/download/desktop)
- create a *bootable USB stick* following these [instructions](https://ubuntu.com/tutorials/install-ubuntu-desktop)
- open *Disk Utility* application form `/Applications/Utilities` ( `CMD+SHIFT+U` in *Finder* )
- create Linux partition 
    - select from menu `View > Show All Devices` ( `CMD+2` )
    - select internal SSD e.g `Macintosh HD` @note(this is top level *volume* or device not a partition)
    - click `Partition` in tool bar ( `CMD+SHIFT+P` )
    - click `+`, choose `Add Partition` 
    - select *Size* @note(e.g `83GB` it is suggested to give it a minimum `64GB`)
    - select *Format* as `APFS` @note(`ExFAT` and `MS-DOS (FAT)` seem to get erased by *Boot Camp Assistent*)
    - click `Apply`
    - @optional(right-click on new partition and select `Delete APFS Volume…`) @note(<- not sure if this is necessary)
    - @optional(maybe take note of the volumes ID e.g `disk2`)
- start *Boot Camp Assistent* form `/Applications/Utilities` ( `CMD+SHIFT+U` in *Finder* )
- boot into *Windows* and run setup
    - @optional(select `I don’t have a Product Key`)
    - @optional(select `Windows 10 Pro`)
- once *Windows* is installed, re-boot and hold `ALT` to bring up *boot menu*
- connect *Ubuntu* bootable USB stick
- select *EFI Boot* 
- select `Try or Install Ubuntu`
- in Ubuntu desktop double-click and run `Install Ubunutu`
    - choose country, keyboard and connection
    - choose "Interactive Installation"
    - after choosing installation flavour
    - choose *Manual installation* ( ⚠️ Important ⚠️ )
    - select the correct drive … this is a bit tricky to identify … look e.g for size e.g `83GB` … or device ID e.g `nvme0n1p5` or type e.g `APFS`
    - example layout:
    ```
    | Partition | Size      | Format | Likely Use                                       |
    | --------- | --------- | ------ | ------------------------------------------------ |
    | nvme0n1p1 | 314.57 MB | VFAT   | **EFI partition** (used by macOS/Windows/Ubuntu) |
    | nvme0n1p2 | 74.00 GB  | APFS   | **macOS**                                        |
    | nvme0n1p3 | 10.00 GB  | exFAT  | maybe Boot Camp helper or recovery               |
    | nvme0n1p4 | 83.68 GB  | NTFS   | **Windows**                                      |
    | nvme0n1p5 | 83.00 GB  | APFS   | likely **Ubuntu** partition**                    |
    ```
    - select *Ubuntu* partition e.g `nvme0n1p5`
    - click `Change`
    - for `Used as` choose `Ext4`
    - for `Mount point` choose `/`
    - press `OK`
    - check `Format` check box
    - select *EFI system partition*
    - for `Mount point:` choose `/boot/efi`
    - uncheck `Format` check box
    - for `Device for boot loader installation` choose the internal SSD e.g `nvme0n1`
    - click `Next`
    - follow the instruction to finish the installation
- reboot and disconnect bootable USB stick right away
- now all 3 systems are installed 
    - to select macOS press `ALT` at start up to bring up *boot menu*
    - to select Ubuntu just re-boot without pressing any key ( *GRUB menu* will appear )
    - to select Windows either way works

NOTE: this process might work on other Macintosh machines as long as they are intel-based Macintosch.