//**********Work on**********
//cd->DONE
//ls->Done
//pwd->Done


/************* cd_ls_pwd.c file **************/
int cd(char* pathname)
{
	int ino = getino(pathname);
	if(ino == 0)
	{
		printf("NOT FOUND\n");
		return -1;
	}
	MINODE *mip = iget(dev,ino);
	if(!S_ISDIR(mip->INODE.i_mode))
	{
		printf("NOT A DIR\n");
		iput(mip);
		return -1;
	}
	iput(running->cwd);
	running->cwd = mip;
  // READ Chapter 11.7.3 HOW TO chdir
}

int ls_file(MINODE *mip, char *name)
{
  //printf("ls_file: to be done: READ textbook!!!!\n");
  // READ Chapter 11.7.3 HOW TO ls
  char *t1 = "xwrxwrxwr-------";
  char *t2 = "----------------";
  if((mip->INODE.i_mode & 0xF000) == 0x8000)
  {
  	printf("%c",'-');
  }
  else if ((mip->INODE.i_mode & 0xF000) == 0x4000)
  {
  	printf("%c",'d');
  }
  else if (((mip->INODE.i_mode & 0xF000) == 0xA000))	
  {
  	printf("%c",'l');
  }
  for(int p = 8; p >=0;p --)
  {
  	if(mip->INODE.i_mode & (1 << p))
  	{
  		printf("%c", t1[p]);
  	}
  	else
  	{
  		printf("%c",t2[p]);
  	}
  
  }
  char ftime[64];
  time_t seconds = mip->INODE.i_mtime;
  strcpy(ftime,ctime(&seconds));
  ftime[strlen(ftime)-1] = 0;
  
  printf(" %4d ",mip->INODE.i_links_count);
  printf(" %4d ",mip->INODE.i_uid);
  printf(" %4d ",mip->INODE.i_gid);
  printf(" %8d ",mip->INODE.i_size);

  printf(" %s ",ftime);
  printf(" %s ",name);
  
  char linking[BLKSIZE];
  if ((mip->INODE.i_mode & 0xF000) == 0xA000)	
  {
  	int length = readlinks(mip,linking);
  	linking[length] = 0;
  	printf("--> %s",linking);
  }
  
  printf("\t[%d, %d]\n",dev,mip->ino);
  

}

int ls_dir(MINODE *mip)
{
  ///printf("ls_dir: list CWD's file names; YOU FINISH IT as ls -l\n");

  char buf[BLKSIZE], temp[256];
  DIR *dp;
  char *cp;
  MINODE* m = mip;
  get_block(dev, mip->INODE.i_block[0], buf);
  dp = (DIR *)buf;
  cp = buf;
  
  while (cp < buf + BLKSIZE){
     strncpy(temp, dp->name, dp->name_len);
     temp[dp->name_len] = 0;
     m = iget(dev,dp->inode);
     //printf("%s  ", temp);
     ls_file(m,temp);
     iput(m);
     cp += dp->rec_len;
     dp = (DIR *)cp;
     
  }
  printf("\n");
}

int ls(char* pathname)
{
  //printf("ls: list CWD only! YOU FINISH IT for ls pathname\n");
  if(strcmp(pathname,"") == 0)
  {
  	ls_dir(running->cwd);
  }
  else
  {
  int i = getino(pathname);
  if(i == 0)
  {	
  	printf("Can't find the file\n");
  	return -1;
  }
   MINODE *mip = iget(dev,i);
  if(!S_ISDIR(mip->INODE.i_mode))
  {
		printf("NOT A DIR\n");
	  	iput(mip);
		return -1;
  }	
  ls_dir(mip);
  iput(mip);
  }
  
}

char *pwd(MINODE *wd)
{
char myname[120];
  if (wd == root){
    printf("/");
    return;
  }
  else
  {
  	int parent = findino(wd,wd->ino);
  	MINODE* pip = iget(dev,parent);
  	//printf("%d \n",parent);
  	findmyname(pip,wd->ino,&myname);
  	//printf("%d \n",wd->ino);
  	pwd(pip);
  	if(wd != running->cwd)
  	{
  	printf("%s/",myname);
  	}
  	else
  	{
  	printf("%s",myname);
  	}
	iput(pip);
  }
//	printf("%s/",myname);  
}



