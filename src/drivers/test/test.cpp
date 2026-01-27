#include "test.h"
#include <px4_platform_common/log.h>
#include <px4_platform_common/module.h>

extern "C" __EXPORT int test_main(int argc,char *argv[]){
	PX4_INFO("TEST SUCCESS!!argc = %d",argc);
	return 0;
}
