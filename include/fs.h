int FSFileExists(char *filename);
int FSOpenFile(char *filename);
int FSFileSize(int file);
int FSFileSize(int file);
int FSReadFile(int file, int offset, void *buffer, int size);
void FSCloseFile(int file);
