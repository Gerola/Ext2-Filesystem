
//-----------------functions in this file-----------------
// mkdir,kmkdir,tokenize_dir,enter_name,creat

//Look over and test all the new functions...
char gpaths[128];
char *names[64];
int numbers;

int tokenize_dir(char* path)
{
   int i;
  char *s;

  strcpy(gpaths, path);
  numbers = 0;

  s = strtok(gpaths, "/");
  while(s){
    names[numbers] = s;
    numbers++;
    s = strtok(0, "/");
  }
  names[numbers] = 0;
}

//split pathname and directory to be created...
int mkdirs(char* pathname)
{
	if(strcmp(pathname,"") == 0)
	{
		printf("Please provide a name\n");
		return;
	}
	tokenize_dir(pathname);
	pathname[strlen(pathname)-strlen(names[numbers-1])-1] = 0;//look over
	//	a/b/c
	int pino = getino(pathname);//will where the directory a/b is not c because of the -2
	MINODE* pmip;
	if(numbers == 1)//only the one
	{
		pmip = running->cwd;
	}
	else if (pino != 0)
	{
	  	pmip = iget(dev,pino);//pathname
	}
	else
	{
		printf("Not found\n");
		return;
	}
	
	//Not a directory
	if(!S_ISDIR(pmip->INODE.i_mode))
	{
		printf("Not a dir\n");
		return;
	}
	if(search(pmip,names[numbers-1]))
	{
		printf("Already created\n");
		return;
	}
	
	kmkdir(pmip,names[numbers-1]);
	pmip->INODE.i_links_count++;
	pmip->dirty = 1;
	pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);//reset the
	if(pmip != running->cwd)
	{
		iput(pmip);
	}
}


int kmkdir(MINODE* pmip,char* base)
{
	int ino = ialloc(dev);
	int blk = balloc(dev);
	MINODE* mip = iget(dev,ino);//get the new inode
	INODE* ip = &mip->INODE;//pointer to the inode in the block
	ip->i_mode = 0x41ed;
	ip->i_uid = running->uid;
	ip->i_gid = running->gid;
	ip->i_size = BLKSIZE;
	ip->i_links_count = 2;
	time_t seconds = time(0L);
	ip->i_atime = ip->i_ctime = ip->i_mtime = seconds;
	
	//printf("%d \n\n",ip->i_mtime);
	ip->i_blocks = 2;
	ip->i_block[0] = blk;
	for(int t = 1; t < 15; t++){
	ip->i_block[t] = 0;
	}
	mip->dirty = 1;
	iput(mip);//write back to disk
	//now for the dir to contain the .. and . values
	char buf[BLKSIZE];
	bzero(buf,BLKSIZE);
	DIR* dp = (DIR *)buf;
	//. entry
	dp->inode = ino;//the current directory
	dp->rec_len = 12;
	dp->name_len = 1;
	dp->name[0] = '.';
	dp = (char *)dp + 12;
	dp->inode = pmip->ino;//parent
	dp->rec_len = BLKSIZE -12;
	dp->name_len = 2;
	dp->name[0] = dp->name[1] = '.';
	put_block(dev,blk,buf);//write the block to the data
	enter_name(pmip, ino,base);//enter the name into the parent directory
}

int enter_name(MINODE* pip, int ino, char* name)
{
	char buf[BLKSIZE];
	bzero(buf,BLKSIZE);
	get_block(pip->dev,pip->INODE.i_block[0],buf);
	//got the block of the parent
	DIR* dp = (DIR *) buf;
	char* cp = buf;
	while(cp + dp->rec_len < buf + BLKSIZE)
	{
		cp += dp->rec_len;
		dp = (DIR *) cp;
		printf("%d \n",dp->rec_len);
	}
	//dp points at the last record
	int ideal_length = 0;
	int need_length  = 0;
 	 ideal_length = (8 + dp->name_len + 3)/4;//ideal length of last entry
	 need_length = (8 + strlen(name) + 3)/4;//what needs to be added
	 ideal_length *= 4;
	 need_length *=4;
	// printf("I:%d N:%d",ideal_length,need_length);
	int remain = dp->rec_len - ideal_length;
	//printf("R:%d ",remain);
	dp->rec_len = ideal_length;//last one needs to be this value for ideal
	//printf("Length now: %d ",dp->rec_len);
	if(remain >= need_length)//needed to place the new last
	{
		dp = (char *) dp + ideal_length;
		dp->inode = ino;//child
		dp->rec_len = remain;//remains
		dp->name_len = strlen(name);//length of string
		strcpy(dp->name,name);
	}
	
	//printf("HERERE");
	put_block(pip->dev,pip->INODE.i_block[0],buf);
}

int creats(char *pathname)
{
	if(strcmp(pathname,"") == 0)
	{
		printf("Please provide a name\n");
		return;
	}
	tokenize_dir(pathname);
	pathname[strlen(pathname)-strlen(names[numbers-1])-1] = 0;//look over
	//	a/b/c
	int pino = getino(pathname);//will where the directory a/b is not c because of the -2
	MINODE* pmip;
	if(numbers == 1)//only the one
	{
		pmip = running->cwd;
	}
	else if (pino != 0)
	{
	  	pmip = iget(dev,pino);//pathname
	}
	else
	{
		printf("Not found\n");
		return;
	}
	
	//Not a directory
	if(!S_ISDIR(pmip->INODE.i_mode))
	{
		printf("Not a dir\n");
		return;
	}
	if(search(pmip,names[numbers-1]))
	{
		printf("Already created\n");
		return;
	}
	
	kcreat(pmip,names[numbers-1]);
	pmip->dirty = 1;
	pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);//reset the
	if(pmip != running->cwd)
	{
		iput(pmip);
	}
	
}

int kcreat(MINODE* pmip, char* base)
{
	int ino = ialloc(dev);
	MINODE* mip = iget(dev,ino);//get the new inode
	INODE* ip = &mip->INODE;//pointer to the inode in the block
	ip->i_mode = 0644 + 040000 + 040000;
	ip->i_uid = running->uid;
	for(int blocks = 0; blocks < 15; blocks++)
	{
		ip->i_block[blocks] = 0;
	}
	ip->i_block[0] = 0;
	ip->i_gid = running->gid;
	ip->i_size = 0;
	ip->i_links_count = 1;
	time_t seconds = time(0L);
	ip->i_atime = ip->i_ctime = ip->i_mtime = seconds;
	mip->dirty = 1;
	iput(mip);//write back to disk
	//now for the dir to contain the .. and . values
	enter_name(pmip, ino,base);//enter the name into the parent directory
}
