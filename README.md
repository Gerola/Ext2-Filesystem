## Ext2-Filesystem
  This is an Ext2 filesystem that is compatible with Linux. It is recommended to be run on a VM or through WSL. Some packages may need to be installed before the program will run properly. In the files there are two disks currently, disk2 and mydisk. With disk2 having files currently on it while mydisk is a disk with nothing. To run the program run `./mk` or  `sudo ./mk`  to create a new disk. From there `a.out` should be generated and can be called instead of `./mk` or `sudo ./mk` with the command being `./a.out`. In order to change the disk the program reads change `char *disk = "disk2";` on line 82 to the disk you would like to use for the program.

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
