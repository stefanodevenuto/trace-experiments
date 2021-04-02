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

	all_records = []

	"""
	If a loop over the two files is done, an error from trace-seq is raised,
	saying that the trace_seq is used after it was destoyed.
	This is true because the same one is used for every file.
	The right way to do it I think would be to implement the kshark function
	`kshark_load_all_entries`.
	"""
	fname = str(sys.argv[1])
	fname1 = str(sys.argv[2])

	f = tc.open_file(file_name=fname)
	f1 = tc.open_file(file_name=fname1)

	records = f.load()
	records1 = f1.load()

	load_records(records, all_records)
	load_records(records1, all_records)

	print_records(all_records)
	