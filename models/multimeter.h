/*
 *  multimeter.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MULTIMETER_H
#define MULTIMETER_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "recording_device.h"
#include "sibling_container.h"

// Includes from sli:
#include "dictutils.h"
#include "name.h"

namespace nest
{

/** @BeginDocumentation
@ingroup Devices
@ingroup detector

Name: multimeter - Device to record analog data from neurons.

Synopsis: multimeter Create

Description:

A multimeter records a user-defined set of state variables from connected nodes
to memory, file or stdout.

The multimeter must be configured with the list of variables to record
from, otherwise it will not record anything. The /recordables property
of a neuron model shows which quantities can be recorded with a multimeter.
A single multimeter should only record from neurons of the same basic
type (e.g. /iaf_cond_alpha and any user-defined models derived from it
using CopyModel). If the defaults or status dictionary of a model neuron
does not contain a /recordables entry, it is not ready for use with
multimeter.

By default, multimeters record values once per ms. Set the parameter /interval
to change this. The recording interval cannot be smaller than the resolution.

Results are returned in the /events entry of the status dictionary. For
each recorded quantity, a vector of doubles is returned. The vector has the
same name as the /recordable.

Remarks:
- The set of variables to record and the recording interval must be set
  BEFORE the multimeter is connected to any node, and cannot be changed
  afterwards.
- A multimeter cannot be frozen.

@note If you want to pick up values at every time stamp,
  you must set the interval to the simulation resolution.
@see UniversalDataLogger


Parameters:

The following parameters can be set in the status dictionary:

\verbatim embed:rst
============  ======  ===================================================
 interval     ms      Recording interval
 record_from  array   Array containing the names of variables to record
                      from, obtained from the /recordables entry of the
                      model from which one wants to record
============  ======  ===================================================
\endverbatim

Examples:

    SLI ] /iaf_cond_alpha Create /n Set
    SLI ] n /recordables get ==
    [/V_m /g_ex /g_in /t_ref_remaining]
    SLI ] /multimeter Create /mm Set
    SLI ] mm << /interval 0.5 /record_from [/V_m /g_ex /g_in] >> SetStatus
    SLI ] mm n Connect
    SLI ] 10 Simulate
    SLI ] mm /events get info
    --------------------------------------------------
    Name                     Type                Value
    --------------------------------------------------
    g_ex                     doublevectortype    <doublevectortype>
    g_in                     doublevectortype    <doublevectortype>
    senders                  intvectortype       <intvectortype>
    times                    doublevectortype    <doublevectortype>
    t_ref_remaining          doublevectortype    <doublevectortype>
    V_m                      doublevectortype    <doublevectortype>
    rate                     doublevectortype    <doublevectortype>
    --------------------------------------------------
    Total number of entries: 6


Sends: DataLoggingRequest

FirstVersion: 2009-04-01

Author: Hans Ekkehard Plesser, Barna Zajzon (added offset support March 2017)

SeeAlso: Device, RecordingDevice
*/

class Multimeter : public RecordingDevice
{

public:
  Multimeter();
  Multimeter( const Multimeter& );

  /**
   * @note Multimeters never have proxies, since they must
   *       sample their targets through local communication.
   */
  bool
  has_proxies() const
  {
    return false;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_signal;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( DataLoggingReply& );

  SignalType sends_signal() const;

  Type get_type() const;
  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

protected:
  void init_state_( Node const& );
  void init_buffers_();
  void calibrate();

  /**
   * Collect and output membrane potential information.
   * This function pages all its targets at all pertinent sample
   * points for membrane potential information and then outputs
   * that information. The sampled nodes must provide data from
   * the previous time slice.
   */
  void update( Time const&, const long, const long );

private:
  /**
   * "Print" one value to file or screen, depending on settings in
   * RecordingDevice.
   * @note The default implementation supports only EntryTypes which
   *       RecordingDevice::print_value() can handle. Otherwise, specialization
   *       is required.
   */
  void print_value_( const std::vector< double >& );

  /**
   * Add recorded data to dictionary.
   * @note By default, only implemented for EntryType double, must
   *       otherwise be specialized.
   * @param /events dictionary to be placed in properties dictionary
   */
  void add_data_( DictionaryDatum& ) const;

  // ------------------------------------------------------------

  struct Buffers_;

  struct Parameters_
  {
    Time interval_; //!< recording interval, in ms
    Time offset_;   //!< offset relative to which interval is calculated, in ms
    std::vector< Name > record_from_; //!< which data to record

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Buffers_& );
  };

  // ------------------------------------------------------------

  struct State_
  {
    /** Recorded data.
     * First dimension: time
     * Second dimension: recorded variables
     * @note In normal mode, data is stored as follows:
     *          For each recorded node, all data points for one time slice are
     *          put after one another in the first dimension. Each entry is a
     *          vector containing one element per recorded quantity.
     *       In accumulating mode, only one data point is stored per time step
     *          and values are added across nodes.
     */
    std::vector< std::vector< double > > data_; //!< Recorded data
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    /** Does this multimeter have targets?
     * Placed here since it is implementation detail.
     * @todo Ideally, one should be able to ask ConnectionManager.
     */
    Buffers_();

    bool has_targets_;
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
};


inline void
nest::Multimeter::get_status( DictionaryDatum& d ) const
{
  // get the data from the device
  RecordingDevice::get_status( d );

  // if we are the device on thread 0, also get the data from the
  // siblings on other threads
  if ( get_thread() == 0 )
  {
    const SiblingContainer* siblings =
      kernel().node_manager.get_thread_siblings( get_gid() );
    std::vector< Node* >::const_iterator sibling;
    for ( sibling = siblings->begin() + 1; sibling != siblings->end();
          ++sibling )
    {
      ( *sibling )->get_status( d );
    }
  }

  P_.get( d );
}

inline void
nest::Multimeter::set_status( const DictionaryDatum& d )
{
  // protect Multimeter from being frozen
  bool freeze = false;
  if ( updateValue< bool >( d, names::frozen, freeze ) && freeze )
  {
    throw BadProperty( "Multimeter cannot be frozen." );
  }

  Parameters_ ptmp = P_;
  ptmp.set( d, B_ );

  RecordingDevice::set_status( d );

  P_ = ptmp;
}

inline SignalType
nest::Multimeter::sends_signal() const
{
  return ALL;
}

} // Namespace

#endif
