# $Id$
#
# Copyright (C) 2008  Kipp C. Cannon
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
Ask Kipp to document this!
"""


from glue import iterutils
from glue import segments
from glue import segmentsUtils
from glue.ligolw import table
from glue.ligolw import lsctables
from pylal.date import LIGOTimeGPS


__author__ = "Kipp Cannon <kipp@gravity.phys.uwm.edu>"
__version__ = "$Revision$"[11:-2]
__date__ = "$Date$"[7:-2]


#
# =============================================================================
#
#                                 Segment List
#
# =============================================================================
#


def segment_def(instruments, name, comment, segment_definer_table, process):
	"""
	Create and initialize a segment_definer row object.
	"""
	row = segment_definer_table.RowType()
	row.process_id = process.process_id
	row.segment_def_id = segment_definer_table.get_next_id()
	instruments = list(instruments)
	instruments.sort()
	row.ifos = ",".join(instruments)
	row.name = name
	row.comment = comment
	return row


def seg_def_map(segment_row, segment_def_row, seg_def_map_table, process):
	"""
	Create and initialize a segment_def_map row object.
	"""
	row = seg_def_map_table.RowType()
	row.process_id = process.process_id
	row.segment_id = segment_row.segment_id
	row.segment_def_id = segment_def_row.segment_def_id
	row.seg_def_map_id = seg_def_map_table.get_next_id()
	return row


class LigolwSegmentList(object):
	"""
	A description of a class of segments.
	"""
	def __init__(self, active = (), inactive = (), instruments = set(), name = None, comment = None):
		self.active = segments.segmentlist(active)
		self.inactive = segments.segmentlist(inactive)
		self.instruments = instruments
		self.name = name
		self.comment = comment

	def sort(self, *args):
		"""
		Sort the internal segment lists.  The optional args are
		passed to the .sort() method of the segment lists.  This
		can be used to control the sort order by providing an
		alternate comparison function.  The default is to sort by
		start time with ties broken by end time.
		"""
		self.active.sort(*args)
		self.inactive.sort(*args)

	def coalesce(self):
		"""
		Coalesce the internal segment lists.
		"""
		self.active.coalesce()
		self.inactive.coalesce()


#
# =============================================================================
#
#                                 Library API
#
# =============================================================================
#


class LigolwSegments(object):
	"""
	A high-level interface to the segments tables in a LIGO Light
	Weight XML document.
	"""
	def __init__(self, xmldoc):
		#
		# Find tables, and synchronize ID generators
		#

		try:
			self.segment_def_table = table.get_table(xmldoc, lsctables.SegmentDefTable.tableName)
		except ValueError:
			self.segment_def_table = xmldoc.childNodes[0].appendChild(lsctables.New(lsctables.SegmentDefTable, ("process_id", "segment_def_id", "ifos", "name", "comment")))

		try:
			self.segment_table = table.get_table(xmldoc, lsctables.SegmentTable.tableName)
		except ValueError:
			self.segment_table = xmldoc.childNodes[0].appendChild(lsctables.New(lsctables.SegmentTable, ("process_id", "segment_id", "start_time", "start_time_ns", "end_time", "end_time_ns", "active")))

		try:
			self.segment_def_map_table = table.get_table(xmldoc, lsctables.SegmentDefMapTable.tableName)
		except ValueError:
			self.segment_def_map_table = xmldoc.childNodes[0].appendChild(lsctables.New(lsctables.SegmentDefMapTable, ("process_id", "segment_id", "segment_def_id", "seg_def_map_id")))

		#
		# Transform segment tables into a collection of
		# LigolwSegmentList objects for more convenient
		# manipulation
		#

		# construct empty LigolwSegmentList objects, one for each
		# entry in the segment_definer table, indexed by
		# segment_definer id
		self.segment_lists = dict((row.segment_def_id, LigolwSegmentList(instruments = row.get_ifos(), name = row.name, comment = row.comment)) for row in self.segment_def_table)
		if len(self.segment_lists) != len(self.segment_def_table):
			raise ValueError, "duplicate segment_definer IDs detected in segment_definer table"
		del self.segment_def_table[:]

		# index the segment table
		index = dict((row.segment_id, row) for row in self.segment_table)
		if len(index) != len(self.segment_table):
			raise ValueError, "duplicated segment IDs detected in segment table"
		del self.segment_table[:]

		# populate LigolwSegmentList objects from segment_def_map
		# table and segment_table index
		for row in self.segment_def_map_table:
			segment_row = index[row.segment_id]
			active = segment_row.get_active()
			if active is True:
				self.segment_lists[row.segment_def_id].active.append(segment_row.get())
			elif active is False:
				self.segment_lists[row.segment_def_id].inactive.append(segment_row.get())
			else:
				raise ValueError, "invalid activity flag '%s' for segment '%s'" % (repr(row.activity), str(row.segment_id))
		del self.segment_def_map_table[:]
		del index

		# replace segment_lists dictionary with a list
		self.segment_lists = self.segment_lists.values()

		#
		# Synchronize ID generators
		#

		self.segment_def_table.sync_next_id()
		self.segment_table.sync_next_id()
		self.segment_def_map_table.sync_next_id()

		#
		# Done
		#


	def coalesce(self):
		"""
		Coalesce the segment lists.
		"""
		for ligolw_segment_list in self.segment_lists:
			ligolw_segment_list.coalesce()


	def sort(self, *args):
		"""
		Sort the segment lists.  The optional args are passed to
		the .sort() methods of the segment lists.  This can be used
		to control the sort order by providing an alternate
		comparison function (the default is to sort all lists by
		segment start time with ties broken by end time).
		"""
		for ligolw_segment_list in self.segment_lists:
			ligolw_segment_list.sort(*args)


	def optimize(self):
		"""
		Identifies segment lists that differ only in their
		instruments --- they have the same active and inactive
		segments, the same name and the same comment --- and then
		deletes all but one of them, leaving just a single list
		having the union of the instruments.
		"""
		self.sort()
		segment_lists = dict(enumerate(self.segment_lists))
		for target, source in [(idx_a, idx_b) for (idx_a, seglist_a), (idx_b, seglist_b) in iterutils.choices(segment_lists.items(), 2) if seglist_a.active == seglist_b.active and seglist_a.inactive == seglist_b.inactive and seglist_a.name == seglist_b.name and seglist_a.comment == seglist_b.comment]:
			try:
				segment_lists[target].instruments |= segment_lists.pop(source).instruments
			except KeyError:
				pass
		self.segment_lists = segment_lists.values()


	def finalize(self, process):
		"""
		Restore the LigolwSegmentList objects to the XML tables in
		preparation for output.  All segments from all segment
		lists are inserted into the tables in time order, but this
		is NOT behaviour external applications should rely on.
		This is done simply in the belief that it might assist in
		constructing well balanced indexed databases from the
		resulting files.  If that proves not to be the case, or for
		some reason this behaviour proves inconvenient to preserve,
		then it might be discontinued without notice.  You've been
		warned.
		"""
		def time_order_rows(ligolw_segment_list, segment_table, process, segment_def_row):
			ligolw_segment_list.sort()
			for seg, activity in iterutils.inorder(((seg, True) for seg in ligolw_segment_list.active), ((seg, False) for seg in ligolw_segment_list.inactive)):
				segment_row = segment_table.RowType()
				segment_row.set(seg)
				segment_row.set_active(activity)
				segment_row.process_id = process.process_id
				segment_row.segment_id = segment_table.get_next_id()
				yield segment_row, segment_def_row

		row_iterators = []
		for ligolw_segment_list in self.segment_lists:
			segment_def_row = segment_def(ligolw_segment_list.instruments, ligolw_segment_list.name, ligolw_segment_list.comment, self.segment_def_table, process)
			self.segment_def_table.append(segment_def_row)
			row_iterators.append(time_order_rows(ligolw_segment_list, self.segment_table, process, segment_def_row))

		for segment_row, segment_def_row in iterutils.inorder(*row_iterators):
			self.segment_table.append(segment_row)
			self.segment_def_map_table.append(seg_def_map(segment_row, segment_def_row, self.segment_def_map_table, process))

		del self.segment_lists[:]


#
# =============================================================================
#
#                                  Utilities
#
# =============================================================================
#


def insert_from_segwizard(ligolw_segments, fileobj, instruments, name, comment):
	"""
	Parse the contents of the file object fileobj as a segwizard-format
	segment list, and insert the result as a new list of "active"
	segments into the LigolwSegments object ligolw_segments.  A new
	entry will be created in the segment_definer table for the segment
	list, and instruments, name and comment are used to populate the
	entry's metadata.
	"""
	ligolw_segments.segment_lists.append(LigolwSegmentList(active = segmentsUtils.fromsegwizard(fileobj, coltype = LIGOTimeGPS), instruments = instruments, name = name, comment = comment))
