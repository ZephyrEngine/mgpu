
#include <mgpu/mgpu.h>

#include "backend/device.hpp"

extern "C" {

void mgpuDeviceDestroy(MGPUDevice device) {
  delete (mgpu::DeviceBase*)device;
}

}  // extern "C"
