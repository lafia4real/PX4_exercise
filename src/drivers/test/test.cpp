/*
重构代码成PX4风格
并给函数加输入校验与错误日志
*/
#include "test.hpp"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int TestModule::task_spawn(int argc,char *argv[]){
	_task_id = px4_task_spawn_cmd(
		"test",
		SCHED_DEFAULT,
		SCHED_PRIORITY_DEFAULT,
		2000,
		(px4_main_t)&run_trampoline,
		(char *const *)argv
	);

	if(_task_id < 0){
		PX4_ERR("task start failed (%d)!!",errno);
		_task_id = -1;
		return errno;
	}

	return PX4_OK;
}

TestModule *TestModule::instantiate(int argc,char *argv[]){
	uint32_t period_ms = 200;
	uint32_t max_count = 20;

	if(parse_args(argc,argv,period_ms,max_count) != PX4_OK){
		return nullptr;
	}

	auto *obj = new TestModule();
	obj->_period_us = period_ms * 1000U;
	obj->_max_count = max_count;
	return obj;
}

int TestModule::custom_command(int argc,char *argv[]){
	(void)argc;
	(void)argv;
	return print_usage("unknown command");
}

int TestModule::print_usage(const char *reason){
	if(reason){
		PX4_WARN("%s",reason);
	}

	PX4_INFO("usage:test {start! | stop! | status } [-p <period_ms>] [-n <max_count>]");
	PX4_INFO(" -p <period_ms> loop period in ms (10...5000),default 200");
	PX4_INFO(" -n <max_count> count limit (1...100000),default 20");
	PX4_INFO("example:test start -p 200 -n 20");
	return PX4_OK;
}

int TestModule::print_status(){
	PX4_INFO("running");
	PX4_INFO("state = %s counter = %u period_ms = %u max_count = %u",
		(_state == State::IDLE) ? "IDLE" : "WORK",
		(unsigned)_counter,
		(unsigned)(_period_us / 1000U),
		(unsigned)_max_count);
	return PX4_OK;
}

void TestModule::run(){
	PX4_INFO("run start (period = %ums max_count = %u)",
		(unsigned)(_period_us / 1000U),
		(unsigned)_max_count);

	while(!should_exit()){
		switch(_state){
			case State::IDLE:
				PX4_INFO("[IDLE]SWITCH TO THE WORK!!");
				_state = State::WORK;
				_counter = 0;
				break;
			case State::WORK:
				PX4_INFO("[WORK]WORK STARTING...counter = %u",(unsigned)_counter);
				if(_counter++ >= _max_count){
					PX4_INFO("[WORK] WORK DONE,BACK TO THE IDLE...");
					_state = State::IDLE;
				}
				break;
		}
		px4_usleep(1000000);
	}
	PX4_INFO("run exit");
}

//---------------PX4填空：输入校验+错误日志----------------------//
int TestModule::parse_u32(const char *s,uint32_t &out,uint32_t minv,uint32_t maxv,const char *name){
	errno = 0;
	char *end = nullptr;
	long v = strtol(s,&end,10);

	if(errno != 0 || end == s || *end != '\0'){
		PX4_ERR("invalid %s : '%s' (must be integer)",name,s);
		return -EINVAL;
	}
	if(v < (long)minv || v > (long)maxv){
		PX4_ERR("%s out of range: %ld (valid %u..%u)",name,v,minv,maxv);
		return -ERANGE;
	}
	out = (uint32_t)v;
	return PX4_OK;
}

int TestModule::parse_args(int argc,char *argv[],uint32_t &period_ms,uint32_t &max_count){
	period_ms = 200;
	max_count = 20;

	//argv[0] 通常是 “start”，所以从optind = 1开始解析
	int ch;
	int myoptind = 1;
	const char *myoptarg = nullptr;

	while((ch = px4_getopt(argc,argv,"p:n:",&myoptind,&myoptarg)) != EOF){
		switch(ch){
			case 'p':
				if(!myoptarg){
					PX4_ERR("missing value for -p");
					print_usage();
					return -EINVAL;
				}
				if(parse_u32(myoptarg,period_ms,10,5000,"period_ms") != PX4_OK){
					print_usage();
					return -EINVAL;
				}
				break;
			case 'n':
				if(!myoptarg){
					PX4_ERR("missing value for -n");
					print_usage();
					return -EINVAL;
				}
				if(parse_u32(myoptarg,max_count,1,100000,"max_count") != PX4_OK){
					print_usage();
					return -EINVAL;
				}
				break;
			default:
				PX4_ERR("unknown option");
				print_usage();
				return -EINVAL;
		}
	}
	return PX4_OK;
}

//入口函数
extern "C" __EXPORT int test_main(int argc,char *argv[]){
	return TestModule::main(argc,argv);
}

