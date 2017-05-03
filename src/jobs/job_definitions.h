#ifndef JOB_DEFINITIONS_H_INCLUDED
#define JOB_DEFINITIONS_H_INCLUDED

#include "job.h"

enum class JobWantTo
{
	Nothing,
	Run,
	RunNext,
	Pause,
	Abort,
};

struct JobTicket
{
	vsedit::Job * pJob;
	JobWantTo wantTo;
	JobWantTo whenDone;
};

#endif // JOB_DEFINITIONS_H_INCLUDED
