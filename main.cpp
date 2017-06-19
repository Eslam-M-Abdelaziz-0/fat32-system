/**
����� : ����� ���� ��� ������
����� : ���� ���� ��� ������ �����
���� : 1
��� : it
OS
������ : �������

�� ��������� ���� ������ �� ����� �����
���� ����� �� ���� ��� ������� ���� ����� ��� ����
������1 : ���� ����� �� ������
http://stackoverflow.com/questions/15495756/how-can-i-find-the-size-of-all-files-located-inside-a-folder/15501719
������ ����� �� ������� ����� 
text

*/

#include "fat32.h"


void main()
{
	//test
	//cout<<sizeof(fat32::MBR)<<endl;
	system("COLOR FC");
	fat32 fat;
	char Path[7] = "\\\\.\\H:";
	char letter;
	cout << "Enter Partition Label: ";
	cin >> letter;
	system("COLOR 0A");
	Path[4] = letter;

	fat32::MBR bootSector;
	fat32::Directory root;
	vector<fat32::Directory> list;

	root.LowClusterNumber = 2;
	root.HiClusterNumber = 0;
	fat.ReadBootSector(Path ,&bootSector);
	fat.ReadDirectory(Path ,&bootSector, &root, list);
	fat.DisplayDirectory(list);

	
	system("Pause");
}

