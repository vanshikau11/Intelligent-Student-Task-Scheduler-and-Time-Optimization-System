#include "main.h"

/* Utility */
void safe_strncat(char *dst, const char *src, size_t cap) {
    if (dst == NULL || src == NULL || cap == 0) return;
    size_t dst_len = strlen(dst);
    if (dst_len >= cap - 1) return;
    strncat(dst, src, cap - dst_len - 1);
}
void ensure_data_folder(void) { CreateDirectory("data", NULL); }

/* Undo/Redo */
void init_undo_system(void) { undo_top = 0; redo_top = 0; }
void push_undo(Action a) { if (undo_top < MAX_STACK) undo_stack[undo_top++] = a; }
int pop_undo(Action *out) { if (undo_top == 0) return 0; *out = undo_stack[--undo_top]; return 1; }
void push_redo(Action a) { if (redo_top < MAX_STACK) redo_stack[redo_top++] = a; }
int pop_redo(Action *out) { if (redo_top == 0) return 0; *out = redo_stack[--redo_top]; return 1; }
void clear_redo(void) { redo_top = 0; }

/* Tasks */
int get_next_task_id(void) { return next_id++; }
void update_next_id(int id) { if (id >= next_id) next_id = id + 1; }

Task* add_task(Task **head_ptr, const char *title, const char *desc, const char *cat, const char *deadline, int priority) {
    Task *t = (Task*)malloc(sizeof(Task));
    if (!t) return NULL;
    t->id = get_next_task_id();
    strncpy(t->title, title ? title : "", MAX_TITLE-1); t->title[MAX_TITLE-1] = '\0';
    strncpy(t->description, desc ? desc : "", MAX_DESC-1); t->description[MAX_DESC-1] = '\0';
    strncpy(t->category, cat ? cat : "", MAX_CATEGORY-1); t->category[MAX_CATEGORY-1] = '\0';
    strncpy(t->deadline, deadline ? deadline : "0000-00-00", 19); t->deadline[19] = '\0';
    t->priority = (priority >=1 && priority <=3) ? priority : 2;
    t->status = 0;
    t->depCount = 0;
    for (int i=0;i<MAX_DEP_PER_TASK;i++) t->dependencyIDs[i]=0;
    t->next = *head_ptr;
    *head_ptr = t;
    return t;
}

Task* add_task_with_id(Task **head_ptr, int id, const char *title, const char *desc, const char *cat, const char *deadline, int priority, int status, int depCount, int deps[]) {
    Task *t = (Task*)malloc(sizeof(Task));
    if (!t) return NULL;
    t->id = id;
    strncpy(t->title, title ? title : "", MAX_TITLE-1); t->title[MAX_TITLE-1] = '\0';
    strncpy(t->description, desc ? desc : "", MAX_DESC-1); t->description[MAX_DESC-1] = '\0';
    strncpy(t->category, cat ? cat : "", MAX_CATEGORY-1); t->category[MAX_CATEGORY-1] = '\0';
    strncpy(t->deadline, deadline ? deadline : "0000-00-00", 19); t->deadline[19] = '\0';
    t->priority = (priority >=1 && priority <=3) ? priority : 2;
    t->status = status ? 1 : 0;
    t->depCount = (depCount > MAX_DEP_PER_TASK) ? MAX_DEP_PER_TASK : depCount;
    for (int i=0;i<t->depCount;i++) t->dependencyIDs[i] = deps[i];
    for (int i=t->depCount;i<MAX_DEP_PER_TASK;i++) t->dependencyIDs[i]=0;
    t->next = *head_ptr;
    *head_ptr = t;
    update_next_id(id);
    return t;
}

Task* find_task_by_id(Task *head_ptr, int id) {
    Task *cur = head_ptr;
    while (cur) { if (cur->id == id) return cur; cur = cur->next; }
    return NULL;
}

Task* remove_task(Task **head_ptr, int id) {
    Task *cur = *head_ptr, *prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next; else *head_ptr = cur->next;
            cur->next = NULL;
            return cur;
        }
        prev = cur; cur = cur->next;
    }
    return NULL;
}

void free_all_tasks(Task *head_ptr) {
    Task *cur = head_ptr;
    while (cur) { 
        Task *next = cur->next;
        free(cur); 
        cur = next;
    }
}

/* Persistence */
void save_tasks_to_file(Task *head_ptr, const char* userID) {
    if (!userID || userID[0] == '\0') return;
    char filename[128];
    snprintf(filename, sizeof(filename), "data/tasks_%s.txt", userID);

    FILE *f = fopen(filename, "w");
    if (!f) return;
    for (Task *t = head_ptr; t; t = t->next) {
        fprintf(f, "%d|%s|%s|%s|%s|%d|%d|%d",
                t->id, t->title, t->description, t->category, t->deadline, t->priority, t->status, t->depCount);
        for (int i=0;i<t->depCount;i++) fprintf(f, "|%d", t->dependencyIDs[i]);
        fprintf(f, "\n");
    }
    fclose(f);
}

void load_tasks_from_file(Task **head_ptr, const char* userID) {
    if (!userID || userID[0] == '\0') return;
    char filename[128];
    snprintf(filename, sizeof(filename), "data/tasks_%s.txt", userID);

    FILE *f = fopen(filename, "r");
    if (!f) return;
    if (*head_ptr != NULL) { free_all_tasks(*head_ptr); *head_ptr=NULL; }
    next_id = 1; // Reset next_id for the new user's list
    char line[2048];
    while (fgets(line, sizeof(line), f)) {
        char *tokens[64]; int tk=0;
        char *p = strtok(line, "|\n");
        while (p && tk < 64) { tokens[tk++] = p; p = strtok(NULL, "|\n"); }
        if (tk < 8) continue;
        int id = atoi(tokens[0]);
        char *title = tokens[1];
        char *desc = tokens[2];
        char *cat = tokens[3];
        char *deadline = tokens[4];
        int prio = atoi(tokens[5]);
        int status = atoi(tokens[6]);
        int depCount = atoi(tokens[7]);
        int deps[MAX_DEP_PER_TASK] = {0};
        for (int i=0;i<depCount && 8+i < tk && i < MAX_DEP_PER_TASK;i++) deps[i] = atoi(tokens[8+i]);
        add_task_with_id(head_ptr, id, title, desc, cat, deadline, prio, status, depCount, deps);
    }
    fclose(f);
}

/* Scheduling & conflicts */
static int compare_tasks(const void *a, const void *b) {
    Task *ta = *(Task**)a;
    Task *tb = *(Task**)b;
    if (ta->status && !tb->status) return 1;
    if (!ta->status && tb->status) return -1;
    if (ta->status && tb->status) return 0;
    int cmp = strcmp(ta->deadline, tb->deadline);
    if (cmp != 0) return cmp;
    return ta->priority - tb->priority;
}

Task** generate_schedule(Task *head_ptr, int *taskCount) {
    int n=0; for (Task *t=head_ptr;t;t=t->next) if (!t->status) n++;
    *taskCount = n;
    if (n==0) return NULL;
    Task **arr = (Task**)malloc(n*sizeof(Task*)); if (!arr){*taskCount=0;return NULL;}
    int i=0; for (Task *t=head_ptr;t;t=t->next) if (!t->status) arr[i++]=t;
    qsort(arr, n, sizeof(Task*), compare_tasks);
    return arr;
}

void detect_conflicts_msgbox(Task *head_ptr) {
    if (!head_ptr) { MessageBox(NULL, "No tasks to check for conflicts.", "Conflicts", MB_OK|MB_ICONINFORMATION); return; }
    int pending=0; for (Task *t=head_ptr;t;t=t->next) if (!t->status) pending++;
    if (pending<2) { MessageBox(NULL, "Not enough pending tasks for conflicts.", "Conflicts", MB_OK|MB_ICONINFORMATION); return; }
    int count=0; Task **arr = generate_schedule(head_ptr, &count);
    if (!arr || count<2) { if (arr) free(arr); MessageBox(NULL, "No pending task conflicts.", "Conflicts", MB_OK); return; }
    char *buffer = (char*)calloc(16384, sizeof(char));
    if (!buffer) { MessageBox(NULL, "Failed to allocate memory.", "Error", MB_OK | MB_ICONERROR); free(arr); return; }
    
    buffer[0]='\0'; int found=0;
    for (int i=0;i<count-1;i++) for (int j=i+1;j<count;j++) {
        if (strcmp(arr[i]->deadline, arr[j]->deadline)==0) {
            if(!found) {
                strcpy(buffer, "Conflicts Found (Same Deadline):\n\n");
                found = 1;
            }
            char tmp[512];
            if (arr[i]->priority != arr[j]->priority) snprintf(tmp,sizeof(tmp)," - Task %d ('%s') and Task %d ('%s') have same deadline (%s) but DIFFERENT priorities.\n",
                arr[i]->id, arr[i]->title, arr[j]->id, arr[j]->title, arr[i]->deadline);
            else snprintf(tmp,sizeof(tmp)," - Task %d ('%s') and Task %d ('%s') have same deadline (%s) and SAME priority.\n",
                arr[i]->id, arr[i]->title, arr[j]->id, arr[j]->title, arr[i]->deadline);
            safe_strncat(buffer, tmp, 16384);
            
            for (int d=0; d < arr[i]->depCount; d++) if (arr[i]->dependencyIDs[d] == arr[j]->id) { safe_strncat(buffer, "   -> Note: First task depends on second.\n", 16384); break; }
            for (int d=0; d < arr[j]->depCount; d++) if (arr[j]->dependencyIDs[d] == arr[i]->id) { safe_strncat(buffer, "   -> Note: Second task depends on first.\n", 16384); break; }
        }
    }
    if (!found) strcpy(buffer,"No pending task conflicts.");
    MessageBoxA(NULL, buffer, "Conflict Detection", MB_OK);
    free(arr);
    free(buffer);
}

/* ---------------- Dependency cycle detection ---------------- */
static int find_task_index_local(int *ids, int n, int id) {
    for (int i=0;i<n;i++) if (ids[i] == id) return i;
    return -1;
}

int detect_cycles(Task *head_ptr) {
    int n = 0; for (Task *t = head_ptr; t; t = t->next) n++;
    if (n == 0) return 0;
    Task **arr = (Task**)malloc(n * sizeof(Task*));
    int *ids = (int*)malloc(n * sizeof(int));
    if (!arr || !ids) { if (arr) free(arr); if (ids) free(ids); return 0; }
    int idx=0; for (Task *t = head_ptr; t; t = t->next) { arr[idx]=t; ids[idx]=t->id; idx++; }

    int **cyc_adj = (int**)calloc(n, sizeof(int*));
    int *cyc_adj_sz = (int*)calloc(n, sizeof(int));
    int *cyc_state = (int*)calloc(n, sizeof(int));
    if (!cyc_adj || !cyc_adj_sz || !cyc_state) {
        if (cyc_adj) free(cyc_adj);
        if (cyc_adj_sz) free(cyc_adj_sz);
        if (cyc_state) free(cyc_state);
        free(arr); free(ids);
        return 0;
    }
    for (int i=0;i<n;i++) { cyc_adj[i]=NULL; cyc_adj_sz[i]=0; }

    // Build adjacency: edge from task i -> dependency index j
    for (int i=0;i<n;i++) {
        Task *t = arr[i];
        for (int d=0; d < t->depCount; d++) {
            int dep = t->dependencyIDs[d];
            int j = find_task_index_local(ids, n, dep);
            if (j >= 0) {
                cyc_adj[i] = (int*)realloc(cyc_adj[i], (cyc_adj_sz[i] + 1) * sizeof(int));
                if (!cyc_adj[i]) continue;
                cyc_adj[i][cyc_adj_sz[i]++] = j;
            }
        }
    }

    // DFS
    int dfs(int u, int *state, int **adj, int *adj_sz){
        state[u] = 1;
        for (int k=0;k<adj_sz[u];k++) {
            int v = adj[u][k];
            if (state[v] == 0) {
                if (dfs(v, state, adj, adj_sz)) return 1;
            } else if (state[v] == 1) {
                return 1;
            }
        }
        state[u] = 2;
        return 0;
    }
    int hasCycle = 0;
    for (int i=0;i<n && !hasCycle;i++) if (cyc_state[i] == 0) if (dfs(i, cyc_state, cyc_adj, cyc_adj_sz)) hasCycle = 1;

    for (int i=0;i<n;i++) if (cyc_adj[i]) free(cyc_adj[i]);
    free(cyc_adj); free(cyc_adj_sz); free(cyc_state); free(arr); free(ids);
    return hasCycle;
}

/* ---------------- Export DOT ---------------- */
void export_deps_dot(Task *head_ptr, const char *filename_base, const char* userID) {
    if (!userID || userID[0] == '\0') return;
    char filename[128];
    snprintf(filename, sizeof(filename), "data/%s_%s.dot", filename_base, userID);

    FILE *f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "digraph dependencies {\n");
    fprintf(f, "  rankdir=LR;\n");
    for (Task *t = head_ptr; t; t = t->next) {
        char safe_title[MAX_TITLE*2]; int si=0;
        for (int i=0;t->title[i] && si < (int)sizeof(safe_title)-2;i++) {
            if (t->title[i] == '"') safe_title[si++]='\\';
            safe_title[si++] = t->title[i];
        }
        safe_title[si] = '\0';
        fprintf(f, "  node%d [label=\"%d: %s\"];\n", t->id, t->id, safe_title);
    }
    for (Task *t = head_ptr; t; t = t->next) {
        for (int d=0; d < t->depCount; d++) {
            int dep = t->dependencyIDs[d];
            fprintf(f, "  node%d -> node%d;\n", t->id, dep);
        }
    }
    fprintf(f, "}\n");
    fclose(f);
}