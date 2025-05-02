
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

extern "C" CAMetalLayer* TMP_Cocoa_CreateMetalLayer(NSWindow* ns_window) {
  CAMetalLayer* metal_layer = nullptr;
  [ns_window.contentView setWantsLayer : YES];
  metal_layer = [CAMetalLayer layer];
  [ns_window.contentView setLayer : metal_layer];
  return metal_layer;
}