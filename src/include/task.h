#ifndef TASK_H_
#define TASK_H_

#include "main.h"

/* Utility */
void safe_strncat(char *dst, const char *src, size_t cap);
void ensure_data_folder(void);

/* Undo/Redo */
void init_undo_system(void);
void push_undo(Action a);
int pop_undo(Action *out);
void push_redo(Action a);
int pop_redo(Action *out);
void clear_redo(void);

/* Tasks */
int get_next_task_id(void);
void update_next_id(int id);
Task* add_task(Task **head_ptr, const char *title, const char *desc, const char *cat, const char *deadline, int priority);
Task* add_task_with_id(Task **head_ptr, int id, const char *title, const char *desc, const char *cat, const char *deadline, int priority, int status, int depCount, int deps[]);
Task* find_task_by_id(Task *head_ptr, int id);
Task* remove_task(Task **head_ptr, int id);
void free_all_tasks(Task *head_ptr);

/* Persistence */
void save_tasks_to_file(Task *head_ptr, const char* userID);
void load_tasks_from_file(Task **head_ptr, const char* userID);

/* Scheduling & conflicts */
Task** generate_schedule(Task *head_ptr, int *taskCount);
void detect_conflicts_msgbox(Task *head_ptr);
int detect_cycles(Task *head_ptr);

/* Export */
void export_deps_dot(Task *head_ptr, const char *filename_base, const char* userID);


#endif // TASK_H_