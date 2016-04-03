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
