#include "monome_js.h"

namespace monome {

using namespace v8;

Persistent<String> onButtonDown;
Persistent<String> onButtonUp;

const char* Monome::ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void Monome::Init(Handle<Object> target)
{
  HandleScope scope;

  onButtonDown = NODE_PSYMBOL("onbuttondown");
  onButtonUp   = NODE_PSYMBOL("onbuttonup");

  Local<FunctionTemplate> t = FunctionTemplate::New(Monome::New);

  ft = Persistent<FunctionTemplate>::New(t);
  ft->SetClassName(String::NewSymbol("Monome"));
  ft->InstanceTemplate()->SetInternalFieldCount(1);

  // Methods
  NODE_SET_PROTOTYPE_METHOD(ft, "all", SetAll);
  NODE_SET_PROTOTYPE_METHOD(ft, "set", SetLED);
  NODE_SET_PROTOTYPE_METHOD(ft, "start", Start);
  NODE_SET_PROTOTYPE_METHOD(ft, "stop", Stop);

  Local<ObjectTemplate> it = ft->InstanceTemplate();

  // Accessors
  it->SetAccessor(String::New("rows"),       GetRows);
  it->SetAccessor(String::New("columns"),    GetColumns);
  it->SetAccessor(String::New("type"),       GetType);
  it->SetAccessor(String::New("devicePath"), GetDevPath);
  it->SetAccessor(String::New("rotation"),   GetRotation, SetRotation);

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
                         const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  monome_rotate_t rotate;

  switch (value->Int32Value()) {
    case 270:
      rotate = MONOME_ROTATE_270;
      break;

    case 180:
      rotate = MONOME_ROTATE_180;
      break;

    case 90:
      rotate = MONOME_ROTATE_90;
      break;

    case 0:
      rotate = MONOME_ROTATE_0;
      break;

    default:
      // Abort
      // TODO: Find a way to throw a meaningful exception to JavsScript land
      return;
  }

  monome_set_rotation(monome->device, rotate);
}

// this.type
Handle<Value> Monome::GetType(Local<String> property,
                              const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return String::New(monome_get_friendly_name(monome->device));
}

// this.devicePath
Handle <Value> Monome::GetDevPath(Local<String> property,
                                  const AccessorInfo& info) {
  Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

  return String::New(monome_get_devpath(monome->device));
}

// Methods

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

Handle<Value> Monome::Start(const Arguments& args)
{
  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());
  m->Start();

  return args.This();
}

void Monome::Start() {
  if (!ev_is_active(&watcher)) {
    ev_io_start(EV_DEFAULT_ &watcher);
    Ref();

    monome_register_handler(device, MONOME_BUTTON_UP,   EventHandler, this);
    monome_register_handler(device, MONOME_BUTTON_DOWN, EventHandler, this);
  }
}

Handle<Value> Monome::Stop(const Arguments& args)
{
  Monome* m = node::ObjectWrap::Unwrap<Monome>(args.This());
  m->Stop();

  return args.This();
}

void Monome::Stop() {
  if (ev_is_active(&watcher)) {
    monome_unregister_handler(device, MONOME_BUTTON_DOWN);
    monome_unregister_handler(device, MONOME_BUTTON_UP);

    ev_io_stop(EV_DEFAULT_ &watcher);
    Unref();
  }
}

// Event handling
void Monome::EventCallback(EV_P_ ev_io *watcher, int revents)
{
  Monome *m = (Monome *) watcher->data;

  while(monome_event_handle_next(m->device));
}

void Monome::EventHandler(const monome_event_t *event, void *monome)
{
  Persistent<String> symbol;

  switch (event->event_type) {
    case MONOME_BUTTON_UP:
      symbol = onButtonUp;
      break;

    case MONOME_BUTTON_DOWN:
      symbol = onButtonDown;
      break;

    default:
      return;
  }

  Monome *m = (Monome *) monome;

  Local<Value> callback_v = m->handle_->Get(symbol);
  if (!callback_v->IsFunction()) {
       // callback not defined, ignore
       return;
  }
  Local<Function> callback = Local<Function>::Cast(callback_v);

  // Create event argument
  Local<Value> argv[2];
  argv[0] = Local<Value>::New(Integer::New(event->grid.x));
  argv[1] = Local<Value>::New(Integer::New(event->grid.y));

  callback->Call(m->handle_, 2, argv);
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