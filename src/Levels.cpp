#include "Level.h"

#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"

TestEntry g_testEntries[] =
{
	{"Level 1", Level1::create},
	{"Level 2", Level2::create},
	{"Level 3", Level3::create},
	{"Level 4", Level4::create},
	{nullptr, nullptr}
};