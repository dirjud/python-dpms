/* Python bindings to DPMS X11 extension for controlling monitor state */

#include <Python.h>
#include <structmember.h>

#include <X11/Xutil.h>
#include <X11/extensions/dpms.h>


typedef struct {
  PyObject_HEAD
  Display *dpy;
  PyStringObject *display;
} pyDPMSObject;

static char *get_disp(PyStringObject *display) {
  if(!display)
    return NULL;
  return PyString_AS_STRING(display);
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
  self->ob_type->tp_free((PyObject*)self);
}

PyObject *pyDPMS_str(PyObject *self) {
  return PyString_FromFormat("DPMS(display=%s)", XDisplayName(get_disp(((pyDPMSObject *) self)->display)));
}

static PyObject *pyDPMS_display(pyDPMSObject* self) {
  return PyString_FromString(XDisplayName(get_disp(self->display)));
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
    0,                     /*ob_size*/
    "dpms.DPMS",           /*tp_name*/
    sizeof(pyDPMSObject),     /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    pyDPMS_dealloc,            /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    pyDPMS_str,                 /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    pyDPMS_str,                 /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /*tp_flags*/
    "Python DPMS wrapper", /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    pyDPMS_methods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)pyDPMS_init,      /* tp_init */
};

static PyMethodDef dpms_methods[] = {
  {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC initdpms(void) {
    PyObject* m;

    pyDPMSType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&pyDPMSType) < 0)
        return;

    m = Py_InitModule3("dpms", dpms_methods,
                       "Python bindings to DPMS X11 extension");

    Py_INCREF(&pyDPMSType);
    PyModule_AddObject(m, "DPMS", (PyObject *)&pyDPMSType);

    PyModule_AddObject(m, "DPMSModeOn",      PyInt_FromLong(DPMSModeOn     ));
    PyModule_AddObject(m, "DPMSModeStandby", PyInt_FromLong(DPMSModeStandby));
    PyModule_AddObject(m, "DPMSModeSuspend", PyInt_FromLong(DPMSModeSuspend));
    PyModule_AddObject(m, "DPMSModeOff",     PyInt_FromLong(DPMSModeOff    ));
}
