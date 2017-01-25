import elephant
import neo
import numpy as np
import quantities as pq

import nest


def block_from_device(device, block=None):
    if block is None:
        block = neo.Block()
    block.segments.append(segment_from_device(device))
    return block


def segment_from_device(device):
    sgmt = neo.Segment()
    sgmt.spiketrains.extend(from_device(device))
    return sgmt


def from_device(device):
    to_memory = nest.GetStatus(device, 'to_memory')
    to_file = nest.GetStatus(device, 'to_file')

    if not to_memory and not to_file:
        raise ValueError('sadasdasd')

    if to_memory:
        events = nest.GetStatus(device, 'events')[0]
        senders = events['senders']
        times = events['times']

        st = []
        for gid in np.unique(senders):
            x = neo.SpikeTrain(times[senders == gid] * pq.ms, t_start=0. * pq.ms, t_stop=nest.GetKernelStatus('time') * pq.ms)
            x.annotate(gid=gid, sampling_period=nest.GetKernelStatus('resolution'))
            st.append(x)

        return st