
//Files contained here:
//open_file close_file truncate lseek pfd

int my_open(char* filename,int mode)
{
	MINODE* mip;//Pointer
	int ino = getino(filename);//Get the inode number
	if(!ino)//if not created yet
	{
		creats(filename);//creat the file if it doesn't exist yet
		ino = getino(filename);//get the inode now
	}
	mip = iget(dev,ino);//actually get the inode
	if(!(mip->INODE.i_mode == 0644 + 040000 + 040000))//check type and permissions
	{
		printf("Not a regular file\n");//not regular
		iput(mip);//put the file back
		return;//return
	}
	int check = 0;
	for(check = 0; check < 64 && oft[check].refCount != 0; check++)//Make sure not already open and if so make sure for the correct mode
	{
		if(oft[check].minodePtr == mip)//Same inode
		{
			if(oft[check].mode != mode)//not the same mode
			{
				printf("This file is already open for a different mode\n");
				iput(mip);//put back
				return;//return
			}
			else
			{
				break;//found the file
			}
		}
	}
	
	OFT* of;
	int index = 0;
	int find = 0;
	int where = 0;
	if(oft[check].minodePtr == mip && oft[check].mode == mode && oft[check].refCount != 0)//Everything checks out
	{
			of = &oft[check];//have it point to that memory address
			of->refCount+=1;//increase refCount
			where = check;//this is where the proc will point to
	}
	else
	{
		for(find = 0; find < 64; find++)//Need to find a free space
		{
			if(oft[find].refCount == 0)//Nothing here
			{
				of = &oft[find];//find address
				of->mode = mode;//set mode
				of->minodePtr = mip;//set MINODE
				of->refCount = 1;//refCount =1
				
				//What type of mode
				switch(mode)
				{
				case 0: of->offset = 0;
					break;
				case 1: truncate(mip);
					of->offset = 0;
					break;
				case 2: of->offset = 0;
					break;
				case 3: of->offset = mip->INODE.i_size;
					break;
				default: printf("Invalid input\n");
					return;
				
				}
				where = find;//this is where the proc will point to
				break;
			}
		
		}
	}
	for(index = 0; index < 10; index++)//find a place for the fd
	{
		if(running->fd[index] == NULL)
		{
			running->fd[index] = &oft[where];//have it point to that place
			break;
		}
	
	}
	time_t seconds = time(0L);//get time
	if(mode == 0)//set the time either accessed or modified and accessed
	{
		mip->INODE.i_atime = time(0L);//check this...
	}
	else
	{
	mip->INODE.i_atime = mip->INODE.i_mtime = seconds;//upating time
	mip->dirty = 1;//only dirty if write or append
	}
	return index;

}


//Look Over again...
int truncate(MINODE* mip)
{
	///need to work on this part
	int blk;
	char values[BLKSIZE];
	int indirect[256];
	for(int index = 0; blk != 0 && index < 14; index++)
	{
		if(index < 12)
		{
			if(mip->INODE.i_block[index] != 0)
			{
				blk = mip->INODE.i_block[index];
				bdalloc(dev,blk);
				mip->INODE.i_block[index] = 0;
			}
			else
			{
				blk = 0;
			}
		
		}	
		else if (index == 12)
		{
			if(mip->INODE.i_block[12] != 0)
			{
				get_block(dev,mip->INODE.i_block[12],indirect);
				for(int values = 0; values < 256 && blk != 0; values++)
				{
					blk = indirect[values];
					if(blk != 0)
					{
						bdalloc(dev,blk);
					}
					
				}
				bdalloc(dev,mip->INODE.i_block[12]);
				mip->INODE.i_block[12] = 0;
			
			}
			else
			{
				blk = 0;
			}
		
		}
		else
		{
			if(mip->INODE.i_block[13] != 0)
			{
				int doublein[256];
				get_block(dev,mip->INODE.i_block[13],indirect);
				for(int values = 0; values < 256 && blk != 0; values++)
				{
					if(indirect[values] != 0)
					{
						get_block(dev,indirect[values],doublein);
						for(int doub = 0; doub < 256 && blk != 0; doub++)
						{
							blk = doublein[doub];
							if(blk != 0)
							{
								bdalloc(dev,blk);
							}	
							else
							{
								blk = 0;
							}
						
						}
						
						bdalloc(dev,indirect[values]);
					}
					else
					{
						blk = 0;
					}
					
				}
				bdalloc(dev,mip->INODE.i_block[13]);
				mip->INODE.i_block[13] = 0;
			
			}
			else
			{
				blk = 0;
			}
			
		
		}
	
	}
	mip->INODE.i_size = 0;//Updating the size of the minode size all blocks were deallocated
	
}

int my_close(int fd)
{
	printf("%d \n",fd);
	if(fd < -1 || fd > 10)//Only 10 places
	{
		printf("This is out of range\n");
		return;
	}
	
	OFT* place = running->fd[fd];//get the place
	if(place == 0)//nothing there
	{
		printf("Nothing to close\n");
		return;
	}
	
	running->fd[fd] = 0;//now empty
	place->refCount--;//decrease refCount
	if(place->refCount > 0)//If not zero still in use
	{
		return;
	}
	MINODE* mip = place->minodePtr;//no one else using
	iput(mip);//put back
	
	return 0;//return
}


//Just print the files descriptors in the process that are currently open
int pfd()
{
	OFT *place;
	MINODE* mp;
	char type[20];
	printf("fd \tmode\t offset\t INODE\n");
	printf("---------------------------\n");
	for(int index = 0; index < 10; index++)
	{
		if(running->fd[index] == 0)
		{
			continue;
		}
		place = running->fd[index];
		if(place->mode == 0)
		{
			strcpy(type,"READ");
		}
		else if(place->mode == 1)
		{
		
			strcpy(type,"WRITE");
		}
		else if (place->mode == 2)
		{
			strcpy(type,"READ/WRITE");
		}
		else
		{
			strcpy(type,"APPEND");
		}
		printf("%4d %5s %4d [%d,%d]\n",index,type,place->offset,place->minodePtr->dev,place->minodePtr->ino);
	}
	printf("---------------------------\n");
}



//Look Over...
//change position of the file being read or written to
int lseeks(int fd, int position)
{
	if(fd < -1 || fd > 10)
	{
		printf("This is out of range\n");
		return;
	}	
	OFT* find = running->fd[fd];
		if(find == 0)
	{
		printf("Nothing open at this location\n");
		return;
	}
	MINODE* mip = find->minodePtr;
	int file_size = mip->INODE.i_size;
	if(position > file_size || position < -1)
	{
		printf("Not enough items to read with that length\n");
		return;
	}
	find->offset = position;
	return;
}

