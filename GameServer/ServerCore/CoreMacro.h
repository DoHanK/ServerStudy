#pragma once

#define CRASH(cause) do{				\
										\
	uint32* crash = nullptr;			\
	__analysis_assume(crash!=nullptr);	\
	*crash = 0xDEADBEEF;}while(0)



#define ASSERT_CRASH(expr) do{		\
									\
	if(!(expr))						\
	{								\
		CRASH("ASSERT_CRASH");		\
		__analysis_assume(expr);	\
	}}while(0)	