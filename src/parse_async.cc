#include <string>
#include "drafter.h"
#include "protagonist.h"
#include "refractToV8.h"

using std::string;
using namespace v8;
using namespace protagonist;

using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;

namespace
{

    class ParseWorker : public AsyncWorker
    {
        drafter_parse_options parseOptions;
        drafter_serialize_options serializeOptions;
        Nan::Utf8String* sourceData;

        Nan::Persistent<v8::Promise::Resolver>* persistent;

        // Output
        drafter_result* result;
        int parse_err_code;

    public:
        ParseWorker(Callback* callback,
            Nan::Persistent<v8::Promise::Resolver>* persistent,
            drafter_parse_options parseOptions,
            drafter_serialize_options serializeOptions,
            Nan::Utf8String* sourceData)
            : AsyncWorker(callback)
            , parseOptions(parseOptions)
            , serializeOptions(serializeOptions)
            , sourceData(sourceData)
            , persistent(persistent)
            , result(nullptr)
            , parse_err_code(-1)
        {
        }

        void Execute()
        {
            parse_err_code = drafter_parse_blueprint(*(*sourceData), &result, parseOptions);
        }

        void HandleOKCallback()
        {

            Nan::HandleScope scope;

            if (persistent) {
                auto resolver = Nan::New(*persistent);

                if (!parse_err_code) {
                    resolver->Resolve(Nan::GetCurrentContext(), refract2v8(result, serializeOptions));
                } else {
                    resolver->Reject(Nan::GetCurrentContext(), refract2v8(result, serializeOptions));
                }
                v8::Isolate::GetCurrent()->RunMicrotasks();
                return;
            } else if (callback) {

                v8::Local<v8::Value> argv[] = {Nan::Null(), refract2v8(result, serializeOptions)};

                if (0 != parse_err_code) {
                    argv[0] = annotations2v8(result);
                }

                callback->Call(2, argv);
                return;
            }
            Nan::ThrowTypeError("not handled OKCallback");
        }

        virtual ~ParseWorker()
        {
            if (result) {
                drafter_free_result(result);
                result = nullptr;
            }

            if (sourceData) {
                delete sourceData;
                sourceData = nullptr;
            }
        }
    };
}

/**
 * posible args variant
 * (source) -> promise
 * (source, options) -> promise
 * (source, callback) -> callback
 * (source, option, callback) -> callback
 */

static bool isLastParamCallback(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
    return info[info.Length()-1]->IsFunction();
}

NAN_METHOD(protagonist::Parse)
{
    Nan::HandleScope scope;
    Nan::Persistent<v8::Promise::Resolver>* persistent = nullptr;
    Callback* callback = nullptr;
    v8::Local<v8::Promise::Resolver> resolver;

    // Check arguments

    size_t optionIndex = 0;
    size_t callbackIndex = 0;

    if (info.Length() < 1 || info.Length() > 3) {
        Nan::ThrowTypeError("wrong number of arguments, `parse(string, options, callback)` expected");
        return;
    }

    if (!info[0]->IsString()) {
        Nan::ThrowTypeError("wrong 1st argument - string expected");
        return;
    }

    if (isLastParamCallback(info)) { // last arg is callback funtion
        if (info.Length() == 3) {
            optionIndex = 1;
        }
        callbackIndex = info.Length() - 1;
    } else { // is Promise
        if (info.Length() == 2) {
            optionIndex = 1;
        } else if (info.Length() > 2) { // promise shoud not have more than 2 params
            Nan::ThrowSyntaxError("wrong number of arguments, `parse(string, [options])` expected");
            return;
        }
    }

    if (optionIndex && !info[optionIndex]->IsObject()) {
        Nan::ThrowTypeError("wrong 2nd argument - `options` expected `parse(string[, options][, callback])`");
        return;
    }

    // Prepare options
    drafter_parse_options parseOptions = { false };
    drafter_serialize_options serializeOptions = { false, DRAFTER_SERIALIZE_JSON };

    if (optionIndex) {
        OptionsResult* optionsResult = ParseOptionsObject(Handle<Object>::Cast(info[optionIndex]), false);

        if (optionsResult->error != NULL) {
            Nan::TypeError(optionsResult->error);
            return;
        } else {
            parseOptions = optionsResult->parseOptions;
            serializeOptions = optionsResult->serializeOptions;
            FreeOptionsResult(&optionsResult);
        }

    }

    if (callbackIndex) {
        callback = new Callback(info[callbackIndex].As<Function>());
    } else {
        resolver = v8::Promise::Resolver::New(info.GetIsolate());
        persistent = new Nan::Persistent<v8::Promise::Resolver>(resolver);
    }

    AsyncQueueWorker(
        new ParseWorker(callback, persistent, parseOptions, serializeOptions, new Nan::Utf8String(info[0])));

    if (!callbackIndex) {
        info.GetReturnValue().Set(resolver->GetPromise());
    }

}
