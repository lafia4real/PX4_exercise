#include "test.h"
#include <px4_platform_common/log.h>
#include <px4_platform_common/module.h>
// #include <px4_platform_common/time.h>
#include <errno.h>

class TestModule : public ModuleBase<TestModule>{
public:
	TestModule() = default;
	~TestModule() override = default;

	// static int print_usage(const char *reason = nullptr);
	// static int custom_command(int argc,char *argv[]);

	//必须让modulebase能spawn出任务
	static int task_spawn(int argc,char *argv[]){
		_task_id = px4_task_spawn_cmd(
			"test",
			SCHED_DEFAULT,
			SCHED_PRIORITY_DEFAULT,
			2000,
			(px4_main_t)&run_trampoline,
			(char *const *)argv
		);

		if(_task_id < 0){
			_task_id = -1;
			PX4_ERR("task start failed (%d)!",errno);
			return -errno;
		}

		return 0;
	}

	//必须：实例化对象（可解析参数，这里先不解析）
	static TestModule *instantiate(int argc,char *argv[]){
		(void)argc;
		(void)argv;
		return new TestModule();
	}

	static int custom_command(int argc, char *argv[])
	{
		(void)argc;
		(void)argv;
		return print_usage("unknown command");
	}

	static int print_usage(const char *reason = nullptr)
	{
		if (reason) {
			PX4_WARN("%s", reason);
		}
		PX4_INFO("usage: test {start|stop|status}");
		return 0;
	}

	void run() override{
		PX4_INFO("run start!!");

		while(!should_exit()){
			switch (_state){
				case State::IDLE:
					PX4_INFO("[IDLE] wait...switch to WORK");
					_state = State::WORK;
					_counter = 0;
					break;

				case State::WORK:
					PX4_INFO("[WORK] counter=%u",(unsigned)_counter);
					_counter++;

					if(_counter >= 20){
						PX4_INFO("[WORK]done,back to IDLE");
						_state = State::IDLE;
					}
					break;
			}

			px4_usleep(200000);
		}
		PX4_INFO("run() exit!!");
	}

private:
	enum class State : uint8_t{
		IDLE = 0,
		WORK
	};

	State _state{State::IDLE};
	uint32_t _counter{0};

};

extern "C" __EXPORT int test_main(int argc,char *argv[]){
	// PX4_INFO("TEST SUCCESS!!argc = %d",argc);
	// return 0;

	return TestModule::main(argc,argv);
}
