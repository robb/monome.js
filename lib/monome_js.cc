#include "monome_js.h"

namespace monome {

using namespace v8;

Persistent<String> onButtonDown;
Persistent<String> onButtonUp;
Persistent<String> onEncoderDown;
Persistent<String> onEncoderUp;
Persistent<String> onEncoderRotate;
Persistent<String> onTilt;

const char* Monome::ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void Monome::Init(Handle<Object> target)
{
  HandleScope scope;

  onButtonDown    = NODE_PSYMBOL("onButtonDown");
  onButtonUp      = NODE_PSYMBOL("onButtonUp");
  onEncoderDown   = NODE_PSYMBOL("onEncoderDown");
  onEncoderUp     = NODE_PSYMBOL("onEncoderUp");
  onEncoderRotate = NODE_PSYMBOL("onEncoderRotate");
  onTilt          = NODE_PSYMBOL("onTilt");

  Local<FunctionTemplate> t = FunctionTemplate::New(Monome::New);

  ft = Persistent<FunctionTemplate>::New(t);
  ft->SetClassName(String::NewSymbol("Monome"));
  ft->InstanceTemplate()->SetInternalFieldCount(1);

  // Methods
  NODE_SET_PROTOTYPE_METHOD(ft, "setAll",    SetAll);
  NODE_SET_PROTOTYPE_METHOD(ft, "setLED",    SetLED);
  NODE_SET_PROTOTYPE_METHOD(ft, "setRow",    SetRow);
  NODE_SET_PROTOTYPE_METHOD(ft, "setColumn", SetColumn);
  NODE_SET_PROTOTYPE_METHOD(ft, "setRing",   SetRing);

  NODE_SET_PROTOTYPE_METHOD(ft, "start", Start);
  NODE_SET_PROTOTYPE_METHOD(ft, "stop", Stop);
  NODE_SET_PROTOTYPE_METHOD(ft, "enableTilt", EnableTilt);
  NODE_SET_PROTOTYPE_METHOD(ft, "disableTilt", DisableTilt);

  Local<ObjectTemplate> it = ft->InstanceTemplate();

  // Accessors
  it->SetAccessor(String::New("rows"),       GetRows);
  it->SetAccessor(String::New("columns"),    GetColumns);
  it->SetAccessor(String::New("rotation"),   GetRotation, SetRotation);

  it->SetAccessor(String::New("type"),       GetType);
  it->SetAccessor(String::New("devicePath"), GetDevPath);

  target->Set(String::NewSymbol("Monome"), ft->GetFunction());
}

Handle<Value> Monome::New(const Arguments& args)
{
  HandleScope scope;

  if (!args.Length())
    return ThrowException(String::New("Missing parameters"));

  String::Utf8Value device(args[0]);

  Monome* m;
  try {
    m = new Monome(ToCString(device));
  } catch (const char *message) {
    return ThrowException(String::New(message));
  }

  monome_led_all(m->device, 0);
  m->Wrap(args.This());

  return args.This();
}

// Accessors

// this.rows
Handle<Value> Monome::GetRows(Local<String> property,
                              const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return Integer::New(monome_get_rows(monome->device));
}

// this.columns
Handle<Value> Monome::GetColumns(Local<String> property,
                                 const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return Integer::New(monome_get_cols(monome->device));
}

// this.rotation
Handle<Value> Monome::GetRotation(Local<String> property,
                                  const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  int rotation = monome_get_rotation(monome->device) * 90;

  return Integer::New(rotation);
}

// this.rotation=
void Monome::SetRotation(Local<String> property,
                         Local<Value> value,
                         const AccessorInfo& info)
{
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  monome_rotate_t rotate = monome_rotate_t(value->Int32Value() / 90 % 4);
  monome_set_rotation(monome->device, rotate);
}

// this.type
Handle<Value> Monome::GetType(Local<String> property,
                              const AccessorInfo& info)
{
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return String::New(monome_get_friendly_name(monome->device));
}

// this.devicePath
Handle <Value> Monome::GetDevPath(Local<String> property,
                                  const AccessorInfo& info)
{
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return String::New(monome_get_devpath(monome->device));
}

// Methods

// LEDs

Handle<Value> Monome::SetAll(const Arguments& args)
{
  if (!args.Length())
    return ThrowException(String::New("Missing argument state"));

  Monome* monome = node::ObjectWrap::Unwrap<Monome>(args.This());

  monome_led_all(monome->device, args[0]->Int32Value());

  return Undefined();
}

Handle<Value> Monome::SetLED(const Arguments& args)
{
  if (args.Length() < 3)
    return ThrowException(String::New("Invalid args"));

  Monome* monome = node::ObjectWrap::Unwrap<Monome>(args.This());

  unsigned int x, y, on;
  x  = args[0]->Uint32Value();
  y  = args[1]->Uint32Value();
  on = args[2]->Uint32Value();

  monome_led_set(monome->device, x, y, on);

  return Undefined();
}

Handle<Value> Monome::SetRow(const Arguments& args)
{
  if (args.Length() < 2)
    return ThrowException(String::New("Invalid args"));

  Monome *monome = node::ObjectWrap::Unwrap<Monome>(args.This());

  uint row = args[0]->Uint32Value();

  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);
    size_t count = array->Length();

    uint8_t byteArray[count];
    for (uint i = 0; i < count; i++) {
      Local<Value> element = array->Get(i);
      byteArray[i] = element->Uint32Value() & 0xFF;
    }

    monome_led_row(monome->device, 0, row, count, byteArray);
  } else {
    uint8_t  byteArray[4];
    uint32_t value = args[1]->Uint32Value();

    byteArray[0] = (value & 0x000000FF);
    byteArray[1] = (value & 0x0000FF00) >> 8;
    byteArray[2] = (value & 0x00FF0000) >> 16;
    byteArray[3] = (value & 0xFF000000) >> 24;

    monome_led_row(monome->device, 0, row, 4, byteArray);
  }

  return args.This();
}

Handle<Value> Monome::SetColumn(const Arguments& args)
{
  if (args.Length() < 2)
    return ThrowException(String::New("Invalid args"));

  Monome *monome = node::ObjectWrap::Unwrap<Monome>(args.This());

  uint column = args[0]->Uint32Value();

  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);
    size_t count = array->Length();

    uint8_t byteArray[count];
    for (uint i = 0; i < count; i++) {
      Local<Value> element = array->Get(i);
      byteArray[i] = element->Uint32Value() & 0xFF;
    }

    monome_led_col(monome->device, column, 0, count, byteArray);
  } else {
    uint8_t  byteArray[4];
    uint32_t value = args[1]->Uint32Value();

    byteArray[0] = (value & 0x000000FF);
    byteArray[1] = (value & 0x0000FF00) >> 8;
    byteArray[2] = (value & 0x00FF0000) >> 16;
    byteArray[3] = (value & 0xFF000000) >> 24;

    monome_led_col(monome->device, column, 0, 4, byteArray);
  }

  return args.This();
}

Handle<Value> Monome::SetRing(const Arguments& args)
{
  if (args.Length() < 2 || args.Length() > 4)
    return ThrowException(String::New("Invalid args"));

  Monome *monome = node::ObjectWrap::Unwrap<Monome>(args.This());

  uint32_t ring = args[0]->Uint32Value();

  // Map
  if (args.Length() == 2 && args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);
    uint8_t levels[64];

    for (uint i = 0; i < 64; i++) {
      Local<Value> element = array->Get(i);
      levels[i] = element->Uint32Value();
    }
    monome_led_ring_map(monome->device, ring, levels);
    return args.This();
  }

  // All
  if (args.Length() == 2) {
    uint8_t level = args[1]->Uint32Value();

    monome_led_ring_all(monome->device, ring, level);
    return args.This();
  }

  // Set
  if (args.Length() == 3) {
    uint8_t led   = args[1]->Uint32Value();
    uint8_t level = args[2]->Uint32Value();

    monome_led_ring_set(monome->device, ring, led, level);
    return args.This();
  }

  // Range
  if (args.Length() == 4) {
    uint8_t start = args[1]->Uint32Value();
    uint8_t end   = args[2]->Uint32Value();
    uint8_t level = args[3]->Uint32Value();

    monome_led_ring_range(monome->device, ring, start, end, level);
    return args.This();
  }

  assert(false);
  return args.This();
}

// Event Handling

Handle<Value> Monome::Start(const Arguments& args)
{
  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());
  m->Start();

  return args.This();
}

void Monome::Start() {
  if (ev_is_active(&watcher))
    return;

  ev_io_start(EV_DEFAULT_ &watcher);  Ref();
  monome_register_handler(device, MONOME_BUTTON_UP,        EventHandler, this);
  monome_register_handler(device, MONOME_BUTTON_DOWN,      EventHandler, this);
  monome_register_handler(device, MONOME_TILT,             EventHandler, this);
  monome_register_handler(device, MONOME_ENCODER_DELTA,    EventHandler, this);
  monome_register_handler(device, MONOME_ENCODER_KEY_DOWN, EventHandler, this);
  monome_register_handler(device, MONOME_ENCODER_KEY_UP,   EventHandler, this);
}

Handle<Value> Monome::Stop(const Arguments& args)
{
  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());
  m->Stop();

  return args.This();
}

void Monome::Stop() {
  if (!ev_is_active(&watcher))
    return;

  monome_unregister_handler(device, MONOME_ENCODER_KEY_UP);
  monome_unregister_handler(device, MONOME_ENCODER_KEY_DOWN);
  monome_unregister_handler(device, MONOME_ENCODER_DELTA);
  monome_unregister_handler(device, MONOME_TILT);
  monome_unregister_handler(device, MONOME_BUTTON_DOWN);
  monome_unregister_handler(device, MONOME_BUTTON_UP);
  ev_io_stop(EV_DEFAULT_ &watcher);
  Unref();
}

void Monome::EventCallback(EV_P_ ev_io *watcher, int revents)
{
  Monome *m = (Monome *) watcher->data;

  while(monome_event_handle_next(m->device));
}

void Monome::EventHandler(const monome_event_t *event, void *monome)
{
  HandleScope scope;
  Monome *m = (Monome *) monome;

  Local<Function> callback = m->CallbackForEvent(event);

  if (callback.IsEmpty())
    return;

  // Create event argument
  Local<Value> argv[1];
  argv[0] = ObjectForEvent(event);

  TryCatch try_catch;
  callback->Call(m->handle_, 1, argv);
  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }
}

// Tilt

Handle<Value> Monome::EnableTilt(const Arguments& args)
{
  if (!args.Length())
    return ThrowException(String::New("Missing parameters"));

  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());

  int sensor = args[0]->Int32Value();
  int result = monome_tilt_enable(m->device, sensor);

  return Integer::New(result);
}

Handle<Value> Monome::DisableTilt(const Arguments& args)
{
  if (!args.Length())
    return ThrowException(String::New("Missing parameters"));

  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());

  int sensor = args[0]->Int32Value();
  int result = monome_tilt_disable(m->device, sensor);

  return Integer::New(result);
}

// Helper

const char * Monome::EventType(const monome_event_t *event)
{
  switch (event->event_type) {
    case MONOME_BUTTON_DOWN:
      return "buttonDown";

    case MONOME_BUTTON_UP:
      return "buttonUp";

    case MONOME_ENCODER_DELTA:
      return "encoderDelta";

    case MONOME_ENCODER_KEY_DOWN:
      return "encoderDown";

    case MONOME_ENCODER_KEY_UP:
      return "encoderUp";

    case MONOME_TILT:
      return "tilt";

    default:
      assert(false);
      return NULL;
  }
}

Local<Value> Monome::ObjectForEvent(const monome_event_t *event)
{
  Local<Object> object = Object::New();

  object->Set(String::New("type"), String::New(EventType(event)));

  switch (event->event_type) {
    case MONOME_BUTTON_DOWN:
    case MONOME_BUTTON_UP:
      object->Set(String::New("x"), Integer::New(event->grid.x));
      object->Set(String::New("y"), Integer::New(event->grid.y));
      break;

    case MONOME_ENCODER_DELTA:
      object->Set(String::New("delta"), Integer::New(event->encoder.delta));
    case MONOME_ENCODER_KEY_UP:
    case MONOME_ENCODER_KEY_DOWN:
      object->Set(String::New("number"), Integer::New(event->encoder.number));
      break;

    case MONOME_TILT:
      object->Set(String::New("x"), Integer::New(event->tilt.x));
      object->Set(String::New("y"), Integer::New(event->tilt.y));
      object->Set(String::New("z"), Integer::New(event->tilt.z));
      break;

    default:;
  }

  return object;
}

Local<Function> Monome::CallbackForEvent(const monome_event_t *event)
{
  Persistent<String> symbol;
  switch (event->event_type) {
    case MONOME_BUTTON_DOWN:
      symbol = onButtonDown;
      break;

    case MONOME_BUTTON_UP:
      symbol = onButtonUp;
      break;

    case MONOME_ENCODER_DELTA:
      symbol = onEncoderRotate;
      break;

    case MONOME_ENCODER_KEY_DOWN:
      symbol = onEncoderDown;
      break;

    case MONOME_ENCODER_KEY_UP:
      symbol = onEncoderUp;
      break;

    case MONOME_TILT:
      symbol = onTilt;
      break;

    default:
      return Local<Function>();
  }

  Local<Value> callback_v = handle_->Get(symbol);
  if (!callback_v->IsFunction()) {
       return Local<Function>();
  }

  return Local<Function>::Cast(callback_v);
}

Persistent<FunctionTemplate> Monome::ft;

extern "C" {
  static void init (Handle<Object> target)
  {
    Monome::Init(target);
  }

  NODE_MODULE(monome, init);
}

} // Namespace monome
