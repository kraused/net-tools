
#include <python2.7/Python.h>

#include "common.h"
#include "ibutil.h"
#include "adapter.h"

/* Keys for dictionary
 */
static PyObject *pyString_lid;
static PyObject *pyString_lft;
static PyObject *pyString_nodeType;
static PyObject *pyString_numPorts;
static PyObject *pyString_sysGuid;
static PyObject *pyString_nodeGuid;
static PyObject *pyString_portGuid;
static PyObject *pyString_info;

static void convertUInt16List(AllocFunction alloc, void *allocUd,
                              PyObject *pyList, SInt32 *len, UInt16 **list)
{
	PyObject *item;
	SInt32 i, n;
	long   number;

	n = PyList_Size(pyList);

	*len  = n;
	*list = alloc(allocUd, NULL, 0, (*len)*sizeof(UInt16));

	for (i = 0; i < n; ++i) {
		item = PyList_GetItem(pyList, i);
		if (UNLIKELY(!item)) {
			FATAL("Failed to get %dth item", i);
		}
		if (UNLIKELY(!PyInt_Check(item))) {
			FATAL("%dth item is not an integer", i);
		}

		number = PyInt_AsLong(item);
		if (UNLIKELY((number < 0) || (number > UINT16_MAX))) {
			FATAL("Invalid lid in slot %d", i);
		}

		(*list)[i] = (UInt16 )number;
	}
}

static PyObject *convertLinearForwardingTable(struct LinearForwardingTable *LFT)
{
	PyObject *d;
	PyObject *list;
	SInt32 i, err;

	d = PyDict_New();
	if (UNLIKELY(!d)) {
		FATAL("PyDict_New() returned NULL");
	}

	list = PyList_New(LFT->len);
	for (i = 0; i < LFT->len; ++i) {
		err = PyList_SetItem(list, i, PyInt_FromLong(LFT->lft[i]));
		if (UNLIKELY(err)) {
			FATAL("PyList_SetItem() failed");
		}
	}

#define SET_PYDICT_ITEM_FROM_INT(D, KEY, VAL)				\
	do {								\
		err = PyDict_SetItem(D, KEY, PyInt_FromLong(VAL));	\
		if (UNLIKELY(err)) {					\
			FATAL("PyDict_SetItem() failed");		\
		}							\
	} while(0)
#define SET_PYDICT_ITEM_FROM_STR(D, KEY, VAL)				\
	do {								\
		err = PyDict_SetItem(D, KEY, PyString_FromString(VAL));	\
		if (UNLIKELY(err)) {					\
			FATAL("PyDict_SetItem() failed");		\
		}							\
	} while(0)
	/* We do not use PyString_FromFormat() here since this function only supports
	 * a subset of the libc format characters.
 	 */
#define SET_PYDICT_ITEM_FROM_STRFORMAT(D, KEY, FMT, VAL)		\
	do {								\
		char buf[256];						\
		snprintf(buf, sizeof(buf), FMT, VAL);			\
		SET_PYDICT_ITEM_FROM_STR(D, KEY, buf);			\
	} while(0)


	SET_PYDICT_ITEM_FROM_INT(d, pyString_lid, LFT->lid);
	err = PyDict_SetItem(d, pyString_lft, list);
	if (UNLIKELY(err)) {
		FATAL("PyDict_SetItem() failed");
	}

	return d;
}

static PyObject *convertNodeRecord(struct NodeRecord *NR)
{
	PyObject *d;
	SInt32 err;

	d = PyDict_New();
	if (UNLIKELY(!d)) {
		FATAL("PyDict_New() returned NULL");
	}

	SET_PYDICT_ITEM_FROM_INT(d, pyString_lid     , NR->lid);
	SET_PYDICT_ITEM_FROM_INT(d, pyString_nodeType, NR->nodeType);
	SET_PYDICT_ITEM_FROM_INT(d, pyString_numPorts, NR->numPorts);
	SET_PYDICT_ITEM_FROM_STRFORMAT(d, pyString_sysGuid , "0x%" PRIx64, NR->sysGuid);
	SET_PYDICT_ITEM_FROM_STRFORMAT(d, pyString_nodeGuid, "0x%" PRIx64, NR->nodeGuid);
	SET_PYDICT_ITEM_FROM_STRFORMAT(d, pyString_portGuid, "0x%" PRIx64, NR->portGuid);
	SET_PYDICT_ITEM_FROM_STR(d, pyString_info, (char *)NR->info);

	return d;
}

static PyObject* Get_NodeRecord(PyObject *self, PyObject *args)
{
	PyObject *pyListLIDs = NULL;
	char   *CA;
	SInt16 port;
	UInt16 lid;
	SInt32 err;
	SInt32 numLIDs, numNRs;
	UInt16 *LIDs;
	SInt32 i;
	struct NodeRecord *NRs;
	PyObject *pyListNRs;

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
		return Py_None;
	}

	if (!PyArg_ParseTuple(args, "|O", &pyListLIDs)) {
		return Py_None;
	}

	Py_XINCREF(pyListLIDs);

	numLIDs = 0;
	LIDs    = NULL;
	if (pyListLIDs) {
		convertUInt16List(defaultMemoryAlloc, NULL, pyListLIDs, &numLIDs, &LIDs);
	}

	Py_XDECREF(pyListLIDs);

	getNodeRecords(defaultMemoryAlloc, NULL, CA, port,
                       numLIDs, LIDs, &numNRs, &NRs);

	if (LIDs) {
		LIDs = defaultMemoryAlloc(NULL, LIDs, numLIDs*sizeof(UInt16), 0);
	}

	pyListNRs = PyList_New(numNRs);
	if (UNLIKELY(!pyListNRs)) {
		FATAL("PyList_New() returned NULL");
	}

	for (i = 0; i < numNRs; ++i) {
		err = PyList_SetItem(pyListNRs, i, convertNodeRecord(&NRs[i]));
	}
	defaultMemoryAlloc(NULL, NRs, numNRs*sizeof(struct NodeRecord), 0);

	return pyListNRs;
}

static PyObject* Get_LinearForwardingTable(PyObject *self, PyObject *args)
{
	PyObject *pyListLIDs = NULL;
	char   *CA;
	SInt16 port;
	UInt16 lid;
	SInt32 err;
	SInt32 numLIDs, numLFTs;
	UInt16 *LIDs;
	SInt32 i;
	struct LinearForwardingTable *LFTs;
	PyObject *pyListLFTs;

	err = firstActivePort(defaultMemoryAlloc, NULL, &CA, &port, &lid);
	if (UNLIKELY(err < 0)) {
		ERROR("firstActivePort() failed");
		return Py_None;
	}

	if (!PyArg_ParseTuple(args, "|O", &pyListLIDs)) {
		return Py_None;
	}

	Py_XINCREF(pyListLIDs);

	numLIDs = 0;
	LIDs    = NULL;
	if (pyListLIDs) {
		convertUInt16List(defaultMemoryAlloc, NULL, pyListLIDs, &numLIDs, &LIDs);
	}

	Py_XDECREF(pyListLIDs);

	getLinearForwardingTables(defaultMemoryAlloc, NULL, CA, port,
                                  numLIDs, LIDs, &numLFTs, &LFTs);

	if (LIDs) {
		LIDs = defaultMemoryAlloc(NULL, LIDs, numLIDs*sizeof(UInt16), 0);
	}

	pyListLFTs = PyList_New(numLFTs);
	if (UNLIKELY(!pyListLFTs)) {
		FATAL("PyList_New() returned NULL");
	}

	for (i = 0; i < numLFTs; ++i) {
		err = PyList_SetItem(pyListLFTs, i, convertLinearForwardingTable(&LFTs[i]));
		defaultMemoryAlloc(NULL, LFTs[i].lft, LFTs[i].len*sizeof(UInt16), 0);
	}
	defaultMemoryAlloc(NULL, LFTs, numLFTs*sizeof(struct LinearForwardingTable), 0);

	return pyListLFTs;
}

static PyMethodDef methods[] =
{
	{"Get_NodeRecord", Get_NodeRecord, METH_VARARGS, "Get node records"},
	{"Get_LinearForwardingTable", Get_LinearForwardingTable, METH_VARARGS, "Get linear forwarding tables"},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initibutilpy()
{
	PyObject *module;

	module = Py_InitModule("ibutilpy", methods);

#define STATIC_PY_STRING(string)	pyString_ ## string = PyString_FromString(#string)

	STATIC_PY_STRING(lid);
	STATIC_PY_STRING(lft);
	STATIC_PY_STRING(nodeType);
	STATIC_PY_STRING(numPorts);
	STATIC_PY_STRING(sysGuid);
	STATIC_PY_STRING(nodeGuid);
	STATIC_PY_STRING(portGuid);
	STATIC_PY_STRING(info);

	/* Node types
	 */
	PyModule_AddIntConstant(module, "IB_NODE_CA", 1);
	PyModule_AddIntConstant(module, "IB_NODE_SWITCH", 2);
}

