#include "QPULib.h"
#include "Support/Settings.h"

using namespace QPULib;

QPULib::Settings settings;


// Define function that runs on the GPU.
void hello(Ptr<Int> p)
{
  *p = 1;
  //Int val = *p * 2;
  //*p = val;
}


int main(int argc, const char *argv[]) {
	auto ret = settings.init(argc, argv);
	if (ret != CmdParameters::ALL_IS_WELL) return ret;

  // Construct kernel
  auto k = compile(hello);
  k.setNumQPUs(8);

  // Allocate and initialise array shared between ARM and GPU
  SharedArray<int> array(8*16);
  for (int i = 0; i < array.size(); i++)
    array[i] = 100;

  // Invoke the kernel
	k.load(&array);
	settings.process(k);  

	// Display the result
  for (int i = 0; i < array.size(); i++) {
    printf("%i: %i\n", i, array[i]);
  }
  return 0;
}
