#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>

#include "trace-cmd.h"
#include "trace-cmd-private.h"
#include "trace-local.h"
#include "trace-cmd-private.h"
#include "event-parse.h"
#include "event-parse-local.h"
#include "sorting.h"
#include "dyn_array.h"

#define KVM_ENTRY "kvm_entry"
#define KVM_EXIT "kvm_exit"

#define MAX_LEN 1024

struct custom_record {
  struct custom_handle* handle;
  struct tep_handle* tep_handle;
  struct tep_record* record;
};

struct custom_handle {
  struct tracecmd_input* original_handle;
  char* filename;
  int* cpus;
};

int compare_ts(struct custom_record* rec1, struct custom_record* rec2) {
  if (rec1->record->ts < rec2->record->ts) {
    return -1;
  } else if(rec1->record->ts > rec2->record->ts) {
    return 1;
  } else {
    return 0;
  }
}

void print_records(DynArray* all_records) {
  struct tep_event* event;
  struct custom_record* current;

  for (int i = 0; i < DynArray_size(all_records); i++) {
    current = DynArray_get(all_records, i);
    event = tep_find_event_by_record(current->tep_handle, current->record);

    printf("%lX: [%03d]%lld: %s\n",
      tracecmd_get_traceid(current->handle->original_handle),
      current->record->cpu,
      current->record->ts,
      event->name);
  }
}

void free_data(DynArray* all_records, struct custom_handle** custom_handles, int n_handles) {
  for (int i = 0; i < n_handles; i++) {
    struct custom_handle* custom_handle = custom_handles[i];

    tracecmd_close(custom_handle->original_handle);
    free(custom_handle->filename);
    free(custom_handle->cpus);

    free(custom_handle);
  }

  for (int i = 0; i < DynArray_size(all_records); i++) {
    struct custom_record* current = DynArray_get(all_records, i);

    free(current);
  }

  DynArray_free(all_records);
}

int main(int argc, char const *argv[]){
  DynArray* all_records;
  struct tracecmd_input* handle;
  struct tep_record* record;
  struct tep_event* event;
  struct custom_record* current;
  struct custom_handle** custom_handles;
  struct custom_handle* custom_handle;
  int cpu;

  if (argc < 1) {
    printf("ERROR: Indicate at least 1 trace file\n");
    return 1;
  }

  all_records = DynArray_new();
  custom_handles = malloc(sizeof(*custom_handles) * (argc-1));

  for (int i = 1; i < argc; i++) {
    handle = tracecmd_open(argv[i], 0);

    custom_handle = malloc(sizeof(*custom_handle));
    custom_handle->original_handle = handle;
    custom_handle->filename = strndup(argv[i], MAX_LEN);
    custom_handle->cpus = calloc(tracecmd_cpus(handle), sizeof(int));

    custom_handles[i-1] = custom_handle;

    while ((record = tracecmd_read_next_data(handle, &cpu)) != NULL) {

      current = malloc(sizeof(*current));

      current->handle = custom_handle;
      current->tep_handle = tracecmd_get_tep(handle);
      current->record = record;

      DynArray_insert(all_records, current);
    }
  }

  quick_sort(DynArray_get_raw_array(all_records), DynArray_size(all_records), (SortingCmp) compare_ts);

  //print_records(all_records);

  for (int i = 0; i < DynArray_size(all_records); i++) {
    current = DynArray_get(all_records, i);
    event = tep_find_event_by_record(current->tep_handle, current->record);

    record = current->record;

    if (!strcmp(event->name, KVM_ENTRY)) {
      if (current->handle->cpus[record->cpu]) {
        printf("ERROR: %s: double kvm_entry on CPU %03d at %lld\n",
          current->handle->filename,
          record->cpu,
          record->ts);
        return 1;
      }

      current->handle->cpus[record->cpu] = 1;
    }

    if (!strcmp(event->name, KVM_EXIT)) {
      if (!current->handle->cpus[record->cpu]) {
        printf("ERROR: %s: double kvm_exit on CPU %03d at %lld\n",
          current->handle->filename,
          record->cpu,
          record->ts);
        return 1;
      }

      current->handle->cpus[record->cpu] = 0;
    }
  }

  printf("Events kvm_entry and kvm_exit checked correctly!\n");

  free_data(all_records, custom_handles, argc-1);

}