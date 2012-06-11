#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FILEOPS
#define _FILEOPS

bool fsop_GetFileSizeBytes(char *path, size_t *filesize);
u64 fsop_GetFolderBytes(char *source);
u32 fsop_GetFolderKb(char *source);
u32 fsop_GetFreeSpaceKb(char *path);

bool fsop_FileExist(const char *fn);
bool fsop_DirExist(char *path);
bool fsop_MakeFolder(char *path);
bool fsop_CopyFile(char *source, char *target);

bool fsop_CopyFolder(char *source, char *target, const char *gamename, const char *gameID);
void refreshProgressBar();

#endif

#ifdef __cplusplus
}
#endif