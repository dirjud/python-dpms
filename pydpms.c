/* Python bindings to DPMS X11 extension for controlling monitor state */

#include <Python.h>
#include <structmember.h>

#include <X11/Xutil.h>
#include <X11/extensions/dpms.h>


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
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|S", kwlist, &(self->display))){
    PyErr_SetString(PyExc_Exception, "optional display keywork must be a stirng. e.g. ':0'");
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
  Py_TYPE(self)->tp_free((PyObject *) self);
}

PyObject *pyDPMS_str(PyObject *self) {
  return PyUnicode_FromFormat("DPMS(display=%s)", XDisplayName(get_disp(((pyDPMSObject *) self)->display)));
}

static PyObject *pyDPMS_display(pyDPMSObject* self) {
  return PyBytes_FromString(XDisplayName(get_disp(self->display)));
}

static PyObject *pyDPMS_QueryExtension(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  int event_basep=0, error_basep=0;
  PyObject *result = PyBool_FromLong(DPMSQueryExtension(self->dpy, &event_basep, &error_basep));
  return Py_BuildValue("Nii", result, event_basep, error_basep);
}

static PyObject *pyDPMS_GetVersion(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  int major, minor;
  if (DPMSGetVersion(self->dpy, &major, &minor)) {
    return Py_BuildValue("ii", major, minor);
  } else {
    PyErr_SetString(PyExc_Exception, "Error getting version number.");
    return NULL;
  }
}

static PyObject *pyDPMS_Capable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  return PyBool_FromLong(DPMSCapable(self->dpy));
}

static PyObject *pyDPMS_GetTimeouts(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  CARD16 standby, suspend, off;
  if(DPMSGetTimeouts(self->dpy, &standby, &suspend, &off)) {
    return Py_BuildValue("iii", standby, suspend, off);
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error getting timeouts.");
    return NULL; 
  }
}

static PyObject *pyDPMS_SetTimeouts(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"standby", "suspend", "off", NULL};
  CARD16 standby, suspend, off;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "HHH", kwlist, &standby, &suspend, &off)) {
    PyErr_SetString(PyExc_Exception, "Bad arguments. should be (standby, suspend, off)");
    return NULL; 
  }

  Status ret = DPMSSetTimeouts(self->dpy, standby, suspend, off);
  if(ret == 1) {
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_Exception, "Bad arguments");
    return NULL;
  }
}

static PyObject *pyDPMS_Enable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  if(DPMSEnable(self->dpy) == 1) {
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error Enabling DPMS.");
    return NULL; 
  }
}

static PyObject *pyDPMS_Disable(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  if(DPMSDisable(self->dpy) == 1) {
    Py_RETURN_NONE;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error Disabling DPMS.");
    return NULL; 
  }
}

static PyObject *pyDPMS_ForceLevel(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
  static char *kwlist[] = {"level", NULL};
  CARD16 level;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "H", kwlist, &level)) {
    PyErr_SetString(PyExc_Exception, "Bad arguments. should be (level)");
    return NULL; 
  }

  Status ret = DPMSForceLevel(self->dpy, level);
  if(ret == 1) {
    Py_RETURN_NONE;
  } else if(ret == BadValue) {
    PyErr_SetString(PyExc_Exception, "Bad Level.");
    return NULL; 
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown Error ForceLevel.");
    return NULL; 
  }
}

static PyObject *pyDPMS_Info(pyDPMSObject *self, PyObject *args, PyObject *kwds) {
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
    {"QueryExtension", (PyCFunction)pyDPMS_QueryExtension, METH_NOARGS, "DPMSQueryExtension wrapper. Takes no arguments and returns a tuple (result bool, event_basep int, error_basep int)" },
    {"GetVersion", (PyCFunction)pyDPMS_GetVersion, METH_NOARGS, "DPMSGetVersion wrapper. Returns tuple (major, minor) version upon success or throws exception." },
    {"Capable", (PyCFunction)pyDPMS_Capable, METH_NOARGS, "DPMSCapable wrapper. Returns bool." },
    {"GetTimeouts", (PyCFunction)pyDPMS_GetTimeouts, METH_NOARGS, "DPMSGetTimeouts wrapper. Returns tuple (standby, suspend, off) or raises an exception on error." },
    {"SetTimeouts", (PyCFunction)pyDPMS_SetTimeouts, METH_VARARGS | METH_KEYWORDS, "DPMSSetTimeouts wrapper. Arguments are (int standby, int suspend, int off). Returns None. Raises Exception if an invalid timeouts provided." },

    {"Enable", (PyCFunction)pyDPMS_Enable, METH_NOARGS, "DPMSEnable wrapper." },
    {"Disable", (PyCFunction)pyDPMS_Disable, METH_NOARGS, "DPMSDisable wrapper." },
    {"ForceLevel", (PyCFunction)pyDPMS_ForceLevel, METH_VARARGS | METH_KEYWORDS, "DPMSForceLevel wrapper. Arguments are (int level). Returns None. Raises Exception if an invalid level provided. Use DPMSModeOn, DPMSModeStandby, DPMSModeSuspend, DPMSModeOff module attributes for level." },
    {"Info", (PyCFunction)pyDPMS_Info, METH_NOARGS, "DPMSInfo wrapper.  Returns tuple (power_level, state). Raises Exception on error." },

    
    //    {"QueryExtension", (PyCFunction)pyDPMS_QueryExtension, METH_VARARGS | METH_KEYWORDS, "DPMSQueryExtension wrapper" },
//    {"copy", (PyCFunction)pyimg_copy, METH_NOARGS,
//     "Returns a copy of the image where the image data is copied."
//    },
    {NULL}  /* Sentinel */
};


static PyTypeObject pyDPMSType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "dpms.DPMS",           /*tp_name*/
    .tp_basicsize = sizeof(pyDPMSObject),     /*tp_basicsize*/
    .tp_itemsize = 0,                         /*tp_itemsize*/
    .tp_dealloc = pyDPMS_dealloc,            /*tp_dealloc*/
    .tp_repr = pyDPMS_str,                 /*tp_repr*/
    .tp_str = pyDPMS_str,                 /*tp_str*/
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /*tp_flags*/
    .tp_doc = "Python DPMS wrapper", /* tp_doc */
    .tp_methods = pyDPMS_methods,            /* tp_methods */
    .tp_init = (initproc)pyDPMS_init,      /* tp_init */
};

static PyMethodDef dpms_methods[] = {
  {NULL}  /* Sentinel */
};

#define MODNAME "dpms"
#define MODDOC "Python bindings to DPMS X11 extension"

#if PY_MAJOR_VERSION >= 3

static PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    .m_name = MODNAME,
    .m_doc = MODDOC,
    .m_size = -1,
    .m_methods = dpms_methods,
};

#define INITERROR return NULL

PyMODINIT_FUNC PyInit_dpms(void) {
#else

#define INITERROR return

void initdpms(void) {
#endif

    PyObject* m;

    pyDPMSType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyDPMSType) < 0)
        INITERROR;

#if PY_MAJOR_VERSION >= 3
    m = PyModule_Create(&moduledef);
#else
    m = Py_InitModule3(MODNAME, dpms_methods,
                       MODDOC);
#endif

    Py_INCREF(&pyDPMSType);
    PyModule_AddObject(m, "DPMS", (PyObject *)&pyDPMSType);

    PyModule_AddObject(m, "DPMSModeOn",      PyLong_FromLong(DPMSModeOn     ));
    PyModule_AddObject(m, "DPMSModeStandby", PyLong_FromLong(DPMSModeStandby));
    PyModule_AddObject(m, "DPMSModeSuspend", PyLong_FromLong(DPMSModeSuspend));
    PyModule_AddObject(m, "DPMSModeOff",     PyLong_FromLong(DPMSModeOff    ));
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}

