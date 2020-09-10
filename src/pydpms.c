/* Python bindings to DPMS X11 extension for controlling monitor state */

#include <Python.h>
#include <structmember.h>

#include <X11/Xutil.h>
#include <X11/extensions/dpms.h>

/* http://python3porting.com/cextensions.html */
#if PY_MAJOR_VERSION >= 3
  #define MOD_ERROR_VAL NULL
  #define MOD_SUCCESS_VAL(val) val
  #define MOD_INIT(name) PyMODINIT_FUNC PyInit_##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          static struct PyModuleDef moduledef = { \
            PyModuleDef_HEAD_INIT, name, doc, -1, methods, }; \
          ob = PyModule_Create(&moduledef);
#else
  #define MOD_ERROR_VAL
  #define MOD_SUCCESS_VAL(val)
  #define MOD_INIT(name) void init##name(void)
  #define MOD_DEF(ob, name, doc, methods) \
          ob = Py_InitModule3(name, methods, doc);
#endif

typedef struct {
  PyObject_HEAD
  Display *dpy;
  PyBytesObject *display;
} pyDPMSObject;

static char *get_disp(PyBytesObject *display) {
  if(!display)
    return NULL;
  return PyBytes_AS_STRING(display);
}

static int pyDPMS_init(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"display", NULL};

  self->display = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &(self->display))){
    PyErr_SetString(PyExc_Exception, "Optional display keyword must be a string. e.g. ':0'");
    return -1;
  }
  if(self->display) { Py_INCREF(self->display); }

  self->dpy = XOpenDisplay(get_disp(self->display)); /*  Open display */
  if (!self->dpy) {
    PyErr_SetString(PyExc_Exception, "Cannot open display");
    return -1;
  }
  return 0;
}

static void pyDPMS_dealloc(PyObject* _self) {
  pyDPMSObject *self= (pyDPMSObject *) _self;
  if(self->display) { Py_DECREF(self->display); }
  if(self->dpy) { XCloseDisplay(self->dpy); }
  self->dpy = NULL;
  self->display = NULL;
  Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *pyDPMS_str(PyObject *self) {
  return PyUnicode_FromFormat("DPMS(display=%s)", XDisplayName(get_disp(((pyDPMSObject *) self)->display)));
}

static PyObject *pyDPMS_display(pyDPMSObject* self) {
  return PyUnicode_FromString(XDisplayName(get_disp(self->display)));
}

static PyObject *pyDPMS_query_extension(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  int event_basep=0, error_basep=0;
  PyObject *result = PyBool_FromLong(DPMSQueryExtension(self->dpy, &event_basep, &error_basep));
  return Py_BuildValue("Nii", result, event_basep, error_basep);
}

static PyObject *pyDPMS_get_version(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  int major, minor;
  if (DPMSGetVersion(self->dpy, &major, &minor)) {
    return Py_BuildValue("ii", major, minor);
  } else {
    PyErr_SetString(PyExc_Exception, "Error getting version number.");
    return NULL;
  }
}

static PyObject *pyDPMS_capable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  return PyBool_FromLong(DPMSCapable(self->dpy));
}

static PyObject *pyDPMS_get_timeouts(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  CARD16 standby, suspend, off;
  if(DPMSGetTimeouts(self->dpy, &standby, &suspend, &off)) {
    return Py_BuildValue("iii", standby, suspend, off);
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error getting timeouts.");
    return NULL;
  }
}

static PyObject *pyDPMS_set_timeouts(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"standby", "suspend", "off", NULL};
  CARD16 standby, suspend, off;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "HHH", kwlist, &standby, &suspend, &off)) {
    PyErr_SetString(PyExc_Exception, "Bad arguments. Should be (int standby, int suspend, int off).");
    return NULL;
  }

  Status ret = DPMSSetTimeouts(self->dpy, standby, suspend, off);
  if(ret == 1) {
    return pyDPMS_get_timeouts(self, args, kwds);
  } else {
    PyErr_SetString(PyExc_Exception, "Bad arguments");
    return NULL;
  }
}

static PyObject *pyDPMS_enable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  if(DPMSEnable(self->dpy) == 1) {
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error Enabling DPMS.");
    return NULL;
  }
}

static PyObject *pyDPMS_disable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  if(DPMSDisable(self->dpy) == 1) {
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error Disabling DPMS.");
    return NULL;
  }
}

static PyObject *pyDPMS_force_level(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"level", NULL};
  CARD16 level;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "H", kwlist, &level)) {
    PyErr_SetString(PyExc_Exception, "Bad arguments. Should be (int level).");
    return NULL;
  }

  Status ret = DPMSForceLevel(self->dpy, level);
  if(ret == 1) {
    Py_RETURN_NONE;
  } else if(ret == BadValue) {
    PyErr_SetString(PyExc_Exception, "Bad level.");
    return NULL;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error ForceLevel.");
    return NULL;
  }
}

static PyObject *pyDPMS_info(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  CARD16 power_level;
  BOOL state;
  if(DPMSInfo(self->dpy, &power_level, &state) == 1) {
    return Py_BuildValue("iN", power_level, PyBool_FromLong(state));
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error getting Info.");
    return NULL;
  }
}

static PyMethodDef pyDPMS_methods[] = {
    {"display", (PyCFunction)pyDPMS_display, METH_NOARGS, "Returns the display variable of this DPMS"},
    {"query_extension", (PyCFunction)pyDPMS_query_extension, METH_NOARGS, "DPMSQueryExtension wrapper. Takes no arguments and returns a tuple (result bool, event_basep int, error_basep int)" },
    {"get_version", (PyCFunction)pyDPMS_get_version, METH_NOARGS, "DPMSGetVersion wrapper. Returns tuple (major, minor) version upon success or throws exception." },
    {"capable", (PyCFunction)pyDPMS_capable, METH_NOARGS, "DPMSCapable wrapper. Returns bool." },
    {"get_timeouts", (PyCFunction)pyDPMS_get_timeouts, METH_NOARGS, "DPMSGetTimeouts wrapper. Returns tuple (standby, suspend, off) or raises an exception on error." },
    {"set_timeouts", (PyCFunction)pyDPMS_set_timeouts, METH_VARARGS | METH_KEYWORDS, "DPMSSetTimeouts wrapper. Arguments are (int standby, int suspend, int off). Returns tuple with new values (standby, suspend, off). Raises Exception if an invalid timeouts provided." },

    {"enable", (PyCFunction)pyDPMS_enable, METH_NOARGS, "DPMSEnable wrapper." },
    {"disable", (PyCFunction)pyDPMS_disable, METH_NOARGS, "DPMSDisable wrapper." },
    {"force_level", (PyCFunction)pyDPMS_force_level, METH_VARARGS | METH_KEYWORDS, "DPMSForceLevel wrapper. Arguments are (int level). Returns None. Raises Exception if an invalid level provided. Use DPMSModeOn, DPMSModeStandby, DPMSModeSuspend, DPMSModeOff module attributes for level." },
    {"info", (PyCFunction)pyDPMS_info, METH_NOARGS, "DPMSInfo wrapper.  Returns tuple (power_level, state). Raises Exception on error." },
    {NULL}  /* Sentinel */
};


static PyTypeObject pyDPMSType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "dpms.DPMS",                              /* tp_name*/
    sizeof(pyDPMSObject),                     /* tp_basicsize*/
    0,                                        /* tp_itemsize*/
    pyDPMS_dealloc,                           /* tp_dealloc*/
    0,                                        /* tp_print*/
    0,                                        /* tp_getattr*/
    0,                                        /* tp_setattr*/
    0,                                        /* tp_compare*/
    pyDPMS_str,                               /* tp_repr*/
    0,                                        /* tp_as_number*/
    0,                                        /* tp_as_sequence*/
    0,                                        /* tp_as_mapping*/
    0,                                        /* tp_hash */
    0,                                        /* tp_call*/
    pyDPMS_str,                               /* tp_str*/
    0,                                        /* tp_getattro*/
    0,                                        /* tp_setattro*/
    0,                                        /* tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags*/
    "Python DPMS wrapper",                    /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    pyDPMS_methods,                           /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    (initproc)pyDPMS_init,                    /* tp_init */
};


static PyMethodDef dpms_methods[] = {
  {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif

MOD_INIT(dpms) {
    PyObject* module;

    pyDPMSType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyDPMSType) < 0)
        return MOD_ERROR_VAL;

    Py_INCREF(&pyDPMSType);

    MOD_DEF(module, "dpms", "Python bindings to DPMS X11 Extension", dpms_methods);

    PyModule_AddObject(module, "DPMS",            (PyObject *)&pyDPMSType);
    PyModule_AddObject(module, "DPMSModeOn",      PyLong_FromLong(DPMSModeOn     ));
    PyModule_AddObject(module, "DPMSModeStandby", PyLong_FromLong(DPMSModeStandby));
    PyModule_AddObject(module, "DPMSModeSuspend", PyLong_FromLong(DPMSModeSuspend));
    PyModule_AddObject(module, "DPMSModeOff",     PyLong_FromLong(DPMSModeOff    ));

    return MOD_SUCCESS_VAL(module);
}
