////////////////////////////////////////
/// Zach Liss - University of Pittsburgh
/// Assignment 3, Compression
/// email: zll1@pitt.edu
/// date: 12/11/12
/// class: CS1550
/////////////////

/// code for lzw_encode and lzw_decode taken from:
/// http://rosettacode.org/wiki/LZW_compression

#include "header.h"

int num_elements = 0; /// the number of pieces of metadata
int offset = 0;		  /// the offset inside the .pitt file
int mindex = 0;		  /// the index in m
metadata* m;          /// array of pieces of metadata

int main(int argc, char* argv[]) {
	/*
	-c store in the archive file with designated name <archive-file> the files and/or directories provided
		by the list <file/directory-list>. The <archive-file> should feature the postfix .pitt so that it
		is clear it is a pittar archive. If additional files/directories exist in <directory-list>, then all this
		content is recursively stored in the <archive-file> as well.
	-a append filesystem entities indicated in the <file/directory list> in the existing archive file with
		name <archive-file>. If additional files and directories exist in <file/directory list>, they are
		recursively appended along with their content in the designated <archive-file>.
	-x extract all files and catalogs that have been archived in file .pitt file with name <archive-file>.
	-m print out the meta-data (owner, group, rights) for all files/directories that have been archived in
		<archive-file>.
	-p display the hierarchy(-ies) of the files and directories stored in the <archive-file>. Do this in a way
		that can be readily understood by the user.
	-j archive the files contained in <file/directory list> in compressed form while creating
		<archive-file>.
	*/

	if(strcmp(argv[1], "-c") == 0) {
		printf("Compressing files...\n");

		///create the name for the .pitt
		char* pitt_file_name = (char*) malloc(strlen(argv[2]) + 5);
		strcpy(pitt_file_name, argv[2]);
		strcat(pitt_file_name, ".pitt");

		/// compress the directory into the .pitt
		compress(argv[3], pitt_file_name);
		printf("final num elements is: %d\n", num_elements);
		/// free space for the pitt_file_name
		free(pitt_file_name);	
	} else if(strcmp(argv[1], "-a") == 0) {
		printf("I am supposed to append data now... but I don't feel like it ;)\n");
	} else if(strcmp(argv[1], "-x") == 0) {
		printf("Extracting files...\n");
		/// argv[2] is the .pitt file's name
		/// open it for reading.
		FILE* fp;
		if((fp = fopen(argv[2], "r")) == NULL) {
			perror("Error opening argv[2] in main: ");
		}
		fread(&num_elements, sizeof(int), 1, fp);
		printf("num_elements: %d\n", num_elements);
		fclose(fp);

		extract(argv[2]);
	} else if(strcmp(argv[1], "-p") == 0) {
		printf("Displaying Hierarchy(-ies)...\n");
		/// argv[2] is the .pitt file's name
		/// open it for reading.
		FILE* fp;
		if((fp = fopen(argv[2], "r")) == NULL) {
			perror("Error opening argv[2] in main: ");
		}
		fread(&num_elements, sizeof(int), 1, fp);
		printf("num_elements: %d\n", num_elements);

		/// fseek to the end - sizeof(metadata) * num_elements
		fseek(fp, (-1 * sizeof(metadata) * num_elements), SEEK_END);
		m = (metadata*) malloc(sizeof(m) * num_elements);
		fread(m, sizeof(metadata), num_elements, fp);

		int j;
		for(j = 0; j < num_elements; j++) {
			print_name(m[j]);
		}

		fclose(fp);
		free(m);
	} else if(strcmp(argv[1], "-m") == 0) {
		printf("Printing out metadata...\n");
		/// argv[2] is the .pitt file's name
		/// open it for reading.
		FILE* fp;
		if((fp = fopen(argv[2], "r")) == NULL) {
			perror("Error opening argv[2] in main: ");
		}
		fread(&num_elements, sizeof(int), 1, fp);
		printf("num_elements: %d\n", num_elements);

		/// fseek to the end - sizeof(metadata) * num_elements
		fseek(fp, (-1 * sizeof(metadata) * num_elements), SEEK_END);
		m = (metadata*) malloc(sizeof(m) * num_elements);
		fread(m, sizeof(metadata), num_elements, fp);

		int j;
		for(j = 0; j < num_elements; j++) {
			print_metadata(m[j]);
		}

		fclose(fp);
		free(m);
	}

	return 0;
}
void print_name(metadata m) {
	printf("%s\n", m.name);
}

void extract(char* pitt_file_name) {
	int i, j, status;
	char* buf;
	FILE* fp;
	if((fp = fopen(pitt_file_name, "r")) == NULL) {
		perror("Error opening .pitt file in extract(): ");
	}

	fseek(fp, (-1 * sizeof(metadata) * num_elements), SEEK_END);
	
	m = (metadata*) malloc(sizeof(m) * num_elements);
	
	fread(m, sizeof(metadata), num_elements, fp);

	/// first create all the directories
	for(j = 0; j < num_elements; j++) {
		printf("m[j].name: %s\n", m[j].name);
		if(m[j].dir) { /// if you encounter a dir
			printf("Calling mkdir() on %s...\n", m[j].name);
			if(mkdir(m[j].name, 0777) == -1) {
				perror("Error creating directory: ");
			}
			if(chmod(m[j].name, 0777) == -1) {
				perror("Error with chmod(): ");
			}
		}
	}
	
	/// now fill with the files
	for(i = 0; i < num_elements; i++) {
		if(m[i].dir == 0) { /// if you encounter a file
			printf("Decompressing %s...\n", m[i].name);
			decompress(i, pitt_file_name);
		}
	}
	free(m);
	fclose(fp);

}

void decompress(int metaindex, char* pitt_file_name) {
	int fd;
	FILE* fp;

	/// this will eliminate the .Z at the end of the file if it is there
	char* noZname;
	if((m[metaindex].name[(strlen(m[metaindex].name)) - 1] == 'Z') && (m[metaindex].name[(strlen(m[metaindex].name)) - 2] == '.')) {
		noZname = (char*) malloc(sizeof(m[metaindex].name));
		strncpy(noZname, m[metaindex].name, sizeof(m[metaindex].name));

		noZname[strlen(noZname) - 2] = '\0';
	}

	if((fp = fopen(pitt_file_name, "r")) == NULL) {
		perror("Error opening .pitt file in decompress(): ");
	}

	byte *in = _new(byte, m[metaindex].size);

	fseek(fp, m[metaindex].offset, SEEK_SET);
	fread(in, m[metaindex].size, 1, fp);
	_setsize(in, m[metaindex].size);
	fclose(fp);

	byte *dec = lzw_decode(in);

	//printf("%s\n", m[metaindex].name_trim);
	//fd = open(m[metaindex].name_trim, O_WRONLY | O_CREAT);
	fd = open(noZname, O_WRONLY | O_CREAT); 
	write(fd, dec, _len(dec));
	fchmod(fd, (mode_t) m[metaindex].perms);
	close(fd);

	_del(dec);
	_del(in);

	printf ("Decompressed: %s\n", (char*)noZname);
}

void add_meta(char* pitt_file_name) {
	FILE* fp;
	int i;

	fp = fopen(pitt_file_name, "a+");
	for(i = 0; i < num_elements; i++) {
		fwrite((m+i), sizeof(metadata), 1, fp);
	}
	fclose(fp);
}
void add_dir(char* dir_name, char* pitt_file_name) {
	DIR* dir;
	struct dirent* dir_info;
	struct stat st;
	char* pathname;
	FILE* fp;
	int fd;

	/// open dir_name
	dir = opendir(dir_name);

	/// get the info for the dir
	stat(dir_name, &st);

	/// update the metadata information at the proper index
	strcpy(m[mindex].name, dir_name);
	strcpy(m[mindex].name_trim, trim(dir_name));
	m[mindex].size = -1;
	m[mindex].offset = -1;
	m[mindex].dir = 1;
	m[mindex].perms = st.st_mode;
	strcpy(m[mindex].parent_folder, dir_name);
	strcpy(m[mindex].parent_folder_trim, trim(dir_name));
	mindex++;

	/// read through the dir
	while((dir_info = readdir(dir)) != (void*)0) {
		/// we need to ignore a few types of files
		if(dir_info->d_ino == 0) continue;
		if(dir_info->d_name[0] == '.') continue;
		if (strcmp(dir_info->d_name,".") == 0) continue;
		if (strcmp(dir_info->d_name, "..") == 0) continue;

		/// create the path to this element by appending on to the pathname
		pathname = (char*) malloc(strlen(dir_name) + strlen(dir_info->d_name) + 2);
		strcpy(pathname, dir_name);
		strcat(pathname, "/");
		strcat(pathname, dir_info->d_name);

		/// get the info for the item in the dir
		stat(pathname, &st);

		if((st.st_mode & S_IFMT) == S_IFDIR) { /// if its a dir add_dir again...
			add_dir(pathname, pitt_file_name);
		} else if((st.st_mode & S_IFMT) == S_IFREG) { /// else if its a file...
			/// only add the .Z files
			if((dir_info->d_name[(strlen(dir_info->d_name)) - 1] == 'Z') && (dir_info->d_name[(strlen(dir_info->d_name)) - 2] == '.')) {
				printf("Adding: %s\n", dir_info->d_name);

				/// update the metadata information at the proper index
				strcpy(m[mindex].name, pathname);
				strcpy(m[mindex].name_trim, trim(pathname));
				m[mindex].size = (int) st.st_size;
				m[mindex].offset = offset;
				m[mindex].dir = 0;
				m[mindex].perms = st.st_mode;
				strcpy(m[mindex].parent_folder, dir_name);
				strcpy(m[mindex].parent_folder_trim, trim(dir_name));
				mindex++;
				offset += st.st_size;

				/// read the data from the file
				fd = open(pathname, O_RDONLY);
				byte* buf = _new(byte, st.st_size);
				read(fd, buf, st.st_size);
				_setsize(buf, st.st_size);
				close(fd);

				/// write buf to the .pitt file
				fp = fopen(pitt_file_name, "a+");
				fwrite(buf, _len(buf), 1, fp);
				fclose(fp);

				/// free the buf
				_del(buf);
			}
		}

	}
	/// close the dir
	closedir(dir);

}
void add_file(char* filename, char* pitt_file_name) {
	FILE* fp;
	int fd;
	struct stat st;

	/// get info for the .Z
	stat(filename, &st);

	/// open .pitt file and write the header to it
	/// update offset
	fp = fopen(pitt_file_name, "a+");
	fwrite(&num_elements, sizeof(int), 1, fp);

	offset += sizeof(int);

	/// take .Z off the filename
	char* noZname = (char*) malloc(sizeof(filename));
	strcpy(noZname, filename);
	noZname[strlen(noZname) - 2] = '\0';

	/// update the metadata info at the proper index
	strcpy(m[mindex].name, filename);
	strcpy(m[mindex].name, noZname);
	m[mindex].size = st.st_size;
	m[mindex].offset = offset;
	m[mindex].dir = 0;
	m[mindex].root = 1;
	m[mindex].perms = st.st_mode;
	strcpy(m[mindex].parent_folder, "NO PARENT");
	strcpy(m[mindex].parent_folder_trim, "NO PARENT");

	/// read the data from the file
	fd = open(filename, O_RDONLY);
	byte* buf = _new(byte, st.st_size);
	read(fd, buf, st.st_size);
	_setsize(buf, st.st_size);
	close(fd);

	/// write buf to the .pitt file
	//fp = fopen(pitt_file_name, "a+");
	fwrite(buf, _len(buf), 1, fp);
	fclose(fp);

	/// free the buf
	_del(buf);
	free(noZname);

}

void compress(char* item_name, char* pitt_file_name) {
	struct stat st;
	/// get the information of the item
	stat(item_name, &st);
	int status;
	int i;
	FILE* fp;

	if((st.st_mode & S_IFMT) == S_IFDIR) { /// if its a directory...
		/// compress files to .Z in the hierarchy
		num_elements = traverse(item_name, 0);
		printf("num_elements: %d\n", num_elements);

		/// allocate the needed amount of space to store all the metadata
		/// for the end of the .pitt
		m = (metadata*) malloc(sizeof(m) * num_elements);

		/// increment the offset to reflect the addition of the header
		/// the header will be an int saying the number of pieces of metadata
		offset += sizeof(int);

		/// create the .pitt file and add on the header
		fp = fopen(pitt_file_name, "a+");
		fwrite(&num_elements, sizeof(int), 1, fp);
		/// we can close fp for now
		fclose(fp);

		/// need to wait until all the children are done creating the .Z files
		wait(&status);

		/// add the .Z files to the .pitt file
		add_dir(item_name, pitt_file_name);

		/// set this dir as the root
		for(i = 0; i < num_elements; i++) {
			if(strcmp(m[i].name, item_name) == 0) {
				m[i].root = 1;
			}
		}

		/// add the metadata[] m to the end of the .pitt file
		add_meta(pitt_file_name);
		free(m);

	} else if((st.st_mode & S_IFMT) == S_IFREG) { /// else if its a regular file...
		/// create a .Z filename for the file
		char* filename = (char*) malloc(strlen(item_name) + 2);
		strcpy(filename, item_name);
		strcat(filename, ".Z");

		/// compress the file to a .Z
		if(fork() == 0) {
			execl("./compress", "compress", item_name, NULL);
			perror("Could not execl compress: ");
		}

		/// allocate enough space for the metadata... in this case 1
		m = (metadata*) malloc(sizeof(metadata));

		/// wait until the compress is finished
		wait(&status);

		/// at the .Z to the .pitt file
		num_elements = 1;
		add_file(filename, pitt_file_name);
		add_meta(pitt_file_name);

		free(m);
		free(filename);

	}
}

int traverse(char* pathname, int depth) {
	DIR* dp;
	struct stat st;
	struct dirent* dt;
	char* nextpath;
	int nextdepth = depth + 1;
	int i = 0;
	int length = 0;
	int e = 1;


	/// open dir from pathname
	if((dp = opendir(pathname)) == (void*)0) {
		perror("Error opening directory: ");
	}

	/// iterate through each element in the directory
	while((dt = readdir(dp)) != (void*)0) {
		/* need to igonore:
		- d_ino == 0
		- d_name == "."
		- d_name == ".." */
		if(dt->d_ino == 0) continue;
		if(strcmp(dt->d_name, ".") == 0) continue;
		if(strcmp(dt->d_name, "..") == 0) continue;
		if(strcmp(dt->d_name, ".DS_Store") == 0) continue;


		/// edit the string nextpath to reflect the proper connection between
		/// dp and the current element
		nextpath = (char*)malloc(strlen(pathname) + strlen(dt->d_name) + 2);
		strcpy(nextpath, pathname);
		strcat(nextpath, "/");
		strcat(nextpath, dt->d_name);

		/// get the inode info this element
		stat(nextpath, &st);

		/// if its a regular file
		if((st.st_mode & S_IFMT) == S_IFREG) {
			length = strlen(dt->d_name);
			if(dt->d_name[length - 1] != 'Z') {
				
				printf("compressing: >%s\n", dt->d_name);
				e++;
				if(fork() == 0) {
					
					execl("./compress", "compress", nextpath, NULL);
					perror("Error forking: ");
				} 				
			}
		} else if((st.st_mode & S_IFMT) == S_IFDIR) { /// else if its a dir
			for(i = 0; i < nextdepth; i++) {
				printf("	");
			}
			printf("%s\n", dt->d_name);
			e += traverse(nextpath, nextdepth);
		}
		free(nextpath);
	}

	printf("num_elements: %d\n", num_elements);
	return e;
	closedir(dp);

}

char* trim(char * str) {
	char * end = str + strlen(str) - 1;
	if (*end == '/') {
    	while (*end == '/') {
			--end;
		}
    	*(end+1) = 0;

    	while (*end != '/' && end >= str) {
			--end;
		}
    	++end;
    	return end;
    } else {
    	while (*end != '/' && end >= str) {
			--end;
		}
    	++end;
    	return end;
    }
}

void print_metadata(metadata m) {
	printf("--- metadata ----------------\n");
	printf("name              : %s\n", m.name);
	printf("name_trim         : %s\n", m.name_trim);
	printf("size              : %d bytes\n", m.size);
	printf("offset            : %d bytes\n", m.offset);
	printf("dir               : %d\n", m.dir);
	printf("root              : %d\n", m.root);
	printf("perms             : %d\n", m.perms);
	printf("parent_folder     : %s\n", &m.parent_folder);
	printf("parent_folder_trim: %s\n", &m.parent_folder_trim);
}

byte* lzw_decode(byte *in) {
	byte *out = _new(byte, 4);
	int out_len = 0;
 
	inline void write_out(byte c)
	{
		while (out_len >= _len(out)) _extend(out);
		out[out_len++] = c;
	}
 
	lzw_dec_t *d = _new(lzw_dec_t, 512);
	int len, j, next_shift = 512, bits = 9, n_bits = 0;
	ushort code, c, t, next_code = M_NEW;
 
	uint32_t tmp = 0;
	inline void get_code() {
		while(n_bits < bits) {
			if (len > 0) {
				len --;
				tmp = (tmp << 8) | *(in++);
				n_bits += 8;
			} else {
				tmp = tmp << (bits - n_bits);
				n_bits = bits;
			}
		}
		n_bits -= bits;
		code = tmp >> n_bits;
		tmp &= (1 << n_bits) - 1;
	}
 
	inline void clear_table() {
		_clear(d);
		for (j = 0; j < 256; j++) d[j].c = j;
		next_code = M_NEW;
		next_shift = 512;
		bits = 9;
	};
 
	clear_table(); /* in case encoded bits didn't start with M_CLR */
	for (len = _len(in); len;) {
		get_code();
		if (code == M_EOD) break;
		if (code == M_CLR) {
			clear_table();
			continue;
		}
 
		if (code >= next_code) {
			fprintf(stderr, "Bad sequence\n");
			_del(out);
			goto bail;
		}
 
		d[next_code].prev = c = code;
		while (c > 255) {
			t = d[c].prev; d[t].back = c; c = t;
		}
 
		d[next_code - 1].c = c;
 
		while (d[c].back) {
			write_out(d[c].c);
			t = d[c].back; d[c].back = 0; c = t;
		}
		write_out(d[c].c);
 
		if (++next_code >= next_shift) {
			if (++bits > 16) {
				/* if input was correct, we'd have hit M_CLR before this */
				fprintf(stderr, "Too many bits\n");
				_del(out);
				goto bail;
			}
			_setsize(d, next_shift *= 2);
		}
	}
 
	/* might be ok, so just whine, don't be drastic */
	if (code != M_EOD) fputs("Bits did not end in EOD\n", stderr);
 
	_setsize(out, out_len);
bail:	_del(d);
	return out;
}
