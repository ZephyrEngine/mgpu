
#include <mgpu/mgpu.h>

#include "backend/texture.hpp"
#include "validation/texture.hpp"
#include "validation/texture_view.hpp"

extern "C" {

MGPUResult mgpuTextureCreateView(MGPUTexture texture, const MGPUTextureViewCreateInfo* create_info, MGPUTextureView* texture_view) {
  mgpu::TextureBase* cxx_texture = (mgpu::TextureBase*)texture;

  const MGPUTextureViewType view_type = create_info->type;
  const MGPUTextureFormat view_format = create_info->format;
  const MGPUTextureAspect view_aspect = create_info->aspect;

  // Ensure view format is valid and compatible with the texture format
  MGPU_FORWARD_ERROR(validate_texture_format(view_format));
  MGPU_FORWARD_ERROR(validate_texture_formats_compatible(view_format, cxx_texture->Format()));

  // Ensure view type is valid and supported by the texture.
  MGPU_FORWARD_ERROR(validate_texture_view_type(view_type));
  MGPU_FORWARD_ERROR(validate_texture_supports_view_type(view_type, cxx_texture));

  // Ensure that the texture aspect is valid and supported by the view format.
  MGPU_FORWARD_ERROR(validate_texture_aspect(view_aspect));
  MGPU_FORWARD_ERROR(validate_texture_format_has_aspect(view_format, view_aspect));

  // Ensure that the mip and array layer ranges are supported by the texture and view type.
  MGPU_FORWARD_ERROR(validate_texture_view_mip_range(cxx_texture, view_type, create_info->base_mip, create_info->mip_count));
  MGPU_FORWARD_ERROR(validate_texture_view_array_layer_range(cxx_texture, view_type, create_info->base_array_layer, create_info->array_layer_count));

  mgpu::Result<mgpu::TextureViewBase*> cxx_texture_view_result = cxx_texture->CreateView(*create_info);
  MGPU_FORWARD_ERROR(cxx_texture_view_result.Code());
  *texture_view = (MGPUTextureView)cxx_texture_view_result.Unwrap();
  return MGPU_SUCCESS;
}

}  // extern "C"
