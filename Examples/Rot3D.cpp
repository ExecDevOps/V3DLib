#include <sys/time.h>
#include <unistd.h>  // sleep()
#include <math.h>
#include <QPULib.h>
#include "Support/Settings.h"
#include "Support/debug.h"
#include <CmdParameters.h>
#ifdef QPU_MODE
#include "vc4/PerformanceCounters.h"
#endif  // QPU_MODE

using namespace QPULib;

#ifdef QPU_MODE
using PC = PerformanceCounters;
#endif  // QPU_MODE

// Number of vertices and angle of rotation
const int N = 192000; // 192000
const float THETA = (float) 3.14159;


// ============================================================================
// Command line handling
// ============================================================================

std::vector<const char *> const kernels = { "3", "2", "1", "cpu" };  // First is default


CmdParameters params = {
  "Rot3D\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}, {
    "Num QPU's",
    "-n=",
		POSITIVE_INTEGER,
    "Number of QPU's to use. Must be a value between 1 an 12 inclusive (TODO: not enforced yet)",
		12
	}, {
    "Display Results",
    "-d",
		ParamType::NONE,
    "Show the results of the calculations"
#ifdef QPU_MODE
	}, {
    "Performance Counters",
    "-pc",
		ParamType::NONE,
    "Show the values of the performance counters (vc4 only)"
#endif  // QPU_MODE
  }},
	&Settings::params()
};


struct Rot3DSettings : public Settings {
	int    kernel;
	int    num_qpus;
	bool   show_results;
#ifdef QPU_MODE
	bool   show_perf_counters;
#endif  // QPU_MODE

	int init(int argc, const char *argv[]) {
		set_name(argv[0]);

		CmdParameters &params = ::params;

		auto ret = params.handle_commandline(argc, argv, false);
		if (ret != CmdParameters::ALL_IS_WELL) return ret;

		// Init the parameters in the parent
		Settings::process(&params);

		kernel              = params.parameters()["Kernel"]->get_int_value();
		//kernel_name       = params.parameters()["Kernel"]->get_string_value();
		num_qpus            = params.parameters()["Num QPU's"]->get_int_value();
		show_results        = params.parameters()["Display Results"]->get_bool_value();
#ifdef QPU_MODE
		show_perf_counters  = params.parameters()["Performance Counters"]->get_bool_value();
#endif  // QPU_MODE

		printf("Num QPU's in settings: %d\n", num_qpus);
		return ret;
	}
} settings;


// ============================================================================
// Kernels
// ============================================================================

/**
 * Scalar versionS
 */
void rot3D(int n, float cosTheta, float sinTheta, float* x, float* y)
{
  for (int i = 0; i < n; i++) {
    float xOld = x[i];
    float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  }
}


/**
 * Vector version 1
 */
void rot3D_1(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  For (Int i = 0, i < n, i = i+16)
    Float xOld = x[i];
    Float yOld = y[i];
    x[i] = xOld * cosTheta - yOld * sinTheta;
    y[i] = yOld * cosTheta + xOld * sinTheta;
  End
}


/**
 * Vector version 2
 */
void rot3D_2(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = 16;
  Ptr<Float> p = x + index();
  Ptr<Float> q = y + index();
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
     store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}


/**
 * Vector version 3
 */
void rot3D_3(Int n, Float cosTheta, Float sinTheta, Ptr<Float> x, Ptr<Float> y)
{
  Int inc = numQPUs() << 4;
  Ptr<Float> p = x + index() + (me() << 4);
  Ptr<Float> q = y + index() + (me() << 4);
  gather(p); gather(q);
 
  Float xOld, yOld;
  For (Int i = 0, i < n, i = i+inc)
    gather(p+inc); gather(q+inc); 
    receive(xOld); receive(yOld);
    store(xOld * cosTheta - yOld * sinTheta, p);
    store(yOld * cosTheta + xOld * sinTheta, q);
    p = p+inc; q = q+inc;
  End

  receive(xOld); receive(yOld);
}

using KernelType = decltype(rot3D_3);


// ============================================================================
// Local functions
// ============================================================================

#ifdef QPU_MODE
/**
 * @brief Enable the counters we are interested in
 */
void initPerfCounters() {
	PC::Init list[] = {
		{ 0, PC::QPU_INSTRUCTIONS },
		{ 1, PC::QPU_STALLED_TMU },
		{ 2, PC::L2C_CACHE_HITS },
		{ 3, PC::L2C_CACHE_MISSES },
		{ 4, PC::QPU_INSTRUCTION_CACHE_HITS },
		{ 5, PC::QPU_INSTRUCTION_CACHE_MISSES },
		{ 6, PC::QPU_CACHE_HITS },
		{ 7, PC::QPU_CACHE_MISSES },
		{ 8, PC::QPU_IDLE },
		{ PC::END_MARKER, PC::END_MARKER }
	};

	PC::enable(list);
	PC::clear(PC::enabled());

	//printf("Perf Count mask: %0X\n", PC::enabled());

	// The following will show zeroes for all counters, *except*
	// for QPU_IDLE, because this was running from the clear statement.
	// Perhaps there are more counters like that.
	//std::string output = PC::showEnabled();
	//printf("%s\n", output.c_str());
}
#endif  // QPU_MODE


/**
 * TODO: Consolidate with Mandelbrot
 */
void end_timer(timeval tvStart) {
  timeval tvEnd, tvDiff;
  gettimeofday(&tvEnd, NULL);
  timersub(&tvEnd, &tvStart, &tvDiff);

  printf("Run time: %ld.%06lds\n", tvDiff.tv_sec, tvDiff.tv_usec);
}


void run_qpu_kernel(KernelType &kernel) {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

  auto k = compile(rot3D_3);  // Construct kernel

  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise arrays shared between ARM and GPU
  SharedArray<float> x(N), y(N);
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

  settings.process(k, N, cosf(THETA), sinf(THETA), &x, &y);

	end_timer(tvStart);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


void run_scalar_kernel() {
  timeval tvStart;
  gettimeofday(&tvStart, NULL);

  // Allocate and initialise
  float* x = new float [N];
  float* y = new float [N];
  for (int i = 0; i < N; i++) {
    x[i] = (float) i;
    y[i] = (float) i;
  }

	if (settings.compile_only) {
	  rot3D(N, cosf(THETA), sinf(THETA), x, y);
	}

	end_timer(tvStart);

	if (settings.show_results) {
  	for (int i = 0; i < N; i++)
  		printf("%f %f\n", x[i], y[i]);
	}
}


/**
 * Run a kernel as specified by the passed kernel index
 */
void run_kernel(int kernel_index) {
	switch (kernel_index) {
		case 0: run_qpu_kernel(rot3D_3);  break;	
		case 1: run_qpu_kernel(rot3D_2);  break;	
		case 2: run_qpu_kernel(rot3D_1);  break;	
		case 3: run_scalar_kernel(); break;
	}

	auto name = kernels[kernel_index];

	printf("Ran kernel '%s' with %d QPU's.\n", name, settings.num_qpus);
}


// ============================================================================
// Main
// ============================================================================

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

#ifdef QPU_MODE
	if (settings.show_perf_counters) {
		if (Platform::instance().has_vc4) {  // vc4 only
			initPerfCounters();
		} else {
			printf("WARNING: Performance counters enabled for VC4 only.\n");
		}
	}
#endif

	run_kernel(settings.kernel);

#ifdef QPU_MODE
	if (settings.show_perf_counters) {
		if (Platform::instance().has_vc4) {  // vc4 only
			// Show values current counters
			std::string output = PC::showEnabled();
			printf("%s\n", output.c_str());
		}
	}
#endif

  return 0;
}
