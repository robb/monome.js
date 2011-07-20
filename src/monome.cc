#include <v8.h>
#include <node.h>

#include <monome.h>
#include <stdarg.h>

namespace monome {

class Monome: node::ObjectWrap
{
private:
  monome_t *device;

  static const char* ToCString(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
  }

public:

  static v8::Persistent<v8::FunctionTemplate> ft;

  static void Init(v8::Handle<v8::Object> target)
  {
    v8::HandleScope scope;

    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(Monome::New);

    ft = v8::Persistent<v8::FunctionTemplate>::New(t);
    ft->SetClassName(v8::String::NewSymbol("Monome"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);

    // Methods
    NODE_SET_PROTOTYPE_METHOD(ft, "all", SetAll);
    NODE_SET_PROTOTYPE_METHOD(ft, "set", SetLED);

    v8::Local<v8::ObjectTemplate> it = ft->InstanceTemplate();

    // Accessors
    it->SetAccessor(v8::String::New("rows"),       GetRows);
    it->SetAccessor(v8::String::New("columns"),    GetColumns);
    it->SetAccessor(v8::String::New("rotation"),   GetRotation, SetRotation);
    it->SetAccessor(v8::String::New("type"),       GetType);
    it->SetAccessor(v8::String::New("devicePath"), GetDevPath);

    target->Set(v8::String::NewSymbol("Monome"),
                ft->GetFunction());
  }

  Monome(const char *dev, ...)
  {
    va_list arguments;

    va_start(arguments, dev);
    device = monome_open(dev);
    va_end(arguments);

    if (!device) {
      throw "Could not open device";
    }

    monome_led_all(device, 0);
  }

  ~Monome()
  {
    monome_close(device);
  }

  static v8::Handle<v8::Value> New(const v8::Arguments& args)
  {
    v8::HandleScope scope;

    if (!args.Length())
      return v8::ThrowException(v8::String::New("Missing parameters"));

    v8::String::Utf8Value device(args[0]);

    try {
      Monome* m = new Monome(ToCString(device));
      m->Wrap(args.This());
      return args.This();
    } catch (const char *message) {
      return v8::ThrowException(v8::String::New(message));
    }
  }

  // Accessors

  // this.rows
  static v8::Handle<v8::Value> GetRows(v8::Local<v8::String> property,
                                       const v8::AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    return v8::Integer::New(monome_get_rows(monome->device));
  }

  // this.columns
  static v8::Handle<v8::Value> GetColumns(v8::Local<v8::String> property,
                                          const v8::AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    return v8::Integer::New(monome_get_cols(monome->device));
  }

  // this.rotation
  static v8::Handle<v8::Value> GetRotation(v8::Local<v8::String> property,
                                           const v8::AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    int rotation = monome_get_rotation(monome->device) * 90;

    return v8::Integer::New(rotation);
  }

  // this.rotation=
  static void SetRotation(v8::Local<v8::String> property,
                          v8::Local<v8::Value> value,
                          const v8::AccessorInfo& info) {
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
  static v8::Handle<v8::Value> GetType(v8::Local<v8::String> property,
                                       const v8::AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());
    
    return v8::String::New(monome_get_friendly_name(monome->device));
  }
  
  // this.devicePath
  static v8::Handle <v8::Value> GetDevPath(v8::Local<v8::String> property,
                                           const v8::AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());
    
    return v8::String::New(monome_get_devpath(monome->device));
  }

  // Methods

  static v8::Handle<v8::Value> SetAll(const v8::Arguments& args)
  {
    if (!args.Length())
      return v8::ThrowException(v8::String::New("Missing argument state"));
    
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(args.This());

    monome_led_all(monome->device, args[0]->Int32Value());

    return v8::Undefined();
  }

  static v8::Handle<v8::Value> SetLED(const v8::Arguments& args)
  {
    if (args.Length() < 3)
      return v8::ThrowException(v8::String::New("Invalid args"));

    Monome* monome = node::ObjectWrap::Unwrap<Monome>(args.This());

    unsigned int x, y, on;
    x  = args[0]->Uint32Value();
    y  = args[1]->Uint32Value();
    on = args[2]->Uint32Value();

    monome_led_set(monome->device, x, y, on);
    
    return v8::Undefined();
  }
};

v8::Persistent<v8::FunctionTemplate> Monome::ft;

extern "C" {
  static void init (v8::Handle<v8::Object> target)
  {
  Monome::Init(target);
  }

  NODE_MODULE(monome, init);
}

} // Namespace monome