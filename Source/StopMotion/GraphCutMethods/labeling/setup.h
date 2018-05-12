#pragma once

typedef enum {
	OPTIMIZE_TRUE_VERTEX,
	OPTIMIZE_BSHAPES,
	OPTIMIZE_VERTEX,
	COMPUTE_DIFFERENT_ENERGIES_BSHAPES,
	COMPUTE_DIFFERENT_ENERGIES_VERTEX
} WORK_TYPE;

#define D_ATHOME 0
#define D_USE_VELOCITY_TERM 1
#define D_OPTIMIZE_LOWER 1
#define D_USE_KMEANS_INITALIZATION 1
const int D_WORK_TYPE = OPTIMIZE_TRUE_VERTEX; // if 0 then optimize for vertex distances
const int NUM_LABELS = 40;

#if D_ATHOME
	char* OLDMAN_PATH = "C:/Users/rindo/Google Drive/Projects/Claymation/data/oldman/gotolunch";
	char* OLDMAN_PATH_VELOCITY_OPEN_MOUTH = "C:/Users/rindo/Google Drive/Projects/Claymation/data/oldman/gotolunch/velocity_open_mouth";
	char* OLDMAN_PATH_VELOCITY_SMILE = "C:/Users/rindo/Google Drive/Projects/Claymation/data/oldman/gotolunch/velocity_smile";
	char* VALLEY_GIRL_PATH = "C:/Users/rindo/Google Drive/Projects/Claymation/data/valleygirl/hugged";
	char* FLAG_PATH = "C:/Users/rindo/Google Drive/Projects/Claymation/data/flag";
	char* FOLDER_PATH = FLAG_PATH;
#else
	char* OLDMAN_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/oldman/gotolunch";
	char* OLDMAN_PATH_VELOCITY_OPEN_MOUTH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/oldman/gotolunch/velocity_open_mouth";
	char* OLDMAN_PATH_VELOCITY_SMILE = "C:/Users/rinat/Google Drive/Projects/Claymation/data/oldman/gotolunch/velocity_smile";
	char* VALLEY_GIRL_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/valleygirl/hugged";
	char* FLAG_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/flag";
	char* HELLO_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/hello";
	char* HELLO_MERGED_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/hello_merged";
	char* UW_PATH = "C:/Users/rinat/Google Drive/Projects/Claymation/data/uw";
	char* FOLDER_PATH = UW_PATH;
#endif
