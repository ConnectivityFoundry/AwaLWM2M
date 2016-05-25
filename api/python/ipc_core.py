#/************************************************************************************************************************
# Copyright (c) 2016, Imagination Technologies Limited and/or its affiliated group companies.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
# following conditions are met:
#     1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
#        following disclaimer.
#     2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
#        following disclaimer in the documentation and/or other materials provided with the distribution.
#     3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote
#        products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#************************************************************************************************************************/

"""
IPC Core Functionality
"""

from lxml import etree

g_debug = True
def debug(msg):
    if g_debug:
        print(msg)

def _serialize(header, msgType, sessionID, content):
    e_root = etree.Element(header)
    e_root.append(TElement("Type", str(msgType)))
    if sessionID is not None:
        e_root.append(TElement("SessionID", str(sessionID)))
    if content is not None:
        e_root.append(content)
    return etree.tostring(e_root, pretty_print=True)

def is_sequence(arg):
    """Test for sequence, excluding strings: http://stackoverflow.com/a/1835259/143397"""
    return (not hasattr(arg, "strip") and
            hasattr(arg, "__getitem__") or
            hasattr(arg, "__iter__"))

class IpcError(Exception):
    pass

class IpcMessage(object):
    """Abstract Base Class for all IPC messages."""
    Header = "BaseHeader"
    MessageType = "BaseType"

    def __init__(self):
        pass

    def __str__(self):
        return "%s:%s" % (type(self).Header, type(self).MessageType)

class IpcContent(object):
    def __init__(self, element=None):
        if element is not None:
            e_content = element.find("Content")
            self._model = DataModel(e_content)
        else:
            self._model = DataModel()

    def __str__(self):
        return str(self._model)

    def add(self, path, value, label=None):
        """Path is a tuple: (objectID, objectInstanceID, resourceID, resourceInstanceID)"""
        self._model.add(path, value, label)

    def getElement(self):
        e_content = etree.Element("Content")
        e_model = self._model.getElement()
        # strip the outer container
        for child in e_model.getchildren():
            e_content.append(child)
        return e_content

    def getValue(self, path):
        """Path is a tuple: (objectID, objectInstanceID, resourceID, resourceInstanceID)"""
        return self._model.getValue(path)

class IpcRequest(IpcMessage):
    """Base request class - represents a request from an IPC client to an IPC service."""
    Header = "Request"
    SupportedModelPaths = ( "O", "OI", "OIR", "OIRi" )  # support all by default
    ContentType = IpcContent
    PathLabel = None

    def __init__(self, session_id=None):
        self._session_id = session_id
        self._content = type(self).ContentType()

    def __str__(self):
        return super(IpcRequest, self).__str__() + " %s" % (str(self._content),)

    def add(self, path, value=None):
        if not is_sequence(path) or not validateModelPath(path, type(self).SupportedModelPaths):
            raise IpcError("Unsupported path")
        self._content.add(path, value, self.PathLabel)

    def serialize(self):
        # use specified MessageType
        return _serialize(self.Header, self.MessageType, self._session_id, self._content.getElement())

    @property
    def session_id(self):
        return self._session_id

class IpcResponse(IpcMessage):
    """Base response class - represents a response from an IPC service to an IPC client."""
    Header = "Response"
    ContentType = IpcContent

    def __init__(self, xml=None, session_id=None):
        self._content = type(self).ContentType()
        if xml is not None:
            self.parseXml(xml)
        else:
            self._type = type(self).MessageType
            self._session_id = session_id
            self._code = None
            self._content = None

    def __str__(self):
        return super(IpcResponse, self).__str__() + " %s" % (str(self._content),)

    def parseXml(self, xml):
        #debug("RECEIVED XML: %s\n" % (xml, ))
        e_response = etree.fromstring(xml)
        if e_response.tag != "Response":
            raise IpcError("Invalid response")
        try:
            self._type = e_response.find("Type").text
        except AttributeError:
            raise IpcError("Missing message type")
        if self._type != self.MessageType:
            raise IpcError("Invalid message type")
        try:
            self._code = e_response.find("Code").text
        except AttributeError:
            raise IpcError("Missing response code")
        try:
            self._session_id = e_response.find("SessionID").text
        except AttributeError:
            debug("No session ID in response")
        self._content = type(self).ContentType(e_response)
        return e_response

    @property
    def type(self):
        return self._type

    @property
    def code(self):
        return self._code

    @property
    def content(self):
        return self._content

    @property
    def session_id(self):
        return self._session_id

    def getValue(self, path):
        """Path is a tuple: (objectID, objectInstanceID, resourceID, resourceInstanceID)"""
        return self._content.getValue(path)

    def serialize(self):
        # use derived MessageType
        try:
            content = self._content.getElement()
        except AttributeError:
            content = None
        return _serialize(self.Header, self._type, self._session_id, content)

    # TODO:
    #  - expose response paths
    #  - expose response results
    #  - expose labels

class IpcNotification(IpcResponse):
    """Base notification class - represents an asynchronous notification from an IPC service to an IPC client."""
    header = "Notification"

    # TODO: implementation of xml->model for Notifications

    # TODO:
    #  - expose execute parameters
    #  - expose changetype labels


def TElement(tag, text=None, tail=None, parent=None, attrib={}, **extra):
    """Helper function to create an etree.Element with text and tail.
    Ensure that text is a string.
    """
    element = etree.Element(tag, attrib, **extra)
    if text is not None:
        assert isinstance(text, str), "text must be of type str"
        element.text = text
    if tail:
        element.tail = tail
    if not parent == None:   # Issues warning if just 'if parent:'
        parent.append(element)
    return element


def validateModelPath(path, supportedPaths):
    """Model Paths represent path formats. Return true if the path is supported by the list of supportedPaths."""
    # validate path entirely by length:
    pathLengthMap = { "O": 1, "OI":2, "OIR":3, "OIRi":4 }
    res = False
    for item in supportedPaths:
        try:
            pathLen = pathLengthMap[item]
        except KeyError:
            raise IpcError("Invalid path spec: %s" % (item,))
        try:
            if len(path) == pathLen:
                res = True
                break
        except TypeError:   # catch paths with no len()
            continue
    return res



# Deserialization

class Child(object):
    def __init__(self, tags, type):
        self.tags = tags
        self.type = type

class Deserializer(object):
    IDTag = None
    valueTag = None
    children = None

class ResourceInstance(Deserializer):
    IDTag = "ID"
    valueTag = "Value"

class Resource(Deserializer):
    IDTag = "ID"
    children = Child( ("ResourceInstance",), ResourceInstance)
    valueTag = "Value"

class ObjectInstance(Deserializer):
    IDTag = "ID"
    children = Child( ("Resource",), Resource)

class Object(Deserializer):
    IDTag = "ID"
    children = Child( ("ObjectInstance",), ObjectInstance)

class Root(Deserializer):
    IDTag = None
    children = Child( ("Object",), Object)


# The OIRi model is a tree of nodes. Each node has zero or more child nodes, and an ID
# Nodes can be addressed by IDs.
class TreeNode(object):
    def __init__(self, ID=None, value=None):
        self._ID = ID
        self._value = value
        self._label = None
        self._children = {}

    def getID(self):
        return self._ID

    def setValue(self, value):
        self._value = value
    def getValue(self):
        return self._value

    def setLabel(self, label):
        self._label = label
    def getLabel(self):
        return self._label

    def addChild(self, child, ID):
        debug("Added child %s to parent %s with ID %s" % (child, self, ID))
        self._children[ID] = child
    def getChild(self, ID):
        return self._children.get(ID, None)
    def isLeaf(self):
        return self._children == False
    def getChildIDs(self):
        return self._children.keys()
    def numChildren(self):
        return len(self._children)
    def __str__(self):
        s = "[ (%x) ID %s, " % (id(self), self._ID)
        if self._value is not None:
            s += "value %s, " % (self._value,)
        if self._children:
            s += "children "
            for key, value in self._children.iteritems():
                s += str(key) + ":%x" % (id(value),) + " "
        s += "]"
        return s

def buildTree(element, deserializer):
    debug("\ndoDeserialize: %s" % (deserializer,))
    # get ID
    ID = element.find(deserializer.IDTag).text if deserializer.IDTag else None
    try:
        value = element.find(deserializer.valueTag).text if deserializer.valueTag else None
    except AttributeError:
        value = None
    node = TreeNode(ID, value)
    debug("Created %s for element %s" % (node, element.tag))
    # get children
    if deserializer.children:
        for path in deserializer.children.tags:
            e_children = element.findall(path)
            for e_child in e_children:
                child = buildTree(e_child, deserializer.children.type)
                node.addChild(child, child.getID())
    return node

def findLeaf(node, path):
    debug("findLeaf: node %s, path %s" % (node, str(path)))
    if len(path) == 0:
        debug("found")
        return node
    else:
        childID = str(path[0])
        child = node.getChild(childID)
        debug("childID %s, child %s" % (childID, child))
        return (findLeaf(child, path[1:]) if child is not None else None)

# traverse the tree, building nodes if required. Return the final node.
def buildPath(node, path):
    debug("buildPath: node %s, path %s" % (node, str(path)))
    if len(path) == 0:
        return node
    else:
        childID = path[0]
        child = node.getChild(childID)
        if child is None:
            child = TreeNode(childID, None)
            node.addChild(child, childID)
        return buildPath(child, path[1:])

# when sorting, treat None IDs as lowest (-1)
def sortKey(item):
    if item is None:
        return -1
    return int(item)

def walkTree(node, processor, context):
    #print("walkTree: node %s, context %s" % (node, str(context)))
    context = processor.process(context, node)
    # if node has no children, return
    if node.isLeaf():
        return
    else:
        # walk each child in sorted ID order (assume integer IDs)
        for ID in sorted(node.getChildIDs(), key=sortKey):
            walkTree(node.getChild(ID), processor, context)

class etreeProcessor(object):
    def __init__(self):
        pass

    def _createElement(self, treeNode, e_parent, tag, IDTag, valueTag=None):
        # create a new element based on treeNode, and add to parentElement
        e_element = etree.Element(tag)

        ID = treeNode.getID()
        if ID is not None:
            e_element.append(TElement(IDTag, str(ID)))
        else:
            # don't create elements for nodes that have invalid IDs
            return None

        if valueTag is not None:
            value = treeNode.getValue()
            if value is not None:
                e_element.append(TElement(valueTag, str(value)))

        label = treeNode.getLabel()
        if label is not None:
            e_element.append(etree.Element(label))

        debug("Adding %s to %s" % (e_element, e_parent))
        e_parent.append(e_element)
        return e_element

    def process(self, context, node):

        level, e_parent = context
        e_element = None

        debug("process: context %s, node %s" % (context, node))

        if level == 0:
            # no element created, advance to next level
            level = 1
        elif level == 1:
            # objects
            e_element = self._createElement(node, e_parent, "Object", "ID")
        elif level == 2:
            # object instances
            e_element = self._createElement(node, e_parent, "ObjectInstance", "ID")
        elif level == 3:
            # resources
            e_element = self._createElement(node, e_parent, "Resource", "ID", "Value")
        elif level == 4:
            # resource instances
            e_element = self._createElement(node, e_parent, "ResourceInstance", "ID", "Value")
        else:
            raise IpcError("Tree depth exceeds serializer support.")

        # modify context
        if e_element is not None:
            e_parent = e_element
            level += 1

        return (level, e_parent)

    def getElement(self):
        return self._root

class StringProcessor(object):
    def __init__(self, string=None):
        self._string = string and string or ""

    # context is an integer, representing tree depth
    def process(self, context, node):
        self._string += "  " * context + str(node) + "\n"
        return context + 1

    def getString(self):
        return self._string

class DataModel(object):
    """Tree-based OIRi data model."""

    def __init__(self, element=None):
        self._root = None
        if element is not None:
            self._build(element)

    def __str__(self):
        if self._root is None:
            s = "-"
        else:
            processor = StringProcessor("")
            walkTree(self._root, processor, 0)
            s = processor.getString()
        return s

    # build up the model with values
    def add(self, path, value=None, label=None):
        # expect (ObjectID, ObjectInstanceID, ResourceID, ResourceInstanceID), Value
        if self._root is None:
            self._root = TreeNode()
        leaf = buildPath(self._root, path)
        leaf.setValue(value)
        leaf.setLabel(label)

    def getElement(self):
        """Convert the data model into an Element."""
        node = etree.Element("Model")
        objects = etree.Element("Objects")
        node.append(objects)
        if self._root is not None:
            processor = etreeProcessor()
            walkTree(self._root, processor, (0, objects))
        return node

    def _build(self, element):
        """Convert an Element into the data model."""
        # jump over the Objects node
        element2 = element.find("Objects")
        # create tree
        self._root = buildTree(element2, Root)
        debug("root: %s\n" % (self._root,))

    def getValue(self, path):
        # expect (ObjectID, InstanceID, PropertyID, ValueID) path, return stored value
        assert len(path) in (3, 4), "getValue expects path to be a sequence of length 3 or 4"
        leaf = findLeaf(self._root, path)
        if leaf is not None:
            return leaf.getValue()
        return None



