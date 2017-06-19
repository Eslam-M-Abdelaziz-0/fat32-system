#include "fat32.h"

////////////	to read master boot record	///////////////

//char Path[7] = "\\\\.\\E:";
DWORD fat32::ReadBootSector(char Path[7] ,MBR* bootSector)
{
	HANDLE hFile= CreateFile(Path	// Floppy drive to open
		, GENERIC_READ				// Access mode
		, FILE_SHARE_READ			// Share Mode
		, NULL						// Security Descriptor
		, OPEN_EXISTING				// How to create
		, 0							// File attributes
		, NULL);						// Handle to template

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

/////////	read only frist 512 bytes

	DWORD readBytes = 0;
	ReadFile(hFile
		, (BYTE*)bootSector
		, sizeof(MBR)
		, &readBytes
		, 0);

	CloseHandle(hFile);

	if (bootSector->BPBSignature != 0xaa55)
		return 0;

	return readBytes;
}

/*
this function used to read one record(file or object) in fat table
*/

int fat32::ReadFAT32Links(char Path[7] ,MBR* bootSector, Directory* entry, vector<DWORD>& links)
{
	DWORD NumberOfLinksPerSector = bootSector->NumberOfBytesPerSector / sizeof(DWORD);	//to determine number of links per sector
	LONGLONG distance;	//used to determin next link
	LONG low, high;	//used to set low and high to determin link(DWORD)
	DWORD* FAT = new DWORD[NumberOfLinksPerSector];		//array of number of links per sector
	DWORD link, readBytes,index;

	HANDLE hFile= CreateFile(Path	// Floppy drive to open
		, GENERIC_READ				// Access mode
		, FILE_SHARE_READ			// Share Mode
		, NULL						// Security Descriptor
		, OPEN_EXISTING				// How to create
		, 0							// File attributes
		, NULL);					// Handle to template

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	link = MAKELONG(entry->LowClusterNumber, entry->HiClusterNumber); 
	/*
		Creates a LONG value by concatenating the specified values.
		DWORD x = (low) | (high << 16);
	*/
	while (true) //end of clusters chain
	{		
		links.push_back(link);

		distance = bootSector->NumberOfReservedSectors; 
		/*
		NumberOfReservedSectors
		-Some file systems may want a specific alignment of data and therefor will increase the number of reserved sectors to meet that alignment.
		-The number of sectors preceding the start of the first FAT, including the boot sector. The value of this field is always 1.
		-bits before fat and iclude MBR
		*/
		distance += link / NumberOfLinksPerSector;
		/*
		NumberOfLinksPerSector = bootSector->NumberOfBytesPerSector / sizeof(DWORD);
		//to determine number of links (DWORD) per sector
		*/
		distance *= bootSector->NumberOfBytesPerSector; 
		/*
		distance in bytes
		*/
		low = (ULONG)(distance);
		high = (ULONG)(distance >> 32);
		/*
		determine low and higth to determine next link
		*/
		SetFilePointer(hFile, low, &high, FILE_BEGIN);	
		/*
		to move file pointer to next link in fat 
		*/

		ReadFile(hFile
			, (BYTE*)FAT
			, bootSector->NumberOfBytesPerSector	//all sector is read after first link
			, &readBytes	//512 bit size of one sector
			, NULL);
		//test
		//cout<<readBytes<<endl;

		if (readBytes == 0)
		{
			CloseHandle(hFile);
			return 0;
		}

		index = link%NumberOfLinksPerSector;// 2 % 128
		//test
		//cout<<link<<"\t"<<NumberOfLinksPerSector<<endl;
		//cout<<index<<endl;
		link = FAT[index];	//fat[2]
		//test
		//cout<<link<<endl;
		if (link == 0x0fffffff) //end of clusters
			break;
	}

	CloseHandle(hFile);
	delete[] FAT;
	//test
	//cout<<"****** \n";
	return 1;
}


int fat32::ReadClusters(char Path[7] ,MBR* bootSector, vector<DWORD> links, vector<BYTE*>& clusters)
{
	LONGLONG distanceToCluster2 = bootSector->NumberOfReservedSectors + bootSector->NumberOfFATs*bootSector->NumberOfSectorsPerFAT32;
	/*
	put pointer to the first sector hold data in fat table
	*/
	DWORD readBytes ,	//used to put number of bits is read
		clusterSize = bootSector->NumberOfBytesPerSector*bootSector->NumberOfSectorsPerCluster;

	LONGLONG distance;
	LONG low, high;
	BYTE* cluster;	//array of sectors

	HANDLE hFile= CreateFile(Path	// Floppy drive to open
		, GENERIC_READ				// Access mode
		, FILE_SHARE_READ			// Share Mode
		, NULL						// Security Descriptor
		, OPEN_EXISTING				// How to create
		, 0							// File attributes
		, NULL);					// Handle to template

	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	for (int i = 0; i < links.size(); i++)
	{
		//			firsr sector			root
		distance= distanceToCluster2 + (links[i] - 2)*bootSector->NumberOfSectorsPerCluster;
		//test
		//cout<<distance<<"//\\"<<endl;
		/*
		to determin the next sector after the root
		*/
		distance *= bootSector->NumberOfBytesPerSector;
		/*
		distance in bytes
		*/
		low = (LONG)(distance);
		high = (LONG)(distance >> 32);
		/*
		determine low and higth to determine next cluster
		*/
		cluster = new BYTE[clusterSize];
		SetFilePointer(hFile, low, &high, FILE_BEGIN);	//move pointer to to continue read folder
		ReadFile(hFile, cluster, clusterSize, &readBytes, NULL);
		clusters.push_back(cluster);
	}
	CloseHandle(hFile);
	return 1;

}


int fat32::ReadDirectory(char Path[7] ,MBR* bootSector,Directory* directory, vector<Directory>& entries)
{
	DWORD clusterSize = bootSector->NumberOfBytesPerSector*bootSector->NumberOfSectorsPerCluster;
	vector<DWORD> links;	//to use in function (ReadFAT32Links)
	vector<BYTE*> clusters;	//to use in function (ReadClusters)
	DWORD NumberOfEntriesPerCluster = clusterSize / sizeof(Directory); //32 bit
	Directory* entry;	//array of 32 bit (directory)
	
	ReadFAT32Links(Path ,bootSector, directory, links);
	ReadClusters(Path ,bootSector, links, clusters);

	for (int i = 0; i < clusters.size(); i++)
	{
		entry = (Directory*)clusters[i];

		for (int j = 0; j < NumberOfEntriesPerCluster; j++)
		{
			if (entry[j].Name[0] == 0)//break when Name[0] = 0 , end of entries
				break;
			entries.push_back(entry[j]);
		}
	}
	return 1;
}


void fat32::DisplayDirectory(vector<Directory> entries)
{
	cout <<left << setw(10) << "Type" << setw(10) << "Size" << setw(11) << "Name "<<endl;
	
	for (int i = 0; i < entries.size(); i++)
	{
		if (entries[i].Name[0] == 0xE5) //Deleted entry 
			continue;
		if (entries[i].Attributes & 0xF == 0xF)// LFN entry complex more work
			continue;
		if (entries[i].Attributes & 0x8)//partition entry
			continue;
		if (entries[i].Attributes&FILE_ATTRIBUTE_DIRECTORY)
			cout << left<<setw(10) << "folder-->";
		else
			cout <<left<< setw(10) << "File---->";
		cout << left << setw(10) << entries[i].FileSize ;
		for (int j = 0; j < 11; j++)
		{
			cout << entries[i].Name[j];
		}
		cout<< endl;
	}
}


