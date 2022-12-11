## Ext2-Filesystem
  This is an Ext2 filesystem that is compatible with Linux. It is recommended to be run on a VM or through WSL. Some packages may need to be installed before the program will run properly. In addition to the packages the commands ` chmod +x mk ` and ` chmod +x mkdisk ` will need to be run with mk being used to create the a.out and mkdisk for creating new disk images. When first running the program run `./mk` to have a.out generated or  `sudo ./mk` to have a.out and a disk image generated. From there `a.out` can be called instead of `./mk` or `sudo ./mk` with the command being `./a.out`. There are two disks in the files and by default the program will use disk2. There are multiple ways to change the disk the program uses. One of which is to change `char *disk = "disk2";` on line 82 in main.c to the disk you would like to use for the program. The other is to run ` ./a.out diskname ` with the diskname being the disk you want to use for the session.

## The Following Commands are Supported
+ mkdir
+ rmdir
+ cd
+ ls
+ pwd
+ link
+ unlink
+ creat
+ open/close
+ read/write
+ cat
+ symlink
+ cp
+ lseek
+ pfd
+ quit
