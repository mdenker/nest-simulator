import matplotlib.pyplot as plt

import numpy as np
import quantities as pq

import elephant
import neo

from neo_bridge import block_from_device, segment_from_device, from_device
import nest


nest.ResetKernel()
# nest.SetKernelStatus({'overwrite_files': True})
nest.SetKernelStatus({'resolution': 0.01})

print "Status of NEST Kernel before simulation"
ks_before = nest.GetKernelStatus()
for s in ks_before:
    print(s, ": ", ks_before[s])
print()

neuron = nest.Create('iaf_psc_delta', 2)
spikedetector = nest.Create('spike_detector')
poisson = nest.Create('poisson_generator', 1, {'rate': 16000.})

print("IDs of devices")
print neuron, spikedetector, poisson
print()

# nest.SetStatus(neuron, {'I_e': 300.})
# nest.SetStatus(spikedetector, {'to_file': True, 'to_memory': False})

nest.Connect(neuron, spikedetector)
nest.Connect(poisson, neuron)

nest.Simulate(500.)

print("Changes of status of NEST Kernel after simulation")
ks_after = nest.GetKernelStatus()
for s in ks_after:
    if ks_before[s] != ks_after[s]:
        print(s, ": ", ks_before[s], "->", ks_after[s])
print()

spike_trains = from_device(spikedetector)

print("Status of Neuron")
neus = nest.GetStatus(neuron)[0]
for s in neus:
    print(s, ": ", neus[s])
print()


print("Status of Spike Detector")
spds = nest.GetStatus(spikedetector)[0]
for s in spds:
    print(s, ": ", spds[s])
print()


# my_block = block_from_device(spikedetector)
# spike_trains = my_block.filter(gid=2, object=neo.SpikeTrain)

# plt.plot(np.arange(spike_trains[0].t_start, spike_trains, 0.1

# plt_time_unit="ms"
# ir = elephant.statistics.instantaneous_rate(spike_trains[0], spike_trains[0].annotations['sampling_period'] * pq.ms, kernel=elephant.kernels.GaussianKernel(sigma=30. * pq.ms, invert=False))
# plt.plot(ir.times.rescale(plt_time_unit),ir.rescale("Hz"))
# plt.xlabel(plt_time_unit)
# plt.ylabel('Hz')
# plt.show()
