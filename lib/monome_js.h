#ifndef MONOME_JS_H_
#define MONOME_JS_H_

#include <node.h>
#include <ev.h>
#include <v8.h>

#include <monome.h>
#include <stdarg.h>

#include <assert.h>

namespace monome {

class Monome : node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> target);
    static v8::Persistent<v8::FunctionTemplate> ft;

  protected:
    ev_io    watcher;
    monome_t *device;

    Monome(const char *dev)
    {
      device = monome_open(dev);

      if (!device) {
        throw "Could not open device";
      }

      ev_init(&watcher, EventCallback);
      watcher.data = this;

      int fd = monome_get_fd(device);
      ev_io_set(&watcher, fd, EV_READ);
    }

    ~Monome()
    {
      monome_close(device);
    }

    // Methods
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetAll(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetLED(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetRow(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetColumn(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetRing(const v8::Arguments& args);

    static v8::Handle<v8::Value> Start(const v8::Arguments& args);
    static v8::Handle<v8::Value> Stop(const v8::Arguments& args);

    static v8::Handle<v8::Value> EnableTilt(const v8::Arguments& args);
    static v8::Handle<v8::Value> DisableTilt(const v8::Arguments& args);

    // Accessors
    static v8::Handle<v8::Value> GetRows(v8::Local<v8::String> property,
                                         const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetColumns(v8::Local<v8::String> property,
                                            const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetRotation(v8::Local<v8::String> property,
                                             const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetType(v8::Local<v8::String> property,
                                         const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetDevPath(v8::Local<v8::String> property,
                                            const v8::AccessorInfo& info);

    static void SetRotation(v8::Local<v8::String> property,
                            v8::Local<v8::Value> value,
                            const v8::AccessorInfo& info);

    // Callbacks
    static void EventCallback(EV_P_ ev_io *watcher, int revents);
    static void EventHandler(const monome_event_t *event, void *monome);

    void Start();
    void Stop();

    // Helpers
    static const char * EventType(const monome_event_t *event);
    static v8::Local<v8::Value> ObjectForEvent(const monome_event_t *event);
    static const char* ToCString(const v8::String::Utf8Value& value);

    v8::Local<v8::Function> CallbackForEvent(const monome_event_t *event);
};

} // namespace node
#endif  // MONOME_JS_H_
