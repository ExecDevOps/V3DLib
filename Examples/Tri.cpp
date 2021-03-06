#include "V3DLib.h"
#include <CmdParameters.h>
#include "Support/Settings.h"

using namespace V3DLib;

std::vector<const char *> const kernels = { "integer", "float" };  // First is default

CmdParameters params = {
	"Tri - Calculate triangular numbers\n",
  {{
    "Kernel",
    "-k=",
		kernels,
    "Select the kernel to use"
	}}
};


struct TriSettings : public Settings {
	int kernel;

	TriSettings() : Settings(&params, true) {}

	void init_params() override {
		kernel = params.parameters()[0]->get_int_value();
	}

} settings;


///////////////////////////////////////////
// Kernels
///////////////////////////////////////////

void tri_int(Ptr<Int> p) {
  Int n = *p;
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum + n;
      n = n - 1;
    End
  End
  *p = sum;
}


void tri_float(Ptr<Float> p) {
  Int n = toInt(*p);
  Int sum = 0;
  While (any(n > 0))
    Where (n > 0)
      sum = sum + n;
      n = n - 1;
    End
  End
  *p = toFloat(sum);
}


///////////////////////////////////////////
// Local functions
///////////////////////////////////////////

void run_int() {
	printf("Running integer kernel.\n");

  // Construct kernel
  auto k = compile(tri_int);
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(settings.num_qpus*16);
  for (int i = 0; i < (int) array.size(); i++)
    array[i] = i;

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

  // Display the result
  for (int i = 0; i < (int) array.size(); i++)
    printf("%i: %i\n", i, array[i]);
}


void run_float() {
	printf("Running float kernel.\n");

  // Construct kernel
  auto k = compile(tri_float);
  k.setNumQPUs(settings.num_qpus);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<float> array(settings.num_qpus*16);
  for (int i = 0; i < (int) array.size(); i++)
    array[i] = (float) i;

  // Invoke the kernel
  k.load(&array);
  settings.process(k);

  // Display the result
  for (int i = 0; i < (int) array.size(); i++)
    printf("%i: %f\n", i, array[i]);
}


///////////////////////////////////////////
// Main
///////////////////////////////////////////

int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

	switch (settings.kernel) {
		case 0: run_int(); break;
		case 1: run_float(); break;
	}
  
  return 0;
}
