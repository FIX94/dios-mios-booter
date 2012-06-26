/*////////////////////////////////////////////////////////////////////////////////////////

fsop contains coomprensive set of function for file and folder handling

en exposed s_fsop fsop structure can be used by callback to update operation status

(c) 2012 stfour

////////////////////////////////////////////////////////////////////////////////////////*/
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
bool fsop_CopyFolder(char *source, char *target, const char *gamename, const char *gameID, const char *MIOS_inf);
void fsop_deleteFile(char *source);
void fsop_deleteFolder(char *source);

void refreshProgressBar();

#endif

#ifdef __cplusplus
}
#endif