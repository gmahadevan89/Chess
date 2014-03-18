#ifndef __SCHEDULER_H
#define __SCHEDULER_H

//enum sync_type_t {CREATE, UNLOCK, NP};

struct State_elem{
  int cur_tid;
  int sync_pt;
  int choice;
  struct State_elem *prev;
  struct State_elem *next;
};

struct Enab_th_elem {
  int t;
  struct Enab_th_elem *prev;
  struct Enab_th_elem *next;
};

/* We need a struct to hold a state_elem triplet and
 a coresponding list of enabled threads. We'll have
 a list of these structs, where each struct will be
 read from a line from the trace file. */
struct Replay_list_elem {
  //enum sync_type_t type;
  struct State_elem st; // This doesn't NEED to be a an "_elem" type, but what the hell.
  struct Enab_th_elem *et_head; // Damn you, C language
  struct Enab_th_elem *et_tail; // Damn you again
  struct Replay_list_elem *prev;
  struct Replay_list_elem *next;
};

#endif
