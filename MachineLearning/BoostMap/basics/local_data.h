#ifndef VASSILIS_LOCAL_DATA_H
#define VASSILIS_LOCAL_DATA_H

#define LOCAL_ROOT_PATH "C:\\Users\\Diego\\Documents\\Workspace\\boostmap_code\\"
#define CODE_DIR LOCAL_ROOT_PATH "code64\\cpp\\"

#define DATA_DIR LOCAL_ROOT_PATH "data\\"
#define DATA_DIR_C DATA_DIR "c\\"
#define DATA_DIR_D DATA_DIR "d\\"
#define DATA_DIR_R DATA_DIR "r\\"
#define DATA_DIR_L DATA_DIR "l\\"
#define DATA_DIR_S DATA_DIR "s\\"

#define TEST_DIR LOCAL_ROOT_PATH "code64\\testing\\"
#define TEST_DIR_C TEST_DIR "c\\"
#define TEST_DIR_D TEST_DIR "d\\"
#define TEST_DIR_R TEST_DIR "r\\"
#define TEST_DIR_L TEST_DIR "l\\"

///*
extern const char * platform_face_detector_path;
extern const char * platform_skin_directory;
extern const char * platform_non_skin_directory;
extern const char * crl_skin_file;
extern const char * crl_non_skin_file;
extern const char * sequences_directory;
extern char * g_code_directory;
extern const char * g_data_directory;
extern char * nessie;
extern char * nessied;

static const char * platform_face_detector_path1 = DATA_DIR "face_detector\\";
static const char * platform_skin_directory1 = DATA_DIR "training\\skin_pc\\";
static const char * platform_non_skin_directory1 = DATA_DIR "training\\non_skin_pc\\";
static const char * crl_skin_file1 = DATA_DIR "crl_skin\\skin.32.pc";
static const char * crl_non_skin_file1 = DATA_DIR "crl_skin\\non_skin.32.pc";
static const char * sequences_directory1 = DATA_DIR "sequences\\";
static const char * g_default_code_directory = CODE_DIR;
static const char * g_default_data_directory = DATA_DIR;
static const char * nessie1 = DATA_DIR "nessie\\";
static const char * nessied1 = DATA_DIR "nessie\\";

#endif // VASSILIS_GESTURE_DATA_H
 