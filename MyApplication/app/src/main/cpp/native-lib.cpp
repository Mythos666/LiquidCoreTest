#include <jni.h>
#include <string>

#include <JavaScriptCore/JavaScript.h>

using namespace std;

class Point
{
public:
    static int refCount;

    float x, y;
    Point()
    {
        x = 0;
        y = 0;
    }
    Point(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
    void multiply(float scalar)
    {
        x *= scalar;
        y *= scalar;
    }
};
int Point::refCount = 0;


JSClassRef PointClass();
JSObjectRef Point_CTOR(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Point::refCount ++;
    Point *p = nullptr;
    if(argumentCount == 0)
    {
        p = new Point();
    }
    else if(argumentCount == 2)
    {
        double x = JSValueToNumber(ctx, arguments[0], nullptr);
        double y = JSValueToNumber(ctx, arguments[1], nullptr);
        p = new Point((float)x, (float)y);
    }
    else
    {
        //TODO: set the exception here
    }

    constructor = JSObjectMake(ctx, PointClass(), static_cast<void*>(p));
    return constructor;
}
JSValueRef Point_GetX(JSContextRef ctx, JSObjectRef object,JSStringRef propertyName, JSValueRef* exception) {
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    return JSValueMakeNumber(ctx, p->x);
}
bool Point_SetX(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception) {
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    double jsX = JSValueToNumber(ctx, value, nullptr);
    p->x = (float)jsX;

    return true;
}
JSValueRef Point_GetY(JSContextRef ctx, JSObjectRef object,JSStringRef propertyName, JSValueRef* exception) {
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    return JSValueMakeNumber(ctx, p->y);
}
bool Point_SetY(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception) {
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    double jsY = JSValueToNumber(ctx, value, nullptr);
    p->y = (float)jsY;

    return true;
}
JSValueRef Point_Multiply(JSContextRef ctx, JSObjectRef function, JSObjectRef object, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception){
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    double scalar = JSValueToNumber(ctx, arguments[0], nullptr);
    p->multiply((float)scalar);

    return JSValueMakeUndefined(ctx);
}
void Point_Dealloc(JSObjectRef object)
{
    Point* p = static_cast<Point*>(JSObjectGetPrivate(object));
    if(p)
    {
        Point::refCount --;
        delete p;
    }
}
JSClassRef PointClass()
{
    static JSClassRef pointClass;
    if(!pointClass)
    {
        JSClassDefinition classDefinition = kJSClassDefinitionEmpty;

        static JSStaticValue staticValues[] = {
                {"x", Point_GetX, Point_SetX, kJSPropertyAttributeDontDelete},
                {"y", Point_GetY, Point_SetY, kJSPropertyAttributeDontDelete},
                { 0, 0, 0, 0 }
        };
        static JSStaticFunction staticFunctions[] = {
                { "multiply", Point_Multiply, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
                { 0, 0, 0 }
        };

        classDefinition.className = "Point";
        classDefinition.callAsConstructor = Point_CTOR;
        classDefinition.staticValues = staticValues;
        classDefinition.staticFunctions = staticFunctions;
        classDefinition.finalize = Point_Dealloc;

        pointClass = JSClassCreate(&classDefinition);
    }
    return pointClass;
}

std::string JSStringToStdString(JSStringRef jsString) {
    size_t maxBufferSize = JSStringGetMaximumUTF8CStringSize(jsString);
    char* utf8Buffer = new char[maxBufferSize];
    size_t bytesWritten = JSStringGetUTF8CString(jsString, utf8Buffer, maxBufferSize);
    std::string utf_string = std::string (utf8Buffer, bytesWritten);
    delete [] utf8Buffer;
    return utf_string;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mythos_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    JSContextGroupRef contextGroup = JSContextGroupCreate();
    JSGlobalContextRef globalContext = JSGlobalContextCreateInGroup(contextGroup, nullptr);
    JSObjectRef globalObject = JSContextGetGlobalObject(globalContext);
    JSObjectRef pointObject = JSObjectMake(globalContext, PointClass(), nullptr);
    JSObjectSetProperty(globalContext, globalObject, JSStringCreateWithUTF8CString("Point"), pointObject, kJSPropertyAttributeNone, nullptr);
    JSStringRef callStatement = JSStringCreateWithUTF8CString("for(var i = 0; i < 100000; i++) {let p = new Point(100, 100)} 'completed'");
    JSValueRef result = JSEvaluateScript(globalContext, callStatement, nullptr, nullptr, 1,nullptr);
    JSStringRef resultJSStringRef = JSValueToStringCopy(globalContext, result, nullptr);
    std::string resultString = JSStringToStdString(resultJSStringRef);


//    JSGlobalContextRelease(globalContext);
//    JSContextGroupRelease(contextGroup);
    JSStringRelease(callStatement);
    JSStringRelease(resultJSStringRef);

    return env->NewStringUTF(std::to_string(Point::refCount).c_str()); //resultString.c_str()
}

