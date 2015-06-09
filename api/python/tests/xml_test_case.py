import unittest
from lxml import etree

class XmlTestCase(unittest.TestCase):

    def assertEqualXml(self, a, b):
        # strip blank text from XML
        parser = etree.XMLParser(remove_blank_text=True)  # lxml.etree only!
        da = etree.XML(a, parser)
        db = etree.XML(b, parser)
        #da, db= etree.fromstring(a), etree.fromstring(b) # does not strip blank text
        try:
            self._is_equal_xml_element(da, db)
        except AssertionError:
            print("A:\n%s" % (str(a),))
            print("B:\n%s" % (str(b),))
            raise

    def _is_equal_xml_element(self, a, b):
        if a.tag != b.tag:
            raise AssertionError("Tag mismatch [%s]: a %s, b %s" % (a.tag, a.tag, b.tag))
        if sorted(a.attrib) != sorted(b.attrib):
            raise AssertionError("Attribute mismatch [%s]: a %s, b %s" % (a.tag, a.attrib, b.attrib))
        if len(a.getchildren()) != len(b.getchildren()):
            raise AssertionError("Children mismatch [%s]: a %d children, b %d children" % (a.tag, len(a.getchildren()), len(b.getchildren())))
        if a.text != b.text:
            raise AssertionError("Text mismatch [%s]: a %s, b %s" % (a.tag, a.text, b.text))
        for ac, bc in zip(a.getchildren(), b.getchildren()):
            self._is_equal_xml_element(ac, bc)
