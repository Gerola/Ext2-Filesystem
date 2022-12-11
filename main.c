/****************************************************************************
*                   KCW: mount root file system                             *
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <ext2fs/ext2_fs.h>

#include <string.h>
#include <libgen.h>
#include <sys/stat.h>
#include <time.h>

#include "type.h"

extern MINODE *iget();

MINODE minode[NMINODE];
MINODE *root;
PROC   proc[NPROC], *running;

char gpath[128]; // global for tokenized components
char *name[64];  // assume at most 64 components in pathname
int   n;         // number of component strings

int  fd, dev;
int  nblocks, ninodes, bmap, imap, iblk;
char line[128], cmd[32], pathname[128], second[128];
OFT oft[64];

#include "cd_ls_pwd.c"
#include "alloc_dalloc.c"
#include "mkdir_creat.c"
#include "rmdir.c"
#include "link_unlink.c"
#include "symlink.c"
#include "open_close.c"
#include "write_cp.c"
#include "read_cat.c"

int init()
{
  int i, j;
  MINODE *mip;
  PROC   *p;

  printf("init()\n");

  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    mip->dev = mip->ino = 0;
    mip->refCount = 0;
    mip->mounted = 0;
    mip->mptr = 0;
  }
  for (i=0; i<NPROC; i++){
    p = &proc[i];
   	for(int index = 0; index < 10; index++)
   	{
   		p->fd[index] = 0;
   	}
    p->pid = i+1;           // pid = 1, 2
    p->uid = p->gid = 0;    // uid = 0: SUPER user
    p->cwd = 0;             // CWD of process
  }
	
for (i = 0;i < 64; i++)
{
	oft[i].refCount = 0;
}

}

// load root INODE and set root pointer to it
int mount_root()
{  
  printf("mount_root()\n");
  root = iget(dev, 2);
}

char *disk = "disk2";     // change this to YOUR virtual

int main(int argc, char *argv[ ])
{
	if(argc > 1 && strcmp(argv[0],"./a.out")== 0)
	{
		disk = argv[1];	
	}
  int ino;
  char buf[BLKSIZE];
  char bufferread[256];

  printf("checking EXT2 FS ....");
  if ((fd = open(disk, O_RDWR)) < 0){
    printf("open %s failed\n", disk);
    exit(1);
  }

  dev = fd;    // global dev same as this fd   

  /********** read super block  ****************/
  get_block(dev, 1, buf);
  sp = (SUPER *)buf;

  /* verify it's an ext2 file system ***********/
  if (sp->s_magic != 0xEF53){
      printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
      exit(1);
  }     
  printf("EXT2 FS OK\n");
  ninodes = sp->s_inodes_count;
  nblocks = sp->s_blocks_count;

  get_block(dev, 2, buf); 
  gp = (GD *)buf;

  bmap = gp->bg_block_bitmap;
  imap = gp->bg_inode_bitmap;
  iblk = gp->bg_inode_table;
  printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, iblk);

  init();  
  mount_root();
  printf("root refCount = %d\n", root->refCount);

  printf("creating P0 as running process\n");
  running = &proc[0];
  running->cwd = iget(dev, 2);
  printf("root refCount = %d\n", root->refCount);

  // WRTIE code here to create P1 as a USER process
  
  while(1){
    printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink\n open|close|pfd|read|write|cat|cp|lseek|quit] ");
    fgets(line, 128, stdin);
    line[strlen(line)-1] = 0;

    if (line[0]==0)
       continue;
    pathname[0] = 0;

    sscanf(line, "%s %s %s", cmd, pathname,second);
    printf("cmd=%s pathname=%s links=%s \n", cmd, pathname,second);
  
    if (strcmp(cmd, "ls")==0)
    	ls(pathname);
    else if (strcmp(cmd, "cd")==0)
       cd(pathname);
    else if (strcmp(cmd, "pwd")==0){
       pwd(running->cwd);
       printf("\n");}
    else if (strcmp(cmd, "quit")==0)
       quit();
     else if (strcmp(cmd,"mkdir")==0)
     {
        mkdirs(pathname);
     }
     else if (strcmp(cmd,"creat")==0)
     {
     	creats(pathname);
     }
     else if (strcmp(cmd,"rmdir")==0)
     {
     	rmdirs(pathname);
     }
     else if (strcmp(cmd,"link") == 0)
     {
     	links(pathname,second);
     	second[0] = "";//reset the value
     }
     else if(strcmp(cmd,"unlink")==0)
     {
     	unlinks(pathname);
     }
     else if (strcmp(cmd,"symlink")==0)
     {
     	symlinks(pathname,second);
     }
     else if (strcmp(cmd,"open") == 0)
     {
     	int mode = atoi(second);
      my_open(pathname,mode);
     }
     else if (strcmp(cmd,"close") == 0)
     {
     	int mode = atoi(pathname);
     	my_close(mode);
     }
     else if (strcmp(cmd,"pfd") == 0)
     {
     	pfd();
     }
     else if (strcmp(cmd,"read")==0)
     {
     
     	int mode = atoi(pathname);
     	int reading = atoi(second);
     	bzero(bufferread,BLKSIZE);
     	my_read(mode,bufferread,reading);
     	printf("%s \n",bufferread);
     }
     
     else if (strcmp(cmd,"write")==0)
     {
     	int mode = atoi(pathname);
     	int reading = strlen(second);
     	my_write(mode,&second,reading);
     }
     else if(strcmp(cmd,"cat")==0)
     {
      my_cat(pathname);
     }
     else if(strcmp(cmd,"cp")==0)
     {
      my_cp(pathname, second);
     }
     else if(strcmp(cmd,"lseek")==0)
     {
     	int file = atoi(pathname);
     	int size = atoi(second);
     	lseeks(file,size);
     
     }
  }
}

int quit()
{
  int i;
  MINODE *mip;
  for (i=0; i<NMINODE; i++){
    mip = &minode[i];
    if (mip->refCount > 0)
      iput(mip);
  }
  printf("see you later, alligator\n");
  exit(0);
}
