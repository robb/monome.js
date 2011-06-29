#include <v8.h>
#include <node.h>

namespace monome {

class Monome: node::ObjectWrap
{
private:
    //monome_t monome;

public:
    static v8::Persistent<v8::FunctionTemplate> s_ct;
    static void Init(v8::Handle<v8::Object> target)
    {
        v8::HandleScope scope;
    
        v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New();
    
        s_ct = v8::Persistent<v8::FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(v8::String::NewSymbol("Monome"));
    
        NODE_SET_PROTOTYPE_METHOD(s_ct, "setLED", SetLED);
    
        target->Set(v8::String::NewSymbol("Monome"),
                    s_ct->GetFunction());
    }
    
    Monome()
    {
    }
    
    ~Monome()
    {
    }

    static v8::Handle<v8::Value> New(const v8::Arguments& args)
    {
        v8::HandleScope scope;
        
        Monome* m = new Monome();
        m->Wrap(args.This());
        
        return args.This();
    }

    static v8::Handle<v8::Value> SetLED(const v8::Arguments& args)
    {
        return v8::Undefined();
    }

};

v8::Persistent<v8::FunctionTemplate> Monome::s_ct;

extern "C" {
  static void init (v8::Handle<v8::Object> target)
  {
    Monome::Init(target);
  }

  NODE_MODULE(monome, init);
}

} // Namespace monome