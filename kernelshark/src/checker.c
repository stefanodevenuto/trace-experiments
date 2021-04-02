#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libkshark.h"
#include "libkshark-tepdata.h"

#define KVM_ENTRY "kvm/kvm_entry"
#define KVM_EXIT "kvm/kvm_exit"

struct custom_stream {
  struct kshark_data_stream* original_stream;
  int* cpus;
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

void print_entries(struct kshark_entry **entries, ssize_t n_entries) {
  struct kshark_data_stream* stream;
  char* event_name;
  int stream_id;

  for (int i = 0; i < n_entries; ++i) {
    stream = kshark_get_stream_from_entry(entries[i]);
    event_name = kshark_get_event_name(entries[i]);

    stream_id = stream->stream_id;
    printf("%d: %s-%d, %lld [%03d]:%s\t%s\n",
      stream->stream_id, 
      kshark_get_task(entries[i]),
      kshark_get_pid(entries[i]),
      entries[i]->ts,
      entries[i]->cpu,
      event_name,
      kshark_get_info(entries[i]));
  }
}

void free_data(struct kshark_context *kshark_ctx,
               struct custom_stream** custom_streams,
               struct kshark_entry** entries, int n_entries) {
  
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
  int n_guest;
  char* info;
  int host;
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

  entries = NULL;
  n_entries = kshark_load_all_entries(kshark_ctx, &entries);

  //print_entries(entries, n_entries);

  for (int i = 0; i < n_entries; ++i) {
    current = entries[i];

    stream = kshark_get_stream_from_entry(current);
    event_name = kshark_get_event_name(current);

    custom_stream = custom_streams[stream->stream_id];

    printf("%d: %s-%d, %lld [%03d]:%s\t%s\n",
      stream->stream_id, 
      kshark_get_task(entries[i]),
      kshark_get_pid(entries[i]),
      entries[i]->ts,
      entries[i]->cpu,
      event_name,
      kshark_get_info(entries[i]));

    if (!strcmp(event_name, KVM_ENTRY) || !strcmp(event_name, KVM_EXIT)) {
      if (!strcmp(event_name, KVM_ENTRY)) {

        /*
         * The recovering process of the vCPU field of the kvm_entry event
         * is done by splitting the info field, because apparently the vCPU one
         * is not considered a "real" field (like for example the rip one of the same event)
         */
        info = kshark_get_info(current);

        char * token = strtok(info, " ");
        token = strtok(NULL, " ");

        // Removing the last comma
        token[strlen(token) - 1] = '\0';

        custom_stream->cpus[current->cpu] = atoi(token);

        free(info);
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

        if (v_i == host_stream->original_stream->n_cpus) {
          printf("ERROR: guest event outside of kvm_entry/kvm_exit block\n");
          //return 1;
        }

      /*
       * If the event comes from a host, recover the CPU that executed the event
       * and check if it's NOT INSIDE a kvm_entry/kvm_exit block.
       */
      } else {
        if (custom_stream->cpus[current->cpu] != -1) {
          printf("ERROR: host event inside of kvm_entry/kvm_exit block\n");
          //return 1;
        }
      }
    }
  }

  free_data(kshark_ctx, custom_streams, entries, n_entries);
}
