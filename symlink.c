

//Symlink and readlinks

int symlinks(char* old_file, char* new_file)
{
	if(strcmp(old_file,"")==0)//check to make sure names were given
	{
		printf("No file given to link for old\n");
		return;
	}
	if(strcmp(new_file,"")==0)//Check to make sure names were given
	{
		printf("No file given to link for new\n");
		return;
	}
	int old = getino(old_file);//get the old file that needs to be linked
	if(old == 0)//Doesn't exist
	{
		printf("This file does not exist yet\n");
		return;
	}
	int new = getino(new_file);//make sure not created yet
	if(new)//exists
	{
		printf("Already created\n");
		return;
	}
	//now the old file exists and the new files does not at this point
	tokenz(new_file);
	MINODE* parent;
	new_file[strlen(new_file)-strlen(nam[v-1])-1] = 0;
	int parent_ino = getino(new_file);//get the parent inode	
	if( v == 1)
	{
		parent = running->cwd;
	}
	else
	{
		if(!parent_ino)//need to make sure exists
		{
			printf("Not found\n");
			return;
		}
		parent = iget(dev,parent_ino);//get the inode
	}
	int ino = ialloc(dev);//new Inode
	int blk = balloc(dev);//block
	
	enter_name(parent, ino, nam[v-1]);//enter the name into the parent directory
	char buf[BLKSIZE];
	DIR* dp = (DIR *) buf;
	char* cp = buf;
	bzero(buf,BLKSIZE);//zero out
	tokenz(old_file);//now the old file
		
		dp->inode = parent_ino;//child
		dp->rec_len = 1024;//remains
		dp->name_len = strlen(nam[v-1]);//length of string
		strcpy(dp->name,nam[v-1]);
		
	MINODE* mip = iget(dev,ino);//get the new inode
	INODE* ip = &mip->INODE;//pointer to the inode in the block
	ip->i_mode = 0644 + 0xA000;
	ip->i_uid = running->uid;
	ip->i_block[0] = blk;
	ip->i_gid = running->gid;
	ip->i_size = strlen(nam[v-1]);
	ip->i_links_count = 1;
	time_t seconds = time(0L);
	ip->i_atime = ip->i_ctime = ip->i_mtime = seconds;
	mip->dirty = 1;
	iput(mip);//write back to disk
	put_block(dev,blk,buf);//write the block to the data
	
	parent->dirty = 1;
	if(parent != running->cwd)
	{
	
		iput(parent);
	}
}


int readlinks(MINODE* mip, char* buf)
{
	char temp[256];
	char b[BLKSIZE];
	if (!((mip->INODE.i_mode & 0xF000) == 0xA000))
	{
		printf("Not a link\n");
		return;
	}
	
	
	get_block(dev,mip->INODE.i_block[0],b);
	
	DIR* dp = (DIR *) b;
	char* cp = b;
	
	strncpy(temp, dp->name, dp->name_len);
     	temp[dp->name_len] = 0;
     
	strcpy(buf,temp);
	return dp->name_len;
}


