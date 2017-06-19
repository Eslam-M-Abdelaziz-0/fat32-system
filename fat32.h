#undef UNICODE
#include<vector>
#include<iostream>
#include<string>
#include<cstring>
#include<iomanip>
#include<Windows.h>
using namespace std;

class fat32{

public:

/////////	master boot record structure	//////////////

#pragma pack(1)
struct MBR
{
	BYTE			JMP[3];
	BYTE			OEM[8];
	WORD			NumberOfBytesPerSector;
	BYTE			NumberOfSectorsPerCluster;
	WORD 			NumberOfReservedSectors;
	BYTE			NumberOfFATs;
	WORD			NumberOfRootEntries16;
	WORD			LowNumbferOfSectors;
	BYTE			MediaDescriptor;
	WORD			NumberOfSectorsPerFAT16;
	WORD			NumberOfSectorsPerTrack;
	WORD			NumberOfHeads;
	DWORD			NumberOfHiddenSectors;
	DWORD			HighNumberOfSectors;
	DWORD			NumberOfSectorsPerFAT32;
	WORD			Flags;
	WORD			FATVersionNumber;
	DWORD			RootDirectoryClusterNumber;
	WORD			FSInfoSector;
	WORD			BackupSector;
	BYTE			Reserver[12];
	BYTE			BiosDrive;
	BYTE			WindowsNTFlag;
	BYTE			Signature;
	DWORD			VolumeSerial;
	BYTE			VolumeLabel[11];
	BYTE			SystemID[8];
	BYTE			CODE[420];
	WORD			BPBSignature;
};

////////////	dirctory structure	//////////////

struct Directory
{
	BYTE Name[11];					
	BYTE Attributes;				
	BYTE Reserved;
	BYTE CreationTimeTenth;
	WORD CreationTime;
	WORD CreationDate;
	WORD LastAccessTime;
	WORD HiClusterNumber;		//	The high-order word of the new value.
	WORD WriteTime;
	WORD WriteDate;
	WORD LowClusterNumber;		//	The low-order word of the new value.
	DWORD FileSize;
};


////////////	to read master boot record	///////////////

DWORD ReadBootSector(char Path[7] ,MBR* bootSector);

///////////		this function used to read one record(file or object) in fat table

int ReadFAT32Links(char Path[7] ,MBR* bootSector, Directory* entry, vector<DWORD>& links);

//////////		to

int ReadClusters(char Path[7] ,MBR* bootSector, vector<DWORD> links, vector<BYTE*>& clusters);

/////////		

int ReadDirectory(char Path[7] ,MBR* bootSector,Directory* directory, vector<Directory>& entries);

/////////

void DisplayDirectory(vector<Directory> entries);

};