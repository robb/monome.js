#include <v8.h>
#include <node.h>

#include <monome.h>
#include <stdarg.h>

namespace monome {

using namespace v8;

class Monome: node::ObjectWrap
{
private:
  monome_t *device;

  static const char* ToCString(const String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
  }

public:

  static Persistent<FunctionTemplate> ft;

  static void Init(Handle<Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(Monome::New);

    ft = Persistent<FunctionTemplate>::New(t);
    ft->SetClassName(String::NewSymbol("Monome"));
    ft->InstanceTemplate()->SetInternalFieldCount(1);

    // Methods
    NODE_SET_PROTOTYPE_METHOD(ft, "all", SetAll);
    NODE_SET_PROTOTYPE_METHOD(ft, "set", SetLED);

    Local<ObjectTemplate> it = ft->InstanceTemplate();

    // Accessors
    it->SetAccessor(String::New("rows"),       GetRows);
    it->SetAccessor(String::New("columns"),    GetColumns);
    it->SetAccessor(String::New("rotation"),   GetRotation, SetRotation);
    it->SetAccessor(String::New("type"),       GetType);
    it->SetAccessor(String::New("devicePath"), GetDevPath);

    target->Set(String::NewSymbol("Monome"),
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

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;

    if (!args.Length())
      return ThrowException(String::New("Missing parameters"));

    String::Utf8Value device(args[0]);

    try {
      Monome* m = new Monome(ToCString(device));
      m->Wrap(args.This());
      return args.This();
    } catch (const char *message) {
      return ThrowException(String::New(message));
    }
  }

  // Accessors

  // this.rows
  static Handle<Value> GetRows(Local<String> property,
                                       const AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    return Integer::New(monome_get_rows(monome->device));
  }

  // this.columns
  static Handle<Value> GetColumns(Local<String> property,
                                          const AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    return Integer::New(monome_get_cols(monome->device));
  }

  // this.rotation
  static Handle<Value> GetRotation(Local<String> property,
                                           const AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());

    int rotation = monome_get_rotation(monome->device) * 90;

    return Integer::New(rotation);
  }

  // this.rotation=
  static void SetRotation(Local<String> property,
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
  static Handle<Value> GetType(Local<String> property,
                                       const AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());
    
    return String::New(monome_get_friendly_name(monome->device));
  }
  
  // this.devicePath
  static Handle <Value> GetDevPath(Local<String> property,
                                           const AccessorInfo& info) {
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(info.Holder());
    
    return String::New(monome_get_devpath(monome->device));
  }

  // Methods

  static Handle<Value> SetAll(const Arguments& args)
  {
    if (!args.Length())
      return ThrowException(String::New("Missing argument state"));
    
    Monome* monome = node::ObjectWrap::Unwrap<Monome>(args.This());

    monome_led_all(monome->device, args[0]->Int32Value());

    return Undefined();
  }

  static Handle<Value> SetLED(const Arguments& args)
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
};

Persistent<FunctionTemplate> Monome::ft;

extern "C" {
  static void init (Handle<Object> target)
  {
    Monome::Init(target);
  }

  NODE_MODULE(monome, init);
}

} // Namespace monome