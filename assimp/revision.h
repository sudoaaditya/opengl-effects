#ifndef ASSIMP_REVISION_H_INC
#define ASSIMP_REVISION_H_INC

#define GitVersion 0xe2ee7d2c
#define GitBranch "master"

#define VER_MAJOR 5
#define VER_MINOR 4
#define VER_PATCH 3
#define VER_BUILD 0

#define STR_HELP(x) #x
#define STR(x) STR_HELP(x)

#define VER_FILEVERSION             VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD
#if (GitVersion == 0)
#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)
#else
#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD) " (Commit e2ee7d2c)"
#endif
#define VER_COPYRIGHT_STR           "\xA9 2006-2023"

#ifdef  NDEBUG
#define VER_ORIGINAL_FILENAME_STR   "assimp-vc143-mt.dll"
#else
#define VER_ORIGINAL_FILENAME_STR   "assimp-vc143-mtd.dll"
#endif //  NDEBUG

#endif // ASSIMP_REVISION_H_INC
