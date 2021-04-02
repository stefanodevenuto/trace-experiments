#!/bin/env python3

import sys
import tracecruncher.utils as tc

class Record:
	def __init__(self, ts, event, cpu, pid, offset):
		self.ts       = ts
		self.event    = event
		self.cpu      = cpu
		self.pid      = pid
		self.offset   = offset

def print_usage():
	print("Trace files not specified")
	print("Usage: python3 check.py <trace_file_path>")

def load_records(records, all_records):
	for i in range(0, tc.size(records)):
		rec = Record(records["time"][i], records["event"][i], records["cpu"][i],
					 records["pid"][i], records["offset"][i])
		all_records.append(rec)

def print_records(all_records):
	for i in range(0, len(all_records)):
		print(f"{all_records[i].ts} {all_records[i].event} {all_records[i].cpu} \
			    {all_records[i].pid} {all_records[i].offset}")

if __name__ == "__main__":
	if len(sys.argv) < 2:
		print_usage()
		exit()
	"""
	fname = str(sys.argv[1])
	f = tc.open_file(file_name=fname)

	records = f.load()

	for i in range(0, len(records["event"])):
		print(records["time"][i])
	"""

	all_records = []

	#for i in range(1, len(sys.argv)):
	fname = str(sys.argv[1])
	fname1 = str(sys.argv[2])

	f = tc.open_file(file_name=fname)
	f1 = tc.open_file(file_name=fname1)

	records = f.load()
	records1 = f1.load()
		#for i in range(0, tc.size(records)):
		#all_records.append(records)

	load_records(records, all_records)
	load_records(records1, all_records)

	print_records(all_records)
	

"""
def merge_trace_files(files):
	records_files = []
	result = []

	n_entries = 0
	for f in files:
		records = f.load()
		records_files.append(records)

		n_entries = n_entries + len(records["event"])

	for j in range(0, n_entries):
		min_time 	 = sys.maxsize
		min_event_id = ""
		min_file 	 = ""
		for i in range(0, len(records_files)):
			records = records_files[0]
			if records["time"][i] < min_time:
				min_time 	 = records["time"][i]
				min_event_id = records["event"][i]
				min_file 	 = i

		print(j)

		event_name = files[min_file].event_name(event_id=min_event_id)

		#print("%d: [%d] %s" % min_file, min_time, event_name)

		records_files[min_file].pop(0)


if __name__ == "__main__":
	if len(sys.argv) < 2:
		print_usage()
		exit()

	files = []

	for i in range(1, len(sys.argv)):
		fname = str(sys.argv[i])
		f = tc.open_file(file_name=fname)

		files.append(f)

	merge_trace_files(files)

"""
"""
	fname = str(sys.argv[1])
	f = tc.open_file(file_name=fname)

	records = f.load()

	for i in range(0, len(records["event"])):
		print(records["time"][i])


	for i in range(1, len(sys.argv)):
		fname = str(sys.argv[i])
		f = tc.open_file(file_name=fname)

		open_files.append(f)

	kvm_entry_id = f.event_id(name='kvm/kvm_entry')
	kvm_exit_id = f.event_id(name='kvm/kvm_exit')

	last_record = None

	for i in range(0, n_entries):
	#while last_record != None:
		for file in open_files:
			if records_files["event"][i] == kvm_entry_id:
				print(file.read_event_field(offset=records["offset"][i], event_id=kvm_entry_id, field='vcpu'))

"""