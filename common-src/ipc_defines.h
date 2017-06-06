#ifndef IPC_DEFINES_H_INCLUDED
#define IPC_DEFINES_H_INCLUDED

static const char JOB_SERVER_NAME[] = "vsedit_job_server";
static const uint16_t JOB_SERVER_PORT = 3370;

// Client messages
static const char MSG_GET_JOBS_INFO[] = "GJI";
static const char MSG_GET_LOG[] = "GL";
static const char MSG_SUBSCRIBE[] = "SS";
static const char MSG_UNSUBSCRIBE[] = "USS";
static const char MSG_CLOSE_SERVER[] = "CS";

// Server messages
static const char SMSG_JOBS_INFO[] = "JI";
static const char SMSG_COMPLETE_LOG[] = "LOG";
static const char SMSG_LOG_MESSAGE[] = "LM";
static const char SMSG_JOB_CREATED[] = "JC";
static const char SMSG_JOB_UPDATE[] = "JU";
static const char SMSG_JOB_STATE_UPDATE[] = "JSU";
static const char SMSG_JOB_PROGRESS_UPDATE[] = "JPU";

static const char SMSG_JOBS_SWAPPED[] = "JSW";
static const char JOBS_SWAPPED_ID1[] = "id1";
static const char JOBS_SWAPPED_ID2[] = "id2";

static const char SMSG_JOBS_DELETED[] = "JD";
static const char SMSG_REFUSE[] = "RF";
static const char SMSG_CLOSING_SERVER[] = "SCS";

#endif // IPC_DEFINES_H_INCLUDED
