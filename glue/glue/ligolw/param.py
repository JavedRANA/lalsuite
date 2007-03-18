# $Id$
#
# Copyright (C) 2006  Kipp C. Cannon
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


#
# =============================================================================
#
#                                   Preamble
#
# =============================================================================
#


"""
High-level support for Param elements.
"""


import re
import sys
from xml.sax.saxutils import escape as xmlescape

import ligolw
import types


__author__ = "Kipp Cannon <kipp@gravity.phys.uwm.edu>"
__date__ = "$Date$"[7:-2]
__version__ = "$Revision$"[11:-2]


#
# =============================================================================
#
#                           Param Name Manipulation
#
# =============================================================================
#


#
# Regular expression used to extract the signifcant portion of a param
# name, according to LIGO LW naming conventions.
#


ParamPattern = re.compile(r"(?:\A[a-z0-9_]+:|\A)(?P<Name>[a-z0-9_]+):param\Z")


def StripParamName(name):
	"""
	Return the significant portion of a param name according to LIGO LW
	naming conventions.
	"""
	try:
		return ParamPattern.search(name).group("Name")
	except AttributeError:
		return name


def CompareParamNames(name1, name2):
	"""
	Convenience function to compare two param names according to LIGO
	LW naming conventions.
	"""
	return cmp(StripParamName(name1), StripParamName(name2))


def getParamsByName(elem, name):
	"""
	Return a list of params with name name under elem.
	"""
	return elem.getElements(lambda e: (e.tagName == ligolw.Param.tagName) and (CompareParamNames(e.getAttribute("Name"), name) == 0))


#
# =============================================================================
#
#                                  Utilities
#
# =============================================================================
#


def new_param(name, type, value, comment = None):
	"""
	Construct a LIGO Light Weight XML Param document subtree.
	"""
	elem = Param({u"Name": u"%s:param" % name, u"Type": type})
	elem.pcdata = value
	if comment is not None:
		elem.appendChild(ligolw.Comment())
		elem.childNodes[-1].pcdata = comment
	return elem


def get_param(xmldoc, name):
	"""
	Scan xmldoc for a param named name.  Raises ValueError if not
	exactly 1 such param is found.
	"""
	params = getParamsByName(xmldoc, name)
	if len(params) != 1:
		raise ValueError, "document must contain exactly one %s param" % StripParamName(name)
	return params[0]


#
# =============================================================================
#
#                               Element Classes
#
# =============================================================================
#


#
# FIXME: params of type string should be quoted in order to correctly
# delimit their extent.  If that were done, then the pcdata in a Param
# element could be parsed using the Stream tokenizer (i.e., as though it
# were a single-token stream), which would guarantee that Stream data and
# Param data is parsed using the exact same rules.  Unfortunately, common
# practice is to not quote Param string values, so we parse things
# differently here.  In particular, we strip whitespace from the start and
# stop of all Param pcdata.  If this causes your string Param values to be
# corrupted (because you need leading and trailing white space preserved),
# then you need to make everyone switch to quoting their string Param
# values, and once that is done then this code will be changed.  Perhaps a
# warning should be emitted for non-quoted strings to encourage a
# transition?
#


class Param(ligolw.Param):
	"""
	High-level Param element.  The parsed value is stored in the pcdata
	attribute.
	"""
	def __init__(self, *attrs):
		"""
		Initialize a new Param element.
		"""
		ligolw.Param.__init__(self, *attrs)
		try:
			t = self.getAttribute("Type")
		except KeyError:
			# default
			t = u"lstring"
		self.pytype = types.ToPyType[t]

	def write(self, file = sys.stdout, indent = u""):
		file.write(self.start_tag(indent) + u"\n")
		for c in self.childNodes:
			if c.tagName not in self.validchildren:
				raise ElementError, "invalid child %s for %s" % (c.tagName, self.tagName)
			c.write(file, indent + ligolw.Indent)
		if self.pcdata:
			# FIXME:  does this satisfactorily preserve precision
			# in floating point values?
			file.write(indent + ligolw.Indent)
			file.write(xmlescape(unicode(self.pcdata)))
			file.write(u"\n")
		file.write(self.end_tag(indent) + u"\n")


#
# =============================================================================
#
#                               Content Handler
#
# =============================================================================
#


#
# Override portions of ligolw.LIGOLWContentHandler class
#


def startParam(self, attrs):
	return Param(attrs)


def endParam(self):
	self.current.pcdata = self.current.pytype(self.current.pcdata.strip())


ligolw.LIGOLWContentHandler.startParam = startParam
ligolw.LIGOLWContentHandler.endParam = endParam
