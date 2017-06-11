#include "Python.h"
#include <tuple>
#include <vector>

using namespace std;

struct ByteArrAndSize {
  char* data;
  int size;
};

template<typename... Ts>
vector<tuple<Ts...>> get_args(char* filename, char* function) {
  vector<tuple<Ts...>> tcs;

  // setup python function call
  PyObject* myModule = PyImport_Import(PyString_FromString("get_tc"));
  PyObject* myFunction = PyObject_GetAttrString(myModule, "generate_tests");
  PyObject* args = PyTuple_Pack(2, PyString_FromString(filename), PyString_FromString(function));

  // Call Python function to create testcases
  PyObject* testCases = PyObject_CallObject(myFunction, args);

  // For each testcase
  for (int i = 0; i < PyList_Size(testCases); i++) {
    PyObject* testCase = PyList_GetItem(testCases, i);
    tcs.push_back(make_tuple());
    // for each argument
    for (int j = 0; j < PyList_Size(testCase); j++) {
      PyObject* arg = PyList_GetItem(testCase, j);
      if (PyInt_Check(arg)) {
        long data = PyInt_AsLong(arg);
        auto tc = tcs.back();
        tcs.pop_back();
        tcs.emplace_back(tuple_cat(tc, make_tuple(data)));
      }
      else if (PyByteArray_Check(arg)) {
        char* data = PyByteArray_AsString(arg);
        int size = PyByteArray_Size(arg);
        ByteArrAndSize s = ByteArrAndSize();
        s.data = data;
        s.size = size;
        auto tc = tcs.back();
        tcs.pop_back();
        tcs.emplace_back(tuple_cat(tc, make_tuple(s)));
      }
    }
  }

  return tcs;
}

int main() {
  // Set PYTHONPATH TO working directory
  setenv("PYTHONPATH",".",1);
  
  Py_Initialize();
  
  get_args((char*) "out", (char*) "function");
  get_args((char*) "out2", (char*) "function");
  get_args((char*) "out3", (char*) "function");
  get_args((char*) "out4", (char*) "function");
  get_args((char*) "out5", (char*) "function");
  
  Py_Finalize();

  return 0;
}
