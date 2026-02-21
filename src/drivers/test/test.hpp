/*
重构代码成PX4风格
持续更新：
给函数加输入校验与错误日志
在模块里加入“超时后切状态”的逻辑
加一个“统计计数器”（丢包/CRC错误次数）
头文件
*/
#pragma once
#include <px4_platform_common/module.h>
#include <px4_platform_common/log.h>
#include <px4_platform_common/time.h>
#include <px4_platform_common/getopt.h>
#include <stdint.h>
#include <drivers/drv_hrt.h>
#include <perf/perf_counter.h>

class TestModule final : public ModuleBase<TestModule>{
public:
	TestModule();
	~TestModule() override;

	//modulebase required
	static int task_spawn(int argc,char *argv[]);
	static TestModule *instantiate(int argc,char *argv[]);
	static int custom_command(int argc,char *argv[]);
	static int print_usage(const char *reason = nullptr);

	int print_status() override;
	void run() override;

private:
	enum class State : uint8_t{
		IDLE = 0,
		WORK
	};

	//输入解析/校验
	static int parse_u32(const char *s,uint32_t &out,uint32_t minv,uint32_t maxv,const char *name);
	static int parse_args(int argc,char *argv[],uint32_t &period_ms,uint32_t &max_count);

	//初始化
	State _state{State::IDLE};
	uint32_t _counter{0};
	hrt_abstime _state_entered_us{0};
	uint32_t _drop_count{0};			//因为在 PX4 里 perf_count() 本身不返回计数值
							//所以只能自己维护一个uint32_t镜像来计数

	//参数设置
	uint32_t _period_us{200000};
	uint32_t _max_count{20};

	//PX4风格统计计数器
	perf_counter_t _drop_perf{nullptr};		//丢包/超时丢弃次数
	perf_counter_t _crc_err_perf{nullptr};		//CRC错误次数
};


