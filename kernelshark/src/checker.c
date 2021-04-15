#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libkshark.h"
#include "libkshark-tepdata.h"

#define KVM_ENTRY "kvm/kvm_entry"
#define KVM_EXIT "kvm/kvm_exit"

#define MAX_BUF_LEN 1024

struct custom_stream {
  struct kshark_data_stream* original_stream;
  int* cpus;
  int* check_vcpu;
};

int is_guest(int stream_id, 
             struct kshark_host_guest_map* mapping,
             int n_mapping, int* host) {

  for (int i = 0; i < n_mapping; i++) {
    if (mapping[i].guest_id == stream_id) {
      *host = mapping[i].host_id;
      return 1;
    }
  }

  return 0;
}

int get_string_field(char* string, char* field_name, char** field_value) {

    char* token = strtok(string, " ");
  
    while (token != NULL) {
      if (!strcmp(token, field_name)) {
        token = strtok(NULL, " ");
        *field_value = strndup(token, MAX_BUF_LEN);

        return 1;
      }

      token = strtok(NULL, " ");
    }

    return 0;
}

void print_entry(struct kshark_entry* entry) {
  struct kshark_data_stream* stream;
  char* event_name;
  int stream_id;

  stream = kshark_get_stream_from_entry(entry);
  event_name = kshark_get_event_name(entry);

  stream_id = stream->stream_id;
  printf("       %d: %s-%d, %lld [%03d]:%s\t%s\n",
    stream->stream_id,
    kshark_get_task(entry),
    kshark_get_pid(entry),
    entry->ts,
    entry->cpu,
    event_name,
    kshark_get_info(entry));

}

void print_entries(struct kshark_entry **entries, ssize_t n_entries) {

  for (int i = 0; i < n_entries; ++i) {
    print_entry(entries[i]);
  }
}

void free_data(struct kshark_context *kshark_ctx,
               struct custom_stream** custom_streams,
               struct kshark_entry** entries, int n_entries,
               struct kshark_host_guest_map* host_guest_mapping,
               int n_guest) {
  
  struct custom_stream* custom_stream;

  for (int i = 0; i < kshark_ctx->n_streams; i++) {
    custom_stream = custom_streams[i];

    free(custom_stream->cpus);
    free(custom_stream);
  }
  free(custom_streams);

  for (int i = 0; i < n_entries; i++) {
    free(entries[i]);
  }
  free(entries);

  kshark_tracecmd_free_hostguest_map(host_guest_mapping, n_guest);

  kshark_close_all(kshark_ctx);
  kshark_free(kshark_ctx);
}

int main(int argc, char **argv) {
  struct kshark_host_guest_map* host_guest_mapping;
  struct custom_stream** custom_streams;
  struct custom_stream* custom_stream;
  struct custom_stream* host_stream;
  struct kshark_data_stream* stream;
  struct kshark_context* kshark_ctx;
  struct kshark_entry** entries;
  struct kshark_entry* current;
  ssize_t n_entries;
  char* event_name;
  char* vcpu_field;
  int n_guest;
  char* info;
  int host;
  int vcpu;
  int v_i;
  int sd;

  kshark_ctx = NULL;
  if (!kshark_instance(&kshark_ctx))
    return 1;

  custom_streams = malloc(sizeof(*custom_streams) * (argc-1));

  for (int i = 1; i < argc; i++) {
    sd = kshark_open(kshark_ctx, argv[i]);
    if (sd < 0) {
      kshark_free(kshark_ctx);
      return 1;
    }

    kshark_tep_init_all_buffers(kshark_ctx, sd);

    custom_stream = malloc(sizeof(*custom_stream));
    custom_stream->original_stream = kshark_get_data_stream(kshark_ctx, sd);
    custom_stream->cpus = malloc(custom_stream->original_stream->n_cpus * sizeof(int));
    memset(custom_stream->cpus, -1, custom_stream->original_stream->n_cpus * sizeof(int));

    custom_streams[i-1] = custom_stream;
  }

  host_guest_mapping = NULL;
  n_guest = kshark_tracecmd_get_hostguest_mapping(&host_guest_mapping);
  if (n_guest < 0) {
    printf("Failed mapping: %d\n", n_guest);
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    custom_stream = custom_streams[i-1];
    if (!is_guest(custom_stream->original_stream->stream_id, host_guest_mapping, n_guest, &host)) {
      custom_stream->check_vcpu = calloc(22, sizeof(int)); // TODO: use DynArray
    }
  }

  entries = NULL;
  n_entries = kshark_load_all_entries(kshark_ctx, &entries);

  /* print_entries(entries, n_entries); */

  for (int i = 0; i < n_entries; ++i) {
    current = entries[i];

    stream = kshark_get_stream_from_entry(current);
    event_name = kshark_get_event_name(current);

    custom_stream = custom_streams[stream->stream_id];

    if (!strcmp(event_name, KVM_ENTRY) || !strcmp(event_name, KVM_EXIT)) {

      /*
       * The recovering process of the vCPU field of the kvm_entry event
       * is done by splitting the info field, because apparently the vCPU one
       * is not considered a "real" field (like for example the rip one of the same event)
       */
      info = kshark_get_info(current);
      get_string_field(info, "vcpu", &vcpu_field);

      /* Removing the last comma */
      vcpu_field[strlen(vcpu_field) - 1] = '\0';

      vcpu = atoi(vcpu_field);

      free(vcpu_field);
      free(info);

      custom_stream->check_vcpu[vcpu] = 1;

      if (!strcmp(event_name, KVM_ENTRY)) {
        custom_stream->cpus[current->cpu] = vcpu;
      } else {
        custom_stream->cpus[current->cpu] = -1;
      }

    } else {

      /* 
       * If the event comes from a guest, recover the pCPU where the event was executed
       * and check if it's NOT OUTSIDE a kvm_entry/kvm_exit block.
       */
      if (is_guest(stream->stream_id, host_guest_mapping, n_guest, &host)) {
        host_stream = custom_streams[host];

        for (v_i = 0; v_i < host_stream->original_stream->n_cpus; v_i++) {
          if (current->cpu == host_stream->cpus[v_i])
            break;
        }

        /*
         * Check for not valid guest events only if a previous entry/exit host event
         * for the current vCPU has been encountered.
         */
        if (v_i == host_stream->original_stream->n_cpus && host_stream->check_vcpu[current->cpu]) {
          printf("+ %4d G out:\t", i);
        }

      /*
       * If the event comes from a host, recover the CPU that executed the event
       * and check if it's NOT INSIDE a kvm_entry/kvm_exit block.
       */
      } else {
        if (custom_stream->cpus[current->cpu] != -1) {
          printf("+ %4d H in:\t", i);
        }
      }
    }

    print_entry(entries[i]);
  }

  free_data(kshark_ctx, custom_streams, entries, n_entries, host_guest_mapping, n_guest);
}
