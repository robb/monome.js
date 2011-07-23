#ifndef MONOME_JS_H_
#define MONOME_JS_H_

#include <queue>

#include <node.h>
#include <ev.h>
#include <v8.h>

#include <monome.h>
#include <stdarg.h>
#include <pthread.h>

#include <assert.h>

namespace monome {

class Monome : node::ObjectWrap {
  public:
    static void Init(v8::Handle<v8::Object> target);
    static v8::Persistent<v8::FunctionTemplate> ft;

  protected:
    pthread_t thread;
    ev_async  watcher;

    monome_t  *device;

    std::queue<monome_event_t> eventQueue;
    pthread_mutex_t eventQueueMutex;

    Monome(const char *dev)
    {
      device = monome_open(dev);

      if (!device) {
        throw "Could not open device";
      }

      ev_init(&watcher, EventCallback);
      watcher.data = this;

      ev_set_priority(&watcher, EV_MAXPRI);

      eventQueue = std::queue<monome_event_t>();
      pthread_mutex_init(&eventQueueMutex, NULL);
    }

    ~Monome()
    {
      monome_close(device);
    }

    // Methods
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetLED(const v8::Arguments& args);
    static v8::Handle<v8::Value> SetAll(const v8::Arguments& args);

    static v8::Handle<v8::Value> Start(const v8::Arguments& args);
    static v8::Handle<v8::Value> Stop(const v8::Arguments& args);

    // Accessors
    static v8::Handle<v8::Value> GetRows(v8::Local<v8::String> property, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetColumns(v8::Local<v8::String> property, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetRotation(v8::Local<v8::String> property, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetType(v8::Local<v8::String> property, const v8::AccessorInfo& info);
    static v8::Handle<v8::Value> GetDevPath(v8::Local<v8::String> property, const v8::AccessorInfo& info);

    static void SetRotation(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::AccessorInfo& info);

    // Callbacks
    static void EventCallback(EV_P_ ev_async *watcher, int revents);

    void Start();
    void Stop();

  private:
    static const char* ToCString(const v8::String::Utf8Value& value);

    static void *EventThread(void *userData);
    static void EventHandler(const monome_event_t *event, void *monome);
};

} // namespace node
#endif  // MONOME_JS_H_