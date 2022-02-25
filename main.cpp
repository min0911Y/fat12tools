#include <iostream>
#include <fstream>
#include <io.h>
#include <windows.h>
using namespace std;
string _file;
char *ptr_file;
string ___file;
int dictaddr = 0x2600;
string View_path = "";
int change_dict_times = 0;
void strtoupper(char *str);
void command_run(string commands);
// C:\Users\min0911\Desktop\DeskTop\项目\PowerintDos\Powerint DOS 386 0.5b\soures\Powerint_DOS_386.img
struct FILEINFO {
	unsigned char name[8], ext[3], type;
	char reserve[10];
	unsigned short time, date, clustno;
	unsigned int size;
};

#define ADR_DISKIMG &___file[0]
string path;
void print_date(unsigned short _date, unsigned short _time) {
	unsigned short year = _date & 65024;
	year = year >> 9;
	unsigned short month = _date & 480;
	month = month >> 5;
	unsigned short day = _date & 31;

	unsigned short hour = _time & 63488;
	hour = hour >> 11;
	unsigned short minute = _time & 2016;
	minute = minute >> 5;
	printf("%02u-%02u-%02u %02u:%02u", (year + 1980), month, day, hour, minute);
}
void clean(char *s, int n) {
	for (int i = 0; i < n; i++)
		s[i] = 0;
}
struct FILEINFO *file_search(char *name, struct FILEINFO *finfo, int max) {
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			return 0; /*没有找到*/
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/*将小写字母转换为大写字母*/
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max;) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if ((finfo[i].type & 0x18) == 0) {
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			return finfo + i; /*找到文件*/
		}
next:
		i++;
	}
	return 0; /*没有找到*/
}
struct FILEINFO *dict_search(char *name, struct FILEINFO *finfo, int max) {
	int i, j;
	char s[12];
	for (j = 0; j < 11; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			return 0; /*没有找到*/
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				/*将小写字母转换为大写字母*/
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0; i < max;) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].type == 0x10) {
			for (j = 0; j < 11; j++) {
				if (finfo[i].name[j] != s[j]) {
					goto next;
				}
			}
			return finfo + i; /*找到文件*/
		}
next:
		i++;
	}
	return 0; /*没有找到*/
}
struct FILEINFO *Get_File_Address(char *path1) {
	int bmpDict = dictaddr;
	char *path2 = (char *)malloc(strlen(path1) + 1);
	strcpy(path2, path1);
	strtoupper(path2);
	if (strncmp("A:\\", path2, 3) == 0 || strncmp("A:/", path2, 3) == 0) {
		path2 += 3;
		bmpDict = 0x2600;
	}
	if (path2[0] == '\\' || path2[0] == '/') {
		//跳过反斜杠和正斜杠
		for (int i = 0; i < strlen(path2); i++) {
			if (path2[i] != '\\' && path2[i] != '/') {
				path2 += i;
				break;
			}
		}
	}
	char *temp_name = (char *)malloc(128);
	for (int i = 0; i <= 128; i++) {
		temp_name[i] = 0;
		//TODO
	}
	struct FILEINFO *finfo;
	int i = 0;
	while (1) {
		int j;
		for (j = 0; i < strlen(path2); i++, j++) {
			if (path2[i] == '\\' || path2[i] == '/') {
				temp_name[j] = '\0';
				//printf("Got It:%s,ALL:%s\n", temp_name, path);
				i++;
				break;
			}
			temp_name[j] = path2[i];
		}

		finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
		if (finfo == 0) {

			if (path2[i] != '\0') {
				//printf("last file:%s\n", temp_name);
				return 0;
			}
			finfo = file_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
			if (finfo == 0) {

				//printf("Invalid file:%s\n", temp_name);
				return 0;
			} else {
				goto END;
			}
		} else {
			// printf("dict_search:%s\n", temp_name);
			if (finfo->clustno != 0) {
				bmpDict = (finfo->clustno * 512 + 0x003e00);
			} else {
				//print("finfo->clustno == 0\n");
				bmpDict = 0x002600;
			}
			clean(temp_name, 128);
		}
	}
END:
	free(temp_name);
	free(path2);

	return finfo;
}
struct FILEINFO * clust_sech(int clustno, struct FILEINFO *finfo, int max) {
	int i, j;
	j = 0;
	for (i = 0; i < max; i++) {
		if (finfo[i].clustno == clustno) {
			return finfo + i;


		}
	}
	return 0; /*没有找到*/
}
void strtoupper(char *str) {
	while (*str != '\0') {
		if (*str >= 'a' && *str <= 'z') {
			*str -= 32;
		}
		str++;
	}
}

void changedict(char *dictname) {
	struct FILEINFO *finfo = dict_search(dictname, (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
	if (finfo == (struct FILEINFO *)(0)) {
		printf("Invalid directory.\n");
		return;
	}
	if (finfo->clustno == 0) {
		strcpy((char *)&View_path[0], "");
		change_dict_times = 0;
		dictaddr = 0x002600; // 锟斤拷目录
		return;
	}
	if (strcmp(dictname, "..") != 0 && strcmp(dictname, ".") != 0) {
		if (change_dict_times == 0) {
			strcat((char *)&View_path[0], dictname);
		} else {
			strcat((char *)&View_path[0], "\\");
			strcat((char *)&View_path[0], dictname);

		}
	}

	if (strcmp(dictname, "..") == 0) {
		int i;
		// print("OK\n");
		for (i = strlen((char *)&View_path[0]) - 1; i >= 0; i--) {
			if (View_path[i] == '\\') {
				View_path[i] = '\0';
				break;
			}
		}
		change_dict_times -= 2; //锟饺会还要++锟斤拷锟斤拷-=2
	}
	dictaddr = (finfo->clustno * 512 + 0x003e00);
	change_dict_times++;
	return;
}
int Get_dictaddr(char *path1) {
	int bmpDict = dictaddr;
	char *path = (char *)malloc(strlen(path1) + 1);
	strcpy(path, path1);
	strtoupper(path);
	if (strncmp("A:\\", path, 3) == 0 || strncmp("A:/", path, 3) == 0) {
		path += 3;
		bmpDict = 0x2600;
	}
	if (path[0] == '\\' || path[0] == '/') {
		//跳过反斜杠和正斜杠
		for (int i = 0; i < strlen(path); i++) {
			if (path[i] != '\\' && path[i] != '/') {
				path += i;
				break;
			}
		}
	}
	char *temp_name = (char *)malloc(128);
	struct FILEINFO *finfo;
	int i = 0;
	while (1) {
		int j;
		for (j = 0; i < strlen(path); i++, j++) {
			if (path[i] == '\\' || path[i] == '/') {
				temp_name[j] = '\0';
				// printf("Got It:%s,ALL:%s\n", temp_name, path);
				i++;
				break;
			}
			temp_name[j] = path[i];
		}

		finfo = dict_search(temp_name, (struct FILEINFO *)(ADR_DISKIMG + bmpDict), 224);
		if (finfo == 0) {
			return bmpDict;
		} else {
			// printf("dict_search:%s\n", temp_name);
			if (finfo->clustno != 0) {
				bmpDict = (finfo->clustno * 512 + 0x003e00);
			} else {
				bmpDict = 0x002600;
			}
			clean(temp_name, 128);
			if (path[i] == '\0') {
				goto END;
			}
		}
	}
END:
	free(temp_name);
	free(path);
	return bmpDict;
}
char *fopen(char *name) {
	struct FILEINFO *finfo;
	finfo = Get_File_Address(name);
	if (finfo == 0) {
		return 0;

	} else {
		char *p = (char *)(finfo->clustno * 512 + 0x003e00 + ADR_DISKIMG);
		return p;
	}
}
void cmd_dir() {
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + dictaddr);
	int i, j, k, line = 0;
	char s[30];
	for (i = 0; i != 30; i++) {
		s[i] = 0;
	}
	printf("FILENAME   EXT    LENGTH       TYPE   DATE\n");
	for (i = 0; i < 224; i++, line++) {
		if (finfo[i].name[0] == 0x00) {
			break;
		}
		if (finfo[i].name[0] != 0xe5) {
			if ((finfo[i].type & 0x18) == 0 || finfo[i].type == 0x10) {

				for (j = 0; j < 8; j++) {
					s[j] = finfo[i].name[j];
				}
				s[9] = finfo[i].ext[0];
				s[10] = finfo[i].ext[1];
				s[11] = finfo[i].ext[2];

				if (s[0] != '+') {
					for (k = 0; k < 12; ++k) {
						if (k == 9) {
							printf("   ");
						}
						if (s[k] == '\n') {
							printf("   ");
						} else {

							printf("%c", s[k]);
						}
					}
					printf("    ");
					printf("%d", finfo[i].size);
					// gotoxy(31, get_y());
					if ((finfo[i].type & 0x18) == 0) {
						printf("<FILE> ");
					}
					if (finfo[i].type == 0x10) {
						printf("<DIR>  ");
					}
					print_date(finfo[i].date, finfo[i].time);
					printf("\n");
				}
			}
		}
	}
	printf("\n");
	//&s = 0;
	return;
}
void saveImage(void) {
	fstream out;
	out.open(path, ios::out | ios::binary);
	out << ___file;

}
void mkfile(char *name) {
	char s[12];
	int i, j;
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr(name));

	for (i = strlen(name); i >= 0; i--) {
		if (name[i] == '/' || name[i] == '\\') {
			name += i + 1;
			break;
		}
	}

	for (j = 0; j != 12; j++) {
		s[j] = ' ';
	}
	j = 0;
	for (i = 0; name[i] != 0; i++) {
		if (j >= 11) {
			return;
		}
		if (name[i] == '.' && j <= 8) {
			j = 8;
		} else {
			s[j] = name[i];
			if ('a' <= s[j] && s[j] <= 'z') {
				s[j] -= 0x20;
			}
			j++;
		}
	}
	for (i = 0;; i++) {
		if (finfo[i].name[0] == 0x00) {
			finfo = finfo + i;
			break;
		}
	}
	for (i = 0; i != 8; i++) {
		finfo->name[i] = s[i];
	}
	for (i = 8; i != 11; i++) {
		finfo->ext[i - 8] = s[i];
	}
	finfo->type = 0x20;
	finfo->clustno = finfo[-1].clustno + (finfo[-1].size / 512) + 1;
	for (i = 0; i != 10; i++) {
		finfo->reserve[i] = 0;
	}
	finfo->time = 0;
	finfo->date = 0;
	finfo->size = 0;
	finfo[1].name[0] = 0x00; //堵塞下一个
	return;
}
void mkdir(char *dictname, int last_clust) {
	mkfile(dictname);
	struct FILEINFO *finfo = Get_File_Address(dictname);
	char *p = fopen(dictname);
	finfo->type = 0x10; // 锟斤拷锟侥硷拷锟斤拷为目录
	// 模锟斤拷
	struct FILEINFO dictmodel1;
	struct FILEINFO dictmodel2;
	struct FILEINFO null;
	memcpy(null.name, "NULL       ", 11);
	null.type = 0x20; // 锟斤拷锟斤拷锟侥硷拷
	null.size = 0;
	null.date = 0;
	null.time = 0;
	null.clustno = finfo->clustno + 2; // 预锟斤拷1024Bytes锟斤拷锟斤拷目录
	dictmodel1.name[0] = '.';
	for (int i = 1; i != 8; i++)
		dictmodel1.name[i] = ' ';
	for (int i = 0; i != 3; i++)
		dictmodel1.ext[i] = ' ';
	dictmodel1.type = 0x10;
	dictmodel1.clustno = finfo->clustno; // 指锟斤拷锟皆硷拷
	dictmodel1.size = 0;
	dictmodel1.date = 0;
	dictmodel1.time = 0;
	dictmodel2.name[0] = '.';
	dictmodel2.name[1] = '.';
	for (int i = 2; i != 8; i++)
		dictmodel2.name[i] = ' ';
	for (int i = 0; i != 3; i++)
		dictmodel2.ext[i] = ' ';
	dictmodel2.clustno = last_clust;

	dictmodel2.size = 0;
	dictmodel2.date = 0;
	dictmodel2.time = 0;
	dictmodel2.type = 0x10;
	memcpy(p, &dictmodel1, 32);
	char q[] = "\0";
	
	memcpy(&p[32], &dictmodel2, 32);
	memcpy(&p[64], &null, 32);
	memcpy(&p[96], &q, 1);
	
	
	return;
}
void shell() {
	while (1) {
		printf("%s:>", (char *)&View_path[0]);
		string _cmd = "";
		getline(cin, _cmd);
		command_run(_cmd);
		printf("\n");
	}
}
void Copy(char *path, char *path1) {
	mkfile(path1);
	struct FILEINFO *finfo = Get_File_Address(path1);
	if (finfo == 0) {
		return;
	}
	char *p1 = fopen(path1);
	fstream in;
	in.open(path, ios::in | ios::binary);
	if (!in.is_open()) {
		return;
	}
	in.seekg(0, ios::end);
	int size = in.tellg();
	in.seekg(0, ios::beg);
	char *p = new char[size];
	in.read(p, size);
	in.close();
	memcpy(p1, p, size);
	// copy info
	finfo->size = size;
	finfo->date = 0;
	finfo->time = 0;
	int Bmp = dictaddr;
	dictaddr = Get_dictaddr(path1);
	while (dictaddr != 0x2600) {
		if (dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224) != 0) {
			struct FILEINFO *finfo_this_dict_clust = dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
			struct FILEINFO *finfo_this_dict = clust_sech(finfo_this_dict_clust->clustno, (struct FILEINFO *)(ADR_DISKIMG + Get_dictaddr("../")), 224);
			finfo_this_dict->size += size;
		}
		changedict("..");
	}
	dictaddr = Bmp;
	return;
}
void command_run(string commands) {
	if (commands == "cls") {
		system("cls");
	} else if (commands == "dir") {
		cmd_dir();
	} else if (strncmp(commands.c_str(), "cd ", 3) == 0) {

		char *ptr = (char *)commands.c_str();
		changedict(ptr + 3);
	} else if (strncmp(commands.c_str(), "mkfile ", 7) == 0) {
		char *ptr = (char *)commands.c_str();
		mkfile(ptr + 7);

	} else if (strncmp(commands.c_str(), "mkdir ", 6) == 0) {
		char *ptr = (char *)commands.c_str();
		if (change_dict_times == 0) {
			mkdir(ptr + 6, 0);
		} else {
			struct FILEINFO *finfo = dict_search(".", (struct FILEINFO *)(ADR_DISKIMG + dictaddr), 224);
			mkdir(ptr + 6, finfo->clustno);
		}

	} else if (strncmp(commands.c_str(), "copy ", 5) == 0) {
		char *ptr = (char *)commands.c_str();
		char path_1[100];
		char path_2[100];
		ptr += 5;
		int i;
		for (i = 0; *ptr != ' '; ptr++, i++)
			path_1[i] = *ptr;
		path_1[i] = 0;
		ptr++;
		for (i = 0; *ptr != 0; ptr++, i++)
			path_2[i] = *ptr;
		path_2[i] = 0;

		Copy(path_1, path_2);


	} else if (commands == "save") {
		saveImage();
	} else {
		cout << "Bad Command!\n";
	}
}
int main(int argc, char const *argv[]) {
	/*
		状态码：
		返回-1：打开文件失败
		返回1：未打开文件
		返回0：一切正常
	*/
	bool Path_Ok = false;
	path = "";
	fstream file;
	if (argc == 1) {
		goto _Start;
	} else {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-cons") == 0) {
				goto _Start; //Console
			} else if (strcmp(argv[i], "-file") == 0) {
				path = argv[++i];
				Path_Ok = true;
				file.open(path, ios::in | ios::binary);
				if (!file) {
					cout << "打开失败！" << path << endl;
					return -1;
				}
				file.seekg(0, ios::end);
				___file.resize(file.tellg());
				file.seekg(0, ios::beg);
				file.read(&___file[0], ___file.size());
			} else if (strcmp(argv[i], "-mkdir") == 0) {
				if (Path_Ok) {
					char buf[128];
					sprintf(buf, "mkdir %s", argv[++i]);
					command_run(buf);
				}

			} else if (strcmp(argv[i], "-mkfile") == 0) {
				if (Path_Ok) {
					char buf[128];
					sprintf(buf, "mkfile %s", argv[++i]);
					command_run(buf);
				} else {
					return 1;
				}

			} else if (strcmp(argv[i], "-copy") == 0) {
				if (Path_Ok) {
					char buf[128];
					sprintf(buf, "copy %s %s", argv[++i], argv[++i]);
					command_run(buf);
				} else {
					return 1;
				}
			} else {
				cout << "wrong arg--->" << argv[i] << endl;
			}
		}

	}
	if (Path_Ok) {
		saveImage();
	}
	return 0;
_Start:
	if (!Path_Ok) {
		cout << "请输入镜像地址，以进入控制台：";

		getline(cin, path);

		file.open(path, ios::in | ios::binary);
		if (!file) {
			cout << "打开失败！" << endl;
			return 0;
		}
	}


	//读取到 ___file
	file.seekg(0, ios::end);
	___file.resize(file.tellg());
	file.seekg(0, ios::beg);
	file.read(&___file[0], ___file.size());
	shell();

	return 0;
}

