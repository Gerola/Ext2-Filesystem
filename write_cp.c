//Write to a file
int minimu(int one, int two)
{
	if(one < two)
	{
		return one;
	}
	return two;

}


int my_write(int fd, char *buf, int nbytes)
{
//	printf("%d %s %d\n",fd,buf,nbytes);
	OFT* place = running->fd[fd];
	if(place == 0)
	{
		printf("Nothing to write to...\n");
		return;
	}
	int blk = 0, startbyte = 0, lbk = 0;
	MINODE* mip = running->fd[fd]->minodePtr;
	int indirect[256], temp[256];
	char wbuf[BLKSIZE];
	char *cq = buf;
	
	while(nbytes > 0){
	 lbk = place->offset / BLKSIZE;
	 startbyte = place->offset % BLKSIZE;
	
		if(lbk < 12)
		{
			
			if(mip->INODE.i_block[lbk] == 0)
			{
				mip->INODE.i_block[lbk] = balloc(dev);
			}
			blk = mip->INODE.i_block[lbk];
		}
		else if (lbk >= 12 && lbk < 256 + 12)
		{
			if(mip->INODE.i_block[12] == 0)
			{
				mip->INODE.i_block[12] = balloc(dev);
				get_block(dev,mip->INODE.i_block[12],indirect);
				bzero(indirect, BLKSIZE);
				put_block(dev,mip->INODE.i_block[12],indirect);//with zero values
			}
			get_block(dev,mip->INODE.i_block[12],indirect);
			blk = indirect[lbk - 12];
			if(blk == 0)
			{
				blk = indirect[lbk - 12] = balloc(dev);
				put_block(dev,mip->INODE.i_block[12],indirect);
			}
		}
		else
		{
			if(mip->INODE.i_block[13] == 0)
			{
				mip->INODE.i_block[13] = balloc(dev);
				get_block(dev,mip->INODE.i_block[13],indirect);
				bzero(indirect, BLKSIZE);
				put_block(dev,mip->INODE.i_block[13],indirect);//with zero values
			}
			get_block(dev,mip->INODE.i_block[13],indirect);
			
			lbk -= (12 + 256);
			int i = lbk / 256;
			int j = lbk % 256;
			int second = 0;
			
			if(indirect[i] == 0)
			{
				indirect[i] = balloc(dev);
				get_block(dev,indirect[i],temp);
				bzero(temp,BLKSIZE);
				put_block(dev,indirect[i],temp);//with zero values
				put_block(dev,mip->INODE.i_block[13],indirect);
			}
			second = indirect[i];
			get_block(dev,indirect[i],indirect);
			if(indirect[j] == 0)
			{
				blk = indirect[j] = balloc(dev);
				get_block(dev,blk,temp);
				bzero(temp,BLKSIZE);
				put_block(dev,second,indirect);
			}
			
			/*
			get_block(dev,mip->INODE.i_block[13],indirect);//get the block of ints from 13
			lbk -= (12 + 256);
			int i = lbk / 256;
			int j = lbk % 256;
			get_block(dev,indirect[i],indirect);
			blk = indirect[j];//get the block from the int block
			*/
			
			
		}
		
		get_block(dev,blk,wbuf);
		bzero(wbuf,BLKSIZE);
		char *cp = wbuf + startbyte;
		int remain = BLKSIZE - startbyte;
		char copying[BLKSIZE];
		
		while(remain > 0)
		{
			//*cp++ = *cq++;
			//nbytes -= 1,
			///remain -= 1;
			//place->offset++;
			int write_length = minimu(nbytes,remain);
			
			strncpy(cp,cq,write_length);
			cq += write_length;
			cp += write_length;
			
			place->offset += write_length;
			nbytes -= write_length;
			remain -= write_length;
			
			
			if(place->offset > mip->INODE.i_size)
			{
				mip->INODE.i_size += write_length;
			}
			if(nbytes <= 0)
			{
				break;
			}
		}
		
		put_block(dev,blk,wbuf);
		//printf("%s this \n\n",wbuf);
	}
	
	mip->dirty = 1;
	return nbytes;
}

//from <fcntl.h>:
//O_RDONLY = 0
//O_WRONLY = 1
//O_CREAT = 64
//O_WRONLY|O_CREAT = 65
int my_cp(char* src, char* dst) {  // src is source, dst is destination
    
    int sfile = getino(src);//get the src file
	if(sfile == 0)//Doesn't exist
	{
		printf("Error, source file does not exist\n");
		return;
	}

    int dfile = getino(dst);//get the dst
	if(dfile == 0)//Doesn't exist
	{
		creats(dst);
	}
    
    int fd = my_open(src, 0); // fd = open src for READ
    int gd = my_open(dst, 1);
    int n;
    char mybuf[BLKSIZE]; //went with 1024, unsure if another number would fit better

    while( n = my_read(fd, mybuf, BLKSIZE)){
        my_write(gd, mybuf, n);
    }

    my_close(fd);
    my_close(gd);
    return 0;
}
