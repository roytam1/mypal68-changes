/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// mostly derived from the Allegro source code at:
// http://alleg.svn.sourceforge.net/viewvc/alleg/allegro/branches/4.9/src/macosx/hidjoy.m?revision=13760&view=markup

#include "mozilla/dom/GamepadPlatformService.h"
#include "mozilla/ArrayUtils.h"
#include "nsITimer.h"
#include "nsThreadUtils.h"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDBase.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>

#include <stdio.h>
#include <vector>

namespace {

using namespace mozilla;
using namespace mozilla::dom;
using std::vector;
class DarwinGamepadService;

DarwinGamepadService* gService = nullptr;

struct Button {
  int id;
  bool analog;
  IOHIDElementRef element;
  CFIndex min;
  CFIndex max;

  Button(int aId, IOHIDElementRef aElement, CFIndex aMin, CFIndex aMax)
      : id(aId),
        analog((aMax - aMin) > 1),
        element(aElement),
        min(aMin),
        max(aMax) {}
};

struct Axis {
  int id;
  IOHIDElementRef element;
  uint32_t usagePage;
  uint32_t usage;
  CFIndex min;
  CFIndex max;
};

typedef bool dpad_buttons[4];

// These values can be found in the USB HID Usage Tables:
// http://www.usb.org/developers/hidpage
const unsigned kDesktopUsagePage = 0x01;
const unsigned kSimUsagePage = 0x02;
const unsigned kAcceleratorUsage = 0xC4;
const unsigned kBrakeUsage = 0xC5;
const unsigned kJoystickUsage = 0x04;
const unsigned kGamepadUsage = 0x05;
const unsigned kAxisUsageMin = 0x30;
const unsigned kAxisUsageMax = 0x35;
const unsigned kDpadUsage = 0x39;
const unsigned kButtonUsagePage = 0x09;
const unsigned kConsumerPage = 0x0C;
const unsigned kHomeUsage = 0x223;
const unsigned kBackUsage = 0x224;

// We poll it periodically,
// 50ms is arbitrarily chosen.
const uint32_t kDarwinGamepadPollInterval = 50;

class Gamepad {
 private:
  IOHIDDeviceRef mDevice;
  nsTArray<Button> buttons;
  nsTArray<Axis> axes;
  IOHIDElementRef mDpad;
  dpad_buttons mDpadState;

 public:
  Gamepad() : mDevice(nullptr), mDpad(nullptr), mSuperIndex(-1) {}
  bool operator==(IOHIDDeviceRef device) const { return mDevice == device; }
  bool empty() const { return mDevice == nullptr; }
  void clear() {
    mDevice = nullptr;
    buttons.Clear();
    axes.Clear();
    mDpad = nullptr;
    mSuperIndex = -1;
  }
  void init(IOHIDDeviceRef device);
  size_t numButtons() { return buttons.Length() + (mDpad ? 4 : 0); }
  size_t numAxes() { return axes.Length(); }

  // Index given by our superclass.
  uint32_t mSuperIndex;

  bool isDpad(IOHIDElementRef element) const { return element == mDpad; }

  const dpad_buttons& getDpadState() const { return mDpadState; }

  void setDpadState(const dpad_buttons& dpadState) {
    for (unsigned i = 0; i < ArrayLength(mDpadState); i++) {
      mDpadState[i] = dpadState[i];
    }
  }

  const Button* lookupButton(IOHIDElementRef element) const {
    for (unsigned i = 0; i < buttons.Length(); i++) {
      if (buttons[i].element == element) return &buttons[i];
    }
    return nullptr;
  }

  const Axis* lookupAxis(IOHIDElementRef element) const {
    for (unsigned i = 0; i < axes.Length(); i++) {
      if (axes[i].element == element) return &axes[i];
    }
    return nullptr;
  }
};

class AxisComparator {
 public:
  bool Equals(const Axis& a1, const Axis& a2) const {
    return a1.usagePage == a2.usagePage && a1.usage == a2.usage;
  }
  bool LessThan(const Axis& a1, const Axis& a2) const {
    if (a1.usagePage == a2.usagePage) {
      return a1.usage < a2.usage;
    }
    return a1.usagePage < a2.usagePage;
  }
};

void Gamepad::init(IOHIDDeviceRef device) {
  clear();
  mDevice = device;

  CFArrayRef elements =
      IOHIDDeviceCopyMatchingElements(device, nullptr, kIOHIDOptionsTypeNone);
  CFIndex n = CFArrayGetCount(elements);
  for (CFIndex i = 0; i < n; i++) {
    IOHIDElementRef element =
        (IOHIDElementRef)CFArrayGetValueAtIndex(elements, i);
    uint32_t usagePage = IOHIDElementGetUsagePage(element);
    uint32_t usage = IOHIDElementGetUsage(element);

    if (usagePage == kDesktopUsagePage && usage >= kAxisUsageMin &&
        usage <= kAxisUsageMax) {
      Axis axis = {int(axes.Length()),
                   element,
                   usagePage,
                   usage,
                   IOHIDElementGetLogicalMin(element),
                   IOHIDElementGetLogicalMax(element)};
      axes.AppendElement(axis);
    } else if (usagePage == kDesktopUsagePage && usage == kDpadUsage &&
               // Don't know how to handle d-pads that return weird values.
               IOHIDElementGetLogicalMax(element) -
                       IOHIDElementGetLogicalMin(element) ==
                   7) {
      mDpad = element;
    } else if ((usagePage == kSimUsagePage &&
                (usage == kAcceleratorUsage || usage == kBrakeUsage)) ||
               (usagePage == kButtonUsagePage) ||
               (usagePage == kConsumerPage &&
                (usage == kHomeUsage || usage == kBackUsage))) {
      Button button(int(buttons.Length()), element,
                    IOHIDElementGetLogicalMin(element),
                    IOHIDElementGetLogicalMax(element));
      buttons.AppendElement(button);
    } else {
      // TODO: handle other usage pages
    }
  }

  AxisComparator comparator;
  axes.Sort(comparator);
  for (unsigned i = 0; i < axes.Length(); i++) {
    axes[i].id = i;
  }
}

// This service is created and destroyed in Background thread while
// operates in a seperate thread(We call it Monitor thread here).
class DarwinGamepadService {
 private:
  IOHIDManagerRef mManager;
  vector<Gamepad> mGamepads;

  nsCOMPtr<nsIThread> mMonitorThread;
  nsCOMPtr<nsIThread> mBackgroundThread;
  nsCOMPtr<nsITimer> mPollingTimer;
  bool mIsRunning;

  static void DeviceAddedCallback(void* data, IOReturn result, void* sender,
                                  IOHIDDeviceRef device);
  static void DeviceRemovedCallback(void* data, IOReturn result, void* sender,
                                    IOHIDDeviceRef device);
  static void InputValueChangedCallback(void* data, IOReturn result,
                                        void* sender, IOHIDValueRef newValue);
  static void EventLoopOnceCallback(nsITimer* aTimer, void* aClosure);

  void DeviceAdded(IOHIDDeviceRef device);
  void DeviceRemoved(IOHIDDeviceRef device);
  void InputValueChanged(IOHIDValueRef value);
  void StartupInternal();
  void RunEventLoopOnce();

 public:
  DarwinGamepadService();
  ~DarwinGamepadService();
  void Startup();
  void Shutdown();
  friend class DarwinGamepadServiceStartupRunnable;
  friend class DarwinGamepadServiceShutdownRunnable;
};

class DarwinGamepadServiceStartupRunnable final : public Runnable {
 private:
  ~DarwinGamepadServiceStartupRunnable() {}
  // This Runnable schedules startup of DarwinGamepadService
  // in a new thread, pointer to DarwinGamepadService is only
  // used by this Runnable within its thread.
  DarwinGamepadService MOZ_NON_OWNING_REF* mService;

 public:
  explicit DarwinGamepadServiceStartupRunnable(DarwinGamepadService* service)
      : Runnable("DarwinGamepadServiceStartupRunnable"), mService(service) {}
  NS_IMETHOD Run() override {
    MOZ_ASSERT(mService);
    mService->StartupInternal();
    return NS_OK;
  }
};

class DarwinGamepadServiceShutdownRunnable final : public Runnable {
 private:
  ~DarwinGamepadServiceShutdownRunnable() {}

 public:
  // This Runnable schedules shutdown of DarwinGamepadService
  // in background thread.
  explicit DarwinGamepadServiceShutdownRunnable()
      : Runnable("DarwinGamepadServiceStartupRunnable") {}
  NS_IMETHOD Run() override {
    MOZ_ASSERT(gService);
    MOZ_ASSERT(NS_GetCurrentThread() == gService->mBackgroundThread);

    IOHIDManagerRef manager = (IOHIDManagerRef)gService->mManager;

    if (manager) {
      IOHIDManagerClose(manager, 0);
      CFRelease(manager);
      gService->mManager = nullptr;
    }
    gService->mMonitorThread->Shutdown();
    delete gService;
    gService = nullptr;
    return NS_OK;
  }
};

void DarwinGamepadService::DeviceAdded(IOHIDDeviceRef device) {
  RefPtr<GamepadPlatformService> service =
      GamepadPlatformService::GetParentService();
  if (!service) {
    return;
  }

  size_t slot = size_t(-1);
  for (size_t i = 0; i < mGamepads.size(); i++) {
    if (mGamepads[i] == device) return;
    if (slot == size_t(-1) && mGamepads[i].empty()) slot = i;
  }

  if (slot == size_t(-1)) {
    slot = mGamepads.size();
    mGamepads.push_back(Gamepad());
  }
  mGamepads[slot].init(device);

  // Gather some identifying information
  CFNumberRef vendorIdRef =
      (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey));
  CFNumberRef productIdRef =
      (CFNumberRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey));
  CFStringRef productRef =
      (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
  int vendorId, productId;
  CFNumberGetValue(vendorIdRef, kCFNumberIntType, &vendorId);
  CFNumberGetValue(productIdRef, kCFNumberIntType, &productId);
  char product_name[128];
  CFStringGetCString(productRef, product_name, sizeof(product_name),
                     kCFStringEncodingASCII);
  char buffer[256];
  sprintf(buffer, "%x-%x-%s", vendorId, productId, product_name);
  uint32_t index = service->AddGamepad(
      buffer, mozilla::dom::GamepadMappingType::_empty,
      mozilla::dom::GamepadHand::_empty, (int)mGamepads[slot].numButtons(),
      (int)mGamepads[slot].numAxes(),
      0);  // TODO: Bug 680289, implement gamepad haptics for cocoa
  mGamepads[slot].mSuperIndex = index;
}

void DarwinGamepadService::DeviceRemoved(IOHIDDeviceRef device) {
  RefPtr<GamepadPlatformService> service =
      GamepadPlatformService::GetParentService();
  if (!service) {
    return;
  }
  for (size_t i = 0; i < mGamepads.size(); i++) {
    if (mGamepads[i] == device) {
      service->RemoveGamepad(mGamepads[i].mSuperIndex);
      mGamepads[i].clear();
      return;
    }
  }
}

/*
 * Given a value from a d-pad (POV hat in USB HID terminology),
 * represent it as 4 buttons, one for each cardinal direction.
 */
static void UnpackDpad(int dpad_value, int min, int max,
                       dpad_buttons& buttons) {
  const unsigned kUp = 0;
  const unsigned kDown = 1;
  const unsigned kLeft = 2;
  const unsigned kRight = 3;

  // Different controllers have different ways of representing
  // "nothing is pressed", but they're all outside the range of values.
  if (dpad_value < min || dpad_value > max) {
    // Nothing is pressed.
    return;
  }

  // Normalize value to start at 0.
  int value = dpad_value - min;

  // Value will be in the range 0-7. The value represents the
  // position of the d-pad around a circle, with 0 being straight up,
  // 2 being right, 4 being straight down, and 6 being left.
  if (value < 2 || value > 6) {
    buttons[kUp] = true;
  }
  if (value > 2 && value < 6) {
    buttons[kDown] = true;
  }
  if (value > 4) {
    buttons[kLeft] = true;
  }
  if (value > 0 && value < 4) {
    buttons[kRight] = true;
  }
}

void DarwinGamepadService::InputValueChanged(IOHIDValueRef value) {
  RefPtr<GamepadPlatformService> service =
      GamepadPlatformService::GetParentService();
  if (!service) {
    return;
  }

  uint32_t value_length = IOHIDValueGetLength(value);
  if (value_length > 4) {
    // Workaround for bizarre issue with PS3 controllers that try to return
    // massive (30+ byte) values and crash IOHIDValueGetIntegerValue
    return;
  }
  IOHIDElementRef element = IOHIDValueGetElement(value);
  IOHIDDeviceRef device = IOHIDElementGetDevice(element);

  for (unsigned i = 0; i < mGamepads.size(); i++) {
    Gamepad& gamepad = mGamepads[i];
    if (gamepad == device) {
      if (gamepad.isDpad(element)) {
        const dpad_buttons& oldState = gamepad.getDpadState();
        dpad_buttons newState = {false, false, false, false};
        UnpackDpad(IOHIDValueGetIntegerValue(value),
                   IOHIDElementGetLogicalMin(element),
                   IOHIDElementGetLogicalMax(element), newState);
        const int numButtons = gamepad.numButtons();
        for (unsigned b = 0; b < ArrayLength(newState); b++) {
          if (newState[b] != oldState[b]) {
            service->NewButtonEvent(gamepad.mSuperIndex, numButtons - 4 + b,
                                    newState[b]);
          }
        }
        gamepad.setDpadState(newState);
      } else if (const Axis* axis = gamepad.lookupAxis(element)) {
        double d = IOHIDValueGetIntegerValue(value);
        double v =
            2.0f * (d - axis->min) / (double)(axis->max - axis->min) - 1.0f;
        service->NewAxisMoveEvent(gamepad.mSuperIndex, axis->id, v);
      } else if (const Button* button = gamepad.lookupButton(element)) {
        int iv = IOHIDValueGetIntegerValue(value);
        bool pressed = iv != 0;
        double v = 0;
        if (button->analog) {
          double dv = iv;
          v = (dv - button->min) / (double)(button->max - button->min);
        } else {
          v = pressed ? 1.0 : 0.0;
        }
        service->NewButtonEvent(gamepad.mSuperIndex, button->id, pressed, v);
      }
      return;
    }
  }
}

void DarwinGamepadService::DeviceAddedCallback(void* data, IOReturn result,
                                               void* sender,
                                               IOHIDDeviceRef device) {
  DarwinGamepadService* service = (DarwinGamepadService*)data;
  service->DeviceAdded(device);
}

void DarwinGamepadService::DeviceRemovedCallback(void* data, IOReturn result,
                                                 void* sender,
                                                 IOHIDDeviceRef device) {
  DarwinGamepadService* service = (DarwinGamepadService*)data;
  service->DeviceRemoved(device);
}

void DarwinGamepadService::InputValueChangedCallback(void* data,
                                                     IOReturn result,
                                                     void* sender,
                                                     IOHIDValueRef newValue) {
  DarwinGamepadService* service = (DarwinGamepadService*)data;
  service->InputValueChanged(newValue);
}

void DarwinGamepadService::EventLoopOnceCallback(nsITimer* aTimer,
                                                 void* aClosure) {
  DarwinGamepadService* service = static_cast<DarwinGamepadService*>(aClosure);
  service->RunEventLoopOnce();
}

static CFMutableDictionaryRef MatchingDictionary(UInt32 inUsagePage,
                                                 UInt32 inUsage) {
  CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
      kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
      &kCFTypeDictionaryValueCallBacks);
  if (!dict) return nullptr;
  CFNumberRef number =
      CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsagePage);
  if (!number) {
    CFRelease(dict);
    return nullptr;
  }
  CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), number);
  CFRelease(number);

  number = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &inUsage);
  if (!number) {
    CFRelease(dict);
    return nullptr;
  }
  CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), number);
  CFRelease(number);

  return dict;
}

DarwinGamepadService::DarwinGamepadService()
    : mManager(nullptr), mIsRunning(false) {}

DarwinGamepadService::~DarwinGamepadService() {
  if (mManager != nullptr) CFRelease(mManager);
  mMonitorThread = nullptr;
  mBackgroundThread = nullptr;
  if (mPollingTimer) {
    mPollingTimer->Cancel();
    mPollingTimer = nullptr;
  }
}

void DarwinGamepadService::RunEventLoopOnce() {
  MOZ_ASSERT(NS_GetCurrentThread() == mMonitorThread);
  CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.0, true);

  // This timer must be created in monitor thread
  if (!mPollingTimer) {
    mPollingTimer = do_CreateInstance("@mozilla.org/timer;1");
  }
  mPollingTimer->Cancel();
  if (mIsRunning) {
    mPollingTimer->InitWithNamedFuncCallback(
        EventLoopOnceCallback, this, kDarwinGamepadPollInterval,
        nsITimer::TYPE_ONE_SHOT, "EventLoopOnceCallback");
  } else {
    // We schedule a task shutdown and cleaning up resources to Background
    // thread here to make sure no runloop is running to prevent potential race
    // condition.
    RefPtr<Runnable> shutdownTask = new DarwinGamepadServiceShutdownRunnable();
    mBackgroundThread->Dispatch(shutdownTask.forget(), NS_DISPATCH_NORMAL);
  }
}

void DarwinGamepadService::StartupInternal() {
  if (mManager != nullptr) return;

  IOHIDManagerRef manager =
      IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

  CFMutableDictionaryRef criteria_arr[2];
  criteria_arr[0] = MatchingDictionary(kDesktopUsagePage, kJoystickUsage);
  if (!criteria_arr[0]) {
    CFRelease(manager);
    return;
  }

  criteria_arr[1] = MatchingDictionary(kDesktopUsagePage, kGamepadUsage);
  if (!criteria_arr[1]) {
    CFRelease(criteria_arr[0]);
    CFRelease(manager);
    return;
  }

  CFArrayRef criteria = CFArrayCreate(kCFAllocatorDefault,
                                      (const void**)criteria_arr, 2, nullptr);
  if (!criteria) {
    CFRelease(criteria_arr[1]);
    CFRelease(criteria_arr[0]);
    CFRelease(manager);
    return;
  }

  IOHIDManagerSetDeviceMatchingMultiple(manager, criteria);
  CFRelease(criteria);
  CFRelease(criteria_arr[1]);
  CFRelease(criteria_arr[0]);

  IOHIDManagerRegisterDeviceMatchingCallback(manager, DeviceAddedCallback,
                                             this);
  IOHIDManagerRegisterDeviceRemovalCallback(manager, DeviceRemovedCallback,
                                            this);
  IOHIDManagerRegisterInputValueCallback(manager, InputValueChangedCallback,
                                         this);
  IOHIDManagerScheduleWithRunLoop(manager, CFRunLoopGetCurrent(),
                                  kCFRunLoopDefaultMode);
  IOReturn rv = IOHIDManagerOpen(manager, kIOHIDOptionsTypeNone);
  if (rv != kIOReturnSuccess) {
    CFRelease(manager);
    return;
  }

  mManager = manager;

  mIsRunning = true;
  RunEventLoopOnce();
}

void DarwinGamepadService::Startup() {
  mBackgroundThread = NS_GetCurrentThread();
  Unused << NS_NewNamedThread("Gamepad", getter_AddRefs(mMonitorThread),
                              new DarwinGamepadServiceStartupRunnable(this));
}

void DarwinGamepadService::Shutdown() {
  // Flipping this flag will stop the eventloop in Monitor thread
  // and dispatch a task destroying and cleaning up resources in
  // Background thread
  mIsRunning = false;
}

}  // namespace

namespace mozilla {
namespace dom {

void StartGamepadMonitoring() {
  AssertIsOnBackgroundThread();
  if (gService) {
    return;
  }

  gService = new DarwinGamepadService();
  gService->Startup();
}

void StopGamepadMonitoring() {
  AssertIsOnBackgroundThread();
  if (!gService) {
    return;
  }

  // Calling Shutdown() will delete gService as well
  gService->Shutdown();
}

}  // namespace dom
}  // namespace mozilla
