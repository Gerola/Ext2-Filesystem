
//Look Over all functions in this file....
//Make sure all iget have an iput as well...



char ga[128];
char *nam[64];
int v;


int tokenz(char* pathname)
{
   int i;
  char *s;
  strcpy(ga, pathname);
   v = 0;

  s = strtok(ga, "/");
  while(s){
    nam[v] = s;
    v++;
    s = strtok(0, "/");
  }
  nam[v] = 0;

}


//LOOK OVER...
int links(char* old_file,char* new_file) // links two files in the same dir
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
	MINODE* of = iget(dev,old);//get the INODE of the inode number
	if(S_ISDIR(of->INODE.i_mode))//check to make sure not a directory
	{
		
		printf("This is a directory\n");
		if(of != running->cwd)
		{
			iput(of);
		}
		return;
	}
	int new = getino(new_file);//make sure not created yet
	if(new)
	{
		printf("Already created\n");
		if(of != running->cwd)
		{
			iput(of);
		}
		return;
	}
	tokenz(new_file);//get the parent values
	MINODE* parent;//parent MINODE
	if(v == 1)
	{
		 parent = running->cwd;//only the name so CWD	
	}	
	else//need to find parent
	{
		new_file[strlen(new_file)-strlen(nam[v-1])-1] = 0;//look over
		int parent_ino = getino(new_file);//get the parent inode
		if(!parent_ino)//need to make sure exists
		{
			printf("Not found\n");
			if(of != running->cwd)
			{
				iput(of);
			}
			return;
		}
		parent = iget(dev,parent_ino);//get the inode
	}
	//pointer to the inode in the block;//write back to disk
	enter_name(parent,old,nam[v-1]);//enter the name into the parent directoy with the new name but same inode as the old file
	of->dirty = 1;//mark dirty
	of->INODE.i_links_count++;//increase link count
	if(parent != running->cwd)//put back
	{
		iput(parent);
	}
	if(of != running->cwd)//put back
	{
		iput(of);
	}
	
}


//Look Over as well...
int unlinks(char* pathname)
{

	if(strcmp(pathname,"") ==0)//Make sure not empty
	{
	
		printf("Need the name of a file\n");
		return;
	}

	int ino = getino(pathname);//get the inode number of the file to unlink
	if(!ino)
	{
	
		printf("Doesn't exist\n");
		return;
	}

	MINODE* file = iget(dev,ino);//get the INODE
	if(S_ISDIR(file->INODE.i_mode))//make sure not a directory
	{
		printf("This is a directory\n");
		if(file != running->cwd)
		{
			iput(file);
		}
		return;
	
	}
	tokenz(pathname);
	pathname[strlen(pathname)-strlen(nam[v-1])-1] = 0;//find the parent
	int parent_i = getino(pathname);//get the parent inode
	MINODE* parent;//parent inode
	if(v == 1)
	{
		parent = running->cwd;//CWD
	
	}
	else if (parent_i != 0)
	{
		parent = iget(dev,parent_i);//path
	
	}
	rm_child(parent,nam[v-1]);//remove from parent data block
	parent->dirty = 1;//mark dirty
	if(parent != running->cwd)//put back
	{
		iput(parent);
	}
	file->INODE.i_links_count--;//decrease link count
	if(file->INODE.i_links_count > 0)//greater than 0
	{
		file->dirty = 1;
	}
	else if (file->INODE.i_links_count == 0)//less than 0 delete
 	{
		//  if(file->INODE.i_block[0] != 0)//if data allocated or not
		//  {
		//  	bdalloc(dev, file->INODE.i_block[0]);//dealloc the data block
		//  }
		 truncate(file);
		 idalloc(dev, ino);//dealloc the INODE
	}
	iput(file);//put the file back to the file system
}






















