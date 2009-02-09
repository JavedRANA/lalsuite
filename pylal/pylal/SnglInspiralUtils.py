# $Id$
# Copyright (C) 2006  Duncan A. Brown
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

import copy

from pylal import date
from pylal import SearchSummaryUtils
from glue.ligolw import ligolw
from glue.ligolw import table
from glue.ligolw import lsctables
from glue.ligolw import utils
from glue.ligolw.utils import ligolw_add
from glue import iterutils
from glue import segments

#
# =============================================================================
#
#                                   Input
#
# =============================================================================
#


def ReadSnglInspiralFromFiles(fileList, mangle_event_id=False, verbose=False, non_lsc_tables_ok=False):
  """
  Read the SnglInspiralTables from a list of files

  @param fileList: list of input files
  @param mangle_event_id: ID remapping is necessary in cases where multiple
    files might have event_id collisions (ex: exttrig injections)
  @param verbose: print ligolw_add progress
  """
  # turn on ID remapping if necessary
  if mangle_event_id:
    lsctables.SnglInspiralTable.next_id = lsctables.SnglInspiralID_old(0)

  # ligolw_add will merge all tables, which is overkill, but merge time is
  # much less than I/O time.
  xmldoc = ligolw_add.ligolw_add(ligolw.Document(), fileList, non_lsc_tables_ok=non_lsc_tables_ok,  verbose=verbose)

  # extract the SnglInspiral table
  try:
    snglInspiralTriggers = table.get_table(xmldoc, \
      lsctables.SnglInspiralTable.tableName)
  except:
    snglInspiralTriggers = None

  # return ID remapping to its normal state (off)
  lsctables.SnglInspiralTable.next_id = None

  return snglInspiralTriggers


def ReadSnglInspiralSlidesFromFiles(fileList, shiftVector, vetoFile=None,
  mangleEventId=False, verbose=False, non_lsc_tables_ok=False):
  """
  Function for reading time-slided single inspiral triggers
  with automatic resliding the times, given a list of input files.

  @param fileList: List of files containing single inspiral time-slided
                   triggers
  @param shiftVector: Dictionary of time shifts to apply to triggers
                      keyed by IFO  
  @param vetoFile: segwizard formatted file used to veto all triggers
  @param mangleEventId: ID remapping is necessary in cases where multiple
    files might have event_id collisions (ex: exttrig injections)
  @param verbose: print ligolw_add progress
  """

  # read raw triggers
  inspTriggers = ReadSnglInspiralFromFiles(\
    fileList, verbose=verbose, mangle_event_id=mangleEventId, non_lsc_tables_ok=non_lsc_tables_ok )
  if inspTriggers:
    # get the rings
    segDict = SearchSummaryUtils.GetSegListFromSearchSummaries(fileList)
    rings = segments.segmentlist(iterutils.flatten(segDict.values()))
    rings.sort()
    for i,ring in enumerate(rings):
      rings[i] = segments.segment(rings[i][0], rings[i][1] + 10**(-9))

    # perform the veto
    if vetoFile is not None:
      segList = segmentsUtils.fromsegwizard(open(vetoFile))
      inspTriggers = inspTriggers.veto(segList)

    # now slide all the triggers within their appropriate ring
    slideTriggersOnRingWithVector(inspTriggers, shiftVector, rings)

  # return the re-slided triggers
  return inspTriggers


#
# =============================================================================
#
#                                  Clustering
#
# =============================================================================
#

def CompareSnglInspiralByEndTime(a, b):
  """
  Orders a and b by peak time.
  """
  return cmp(a.get_end(), b.get_end())


def CompareSnglInspiralBySnr(a, b):
  """
  Orders a and b by peak time.
  """
  return cmp(a.snr, b.snr)


def CompareSnglInspiral(a, b, twindow = date.LIGOTimeGPS(0)):
  """
  Returns 0 if a and b are less than twindow appart.
  """
  tdiff = abs(a.get_end() - b.get_end())
  if tdiff < twindow:
    return 0
  else:
    return cmp(a.get_end(), b.get_end())

#
# =============================================================================
#
#                              Time Sliding on Ring
#
# =============================================================================
#

def slideTimeOnRing(time, shift, ring):
  """
  Return time after adding shift, but constrained to lie along the ring
  """
  assert time in ring
  # use ring[0] as an epoch, do arithmetic using floats relative to epoch
  return ring[0] + (float(time - ring[0]) + shift) % float(abs(ring))


def slideTriggersOnRings(triggerList, rings, shifts):
  """
   In-place modify trigger_list so that triggers are slid by appropriate value
   of shifts along their enclosing ring segment by the algorithm given in XXX.
   This function calls the function slideTimeOnRing

   @param triggerList: a SnglInspiralTable
   @param rings:       sorted segment list of possible rings
   @param shifts:      a dictionary of the time-shifts keyed by IFO
  """
  for trigger in triggerList:
    end_time = trigger.get_end()
    trigger.set_end(slideTimeOnRing(end_time, shifts[trigger.ifo], rings[rings.find(end_time)]))

def unslideTriggersOnRings(triggerList, rings, shifts):
  """
   In-place modify trigger_list so that triggers are unslid by appropriate
   value of shifts along their enclosing ring segment by the algorithm given in
   XXX.
   This function calls the function slideTriggersOnRing

   @param triggerList: a SnglInspiralTable
   @param rings:       sorted segment list of possible rings
   @param shifts:      a dictionary of the time-shifts keyed by IFO
  """
  slideTriggersOnRings(triggerList, rings, dict((ifo, -shift) for ifo, shift in shifts.items()))

def slideTriggersOnRingWithVector(triggerList, shiftVector, rings):
   """
   In-place modify trigger_list so that triggers are slid by
   along their enclosing ring segment by the algorithm given in XXX.
   Slide numbers are extracted from the event_id of each trigger,
   and multiplied by the corresponding (ifo-keyed) entry in shift_vector
   to get the total slide amount.
   This function is called by ReadSnglInspiralSlidesFromFiles and
   calls the function slideTimeOnRing

   @param triggerList: a SnglInspiralTable
   @param shiftVector: a dictionary of the unit time-shift vector,
                       keyed by IFO
   @param rings:       sorted segment list of possible rings
   """
   for trigger in triggerList:
     end_time = trigger.get_end()
     trigger.set_end(slideTimeOnRing(end_time, trigger.get_slide_number() * shiftVector[trigger.ifo], rings[rings.find(end_time)]))

def slideSegListDictOnRing(ring, seglistdict, shifts):
  """
   Return seglistdict with segments that are slid by appropriate values of
   shifts along the ring segment by the algorithm given in XXX.

   @param ring:        segment on which to cyclicly slide segments in
                       seglistdict
   @param seglistdict: segments to be slid on ring
   @param shifts:      a dictionary of the time-shifts keyed by IFO
  """
  # don't do this in loops
  ring_duration = float(abs(ring))
  
  # automate multi-list arithmetic
  ring = segments.segmentlistdict.fromkeys(seglistdict.keys(), segments.segmentlist([ring]))

  # make a copy, and extract the segments that are in the ring.
  seglistdict = seglistdict & ring

  # normalize the shift vector.  after this all the shifts are non-negative 	 
  # and less than the duration of the ring. 	 
  shifts = dict((instrument, shift % ring_duration) for instrument, shift in shifts.items()) 	 
  	 
  # apply the shift vector. 	 
  seglistdict.offsets.update(shifts) 	 
  	 
  # split the result into the pieces that are still in the ring, and the 	 
  # pieces that have fallen off the edge.  both retain the shift vector in 	 
  # the offsets attribute. 	 
  extra = seglistdict - ring 	 
  seglistdict &= ring 	 
  	 
  # wrap the fallen-off pieces around the ring. 	 
  for instrument in extra.keys(): 	 
    extra.offsets[instrument] -= ring_duration 	 
  	 
  # return the union of the results.  seglistdict's shift vector is 	 
  # retained in the offests attribute. 	 
  return seglistdict | extra 	 

