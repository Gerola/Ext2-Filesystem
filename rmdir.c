
//Functions in this file
//--------------rm_child--------------rmdir--------------

//Added reference count
//and putting stuff back
//end of rmdir for time and other things

char gpaths[128];
char *names[64];
int numbers;

int tokenize_rm(char* path)
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
    printf("%s \n",s);
  }
  names[numbers] = 0;
}



//Look over and test this function...

int rm_child(MINODE* pmip, char *name)
{
	char buf[BLKSIZE];
	char newbuf[BLKSIZE];//if in the middle or first in the block
	char temp[256];//compare the string
	char *cp;
	int go_back_value = 0, remain = 0;//get the second to last values and what remains to be added to the last value
	get_block(pmip->dev,pmip->INODE.i_block[0],buf);
	
	dp = (DIR *)buf;
   	cp = buf;
   	
   	while (cp < buf + BLKSIZE){//Name and less than
		
	strncpy(temp, dp->name, dp->name_len); // dp->name is NOT a string, copy string
	    temp[dp->name_len] = 0;                // temp is a STRING
	     
		
	if (strcmp(temp, name)==0){            // compare name with temp !!!
	printf("found the value to remove %s : ino = %d\n", temp, dp->inode);//found the value to be erased from the data block
	//go_back_value = dp->rec_len;//get the value of how much to go back after incrementing
	remain = dp->rec_len;//value to be added to the back of the data block
	break;
	     }
	     else{
	 go_back_value = dp->rec_len;//get the value of how much to go back after incrementing
    	 cp += dp->rec_len;//increment
    	 dp = (DIR *)cp;//have a dir struct
    	 }
     }
     //So now the dp should be pointing at the value to be deleted if the first if statement executes
     printf("\n\nREMAINING: %d\n",remain);
     if(cp + dp->rec_len >= buf + BLKSIZE)//this means it would be the last entry in the data block
     {
     	remain = dp->rec_len;//value to be added to the end of the block
     	cp -= go_back_value;//how much to go back
	dp = (DIR *)cp;//get a dir struct for second to last value
	dp->rec_len += remain;//add the remaining to the NEW last value in the data block
	put_block(pmip->dev,pmip->INODE.i_block[0],buf);//write back to the file system
     }
     else//in the middle or first and need to move everything to the left now
     { 
	bzero(newbuf,BLKSIZE);
	DIR* new_value = (DIR *)newbuf;
	dp = (DIR *)buf;//dir struct
	cp = buf;//start of buf
	while(cp < buf + BLKSIZE)
	{
	
	    strncpy(temp, dp->name, dp->name_len); // dp->name is NOT a string, copy string
	    temp[dp->name_len] = 0;                // temp is a STRING
	     
		
	    if (strcmp(temp, name)==0)
	     {      
	     	cp += dp->rec_len;//increase that amount of values
	        dp = (DIR *)cp;
	     }
	     else
	     {
	     	
	     	new_value->inode = dp->inode;//copy all the values over
	     	new_value->rec_len = dp->rec_len;//
	     	new_value->name_len = dp->name_len;//
	     	strcpy(new_value->name,dp->name);//
	     	cp += dp->rec_len;//increment by the length
    	     	dp = (DIR *)cp;//have a dir struct
    	     	
    	     	if(cp >= buf + BLKSIZE)
    	     	{
    	     		new_value->rec_len +=remain;//last value
    	     	}
    	     	
	 	printf(" NEW_VALUE: %d\n\n",new_value->rec_len);
	 	new_value = (char *) new_value + new_value->rec_len;
	 	
    	     	
    	 }
	}
	put_block(pmip->dev,pmip->INODE.i_block[0],newbuf);//write the new values to the data block
     }

}

int rmdirs(char* pathname)
{
	if(strcmp(pathname,".")==0)
	{
		printf("Can't delete this dir\n");
		return;
	}
	if(strcmp(pathname,"")==0)
	{
		printf("Need a name\n");
		return;
	}
	
	tokenize_rm(pathname);
	int removing = getino(pathname);//get inode of removing directory
	pathname[strlen(pathname)-strlen(names[numbers-1])-1] = 0;
	//	a/b/c
	int pino = getino(pathname);//will where the directory a/b is not c because of the -2
	MINODE* pmip;
	if(numbers == 1)//only the one
	{
		pmip = running->cwd;
	}
	else if (pino != 0)
	{
	  	pmip = iget(dev,pino);//inode of the directory with item to be removed
	}
	else
	{
		printf("Not found\n");
		if(pmip != running->cwd)
		{
			iput(pmip);
		}
		return;
	}
	
	//Not a directory
	if(!S_ISDIR(pmip->INODE.i_mode))
	{
		printf("Not a dir\n");
		if(pmip != running->cwd)
		{
			iput(pmip);
		}
		return;
	}
	if(search(pmip,names[numbers-1]))
	{
		printf("Directory is present in the directory\n");
	}
	
	printf("Inode: %d\n",removing);
	if(removing == 0)
	{
	if(pmip != running->cwd)
		{
			iput(pmip);
		}
		return;
	}
	MINODE* r = iget(dev,removing);//get the actual inode
	if(!S_ISDIR(r->INODE.i_mode))//check
	{
		printf("NOT a directory\n");
		iput(r);//put back if not
		if(pmip != running->cwd)
		{
			iput(pmip);
		}
		return;
	}
	//Added this
	if(r->refCount != 1)
	{
		printf("More than one eyes on this directory\n");
		printf("%d \n\n",r->refCount);
		iput(r);
		if(pmip != running->cwd)
		{
			iput(pmip);
		}
		return;
	}
	//Search through the directory to make sure nothing is present in the directory
	char temp[256];
	char buf[BLKSIZE];
	get_block(dev,r->INODE.i_block[0],buf);//have the block that contains the files of the directory
	printf("INODE: %d",r->INODE.i_block[0]);
	char* c = buf;
	DIR* dp = (DIR *)buf;
	while(c < buf + BLKSIZE){
	    strncpy(temp, dp->name, dp->name_len); // dp->name is NOT a string, copy string
	    temp[dp->name_len] = 0;                // temp is a STRING
		if(strcmp(temp,".") == 0 || strcmp(temp,"..") ==0)
		{
	     		c += dp->rec_len;//increase that amount of values
	        	dp = (DIR *)c;
			printf("%s \n",temp);			
		}
		else
		{
			printf("Not empty\n");
			printf("%s \n",temp);
			iput(r);//put back, not empty
			if(pmip != running->cwd)
			{
				iput(pmip);
			}
			return;
		}   
	
	
	}
	rm_child(pmip,names[numbers-1]);
	idalloc(dev,removing);//dealloc inode
	bdalloc(dev,r->INODE.i_block[0]);//dealloc block
	iput(r);//So the ref count will go down by one ran into an issue if not deallocated
	pmip->INODE.i_links_count--;
	pmip->dirty = 1;
	pmip->INODE.i_atime = pmip->INODE.i_ctime = pmip->INODE.i_mtime = time(0L);//reset the time
	if(pmip != running->cwd)
	{
		iput(pmip);
	}
}










