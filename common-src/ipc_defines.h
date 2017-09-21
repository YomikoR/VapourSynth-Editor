#ifndef IPC_DEFINES_H_INCLUDED
#define IPC_DEFINES_H_INCLUDED

// Watcher <-> Job server communication

static const char JOB_SERVER_NAME[] = "vsedit_job_server";
static const uint16_t JOB_SERVER_PORT = 3370;

// Client messages
static const char MSG_GET_JOBS_INFO[] = "GJI";
static const char MSG_GET_LOG[] = "GL";
static const char MSG_SUBSCRIBE[] = "SS";
static const char MSG_UNSUBSCRIBE[] = "USS";
static const char MSG_CLOSE_SERVER[] = "CS";
static const char MSG_GET_TRUSTED_CLIENTS[] = "GTC";
static const char MSG_SET_TRUSTED_CLIENTS[] = "STC";

static const char MSG_CREATE_JOB[] = "CJ";
static const char MSG_CHANGE_JOB[] = "CHJ";
static const char MSG_SET_JOB_DEPENDENCIES[] = "SJD";
static const char MSG_SWAP_JOBS[] = "SJ";
static const char MSG_RESET_JOBS[] = "RJ";
static const char MSG_DELETE_JOBS[] = "DJ";

static const char MSG_START_ALL_WAITING_JOBS[] = "SAWJ";
static const char MSG_PAUSE_ACTIVE_JOBS[] = "PACJ";
static const char MSG_RESUME_PAUSED_JOBS[] = "RPJ";
static const char MSG_ABORT_ACTIVE_JOBS[] = "AACJ";

// Server messages
static const char SMSG_JOBS_INFO[] = "JI";
static const char SMSG_COMPLETE_LOG[] = "LOG";
static const char SMSG_LOG_MESSAGE[] = "LM";
static const char SMSG_JOB_CREATED[] = "JC";
static const char SMSG_JOB_UPDATE[] = "JU";
static const char SMSG_JOB_STATE_UPDATE[] = "JSU";
static const char SMSG_JOB_PROGRESS_UPDATE[] = "JPU";
static const char SMSG_JOB_START_TIME_UPDATE[] = "JSTU";
static const char SMSG_JOB_END_TIME_UPDATE[] = "JETU";
static const char SMSG_JOB_DEPENDENCIES_UPDATE[] = "JDU";
static const char SMSG_JOBS_SWAPPED[] = "JSW";
static const char SMSG_JOBS_DELETED[] = "JD";
static const char SMSG_REFUSE[] = "RF";
static const char SMSG_CLOSING_SERVER[] = "SCS";
static const char SMSG_TRUSTED_CLIENTS_INFO[] = "TCI";

// Editor <-> Watcher communication

static const char JOB_SERVER_WATCHER_LOCAL_SERVER_NAME[] =
	"vsedit_job_server_watcher";

static const char WMSG_SHOW_WINDOW[] = "S";
static const char WMSG_CLI_ENCODE_JOB[] = "CEJ";

#endif // IPC_DEFINES_H_INCLUDED
