#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

using namespace v8;
using namespace node;

// File descriptor
int fd;

Handle<Value> i2c_open(const Arguments& args) {
  // Get scope
  HandleScope scope;
  // Get port number
  int port = args[1]->Int32Value();
  // Get device
  String::Utf8Value utf8(args[0]->ToString());
  int len = utf8.length() + 1;
  char *device = (char *) calloc(sizeof(char), len);
  strncpy(device, *utf8, len);
  // Get file descriptor
  fd = open(device, O_RDWR);
  // Check for fail in opening
  if(fd < 0){
    return scope.Close(Boolean::New(false));
  }
  // IO control to i2c and check for fail
  if(ioctl(fd, I2C_SLAVE, port)<0){
    return scope.Close(Boolean::New(false));
  }
  // Return true if all is good
  return scope.Close(Boolean::New(true));
}

Handle<Value> i2c_write(const Arguments& args) {
  // Get scope
  HandleScope scope;
  // Get buffer 
  Local<Value> buffer = args[0];
  // Get length from buffer
  size_t length = Buffer::Length(buffer->ToObject());
  // Read data from buffer
  char* bufferData = Buffer::Data(buffer->ToObject());
  // Write data
  if(write(fd, bufferData, length) < 0){
    return scope.Close(Boolean::New(false));
  }
  // REturn true if all is good
  return scope.Close(Boolean::New(true));
}

Handle<Value> i2c_read(const Arguments& args) {
  // Get scope
  HandleScope scope;
  // Get length to read
  int length = args[0]->Int32Value();
  // Define data
  char *data[length];
  // Read bytes into data
  read(fd, data, length);
  // Create a slow buffer
  node::Buffer *slowBuffer = node::Buffer::New(length);
  // Copy data to slow buffer
  memcpy(node::Buffer::Data(slowBuffer), data, length);
  // Convert to JS buffer
  Local<Object> globalObj = Context::GetCurrent()->Global();
  Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
  Handle<Value> constructorArgs[3] = { slowBuffer->handle_, Integer::New(length), Integer::New(0) };
  Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
  // Return JS buffer
  return scope.Close(actualBuffer);
}

void init(Handle<Object> target) {
  NODE_SET_METHOD(target, "open", i2c_open);
  NODE_SET_METHOD(target, "write", i2c_write);
  NODE_SET_METHOD(target, "read", i2c_read);
}

NODE_MODULE(i2c, init);