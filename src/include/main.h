#define WINVER 0x0600
#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600

#ifndef MAIN_H_
#define MAIN_H_

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <commctrl.h> // All controls (including calendar) are here


/* ---------------- SETTINGS ---------------- */
#define MAX_TITLE 64
#define MAX_DESC 512
#define MAX_CATEGORY 32
#define MAX_STACK 400
#define MAX_DEP_PER_TASK 10
#define POINTS_PER_TASK 10

/* ---------------- DATA STRUCTURES ---------------- */
typedef struct Task {
    int id;
    char title[MAX_TITLE];
    char description[MAX_DESC];
    char category[MAX_CATEGORY];
    char deadline[20]; // YYYY-MM-DD
    int priority;      // 1-high,2-med,3-low
    int status;        // 0-pending,1-completed
    int dependencyIDs[MAX_DEP_PER_TASK];
    int depCount;
    struct Task *next;
} Task;

typedef struct {
    int type;
    Task snapshot;
} Action;
#define ACTION_ADD 1
#define ACTION_DELETE 2
#define ACTION_UPDATE 3

/* ---------------- CONTROL IDs ---------------- */
enum {
    ID_SUBMIT_LOGIN = 100,
    ID_SUBMIT_SIGNUP,
    ID_DASHBOARD_ADD,
    ID_DASHBOARD_VIEW,
    ID_DASHBOARD_COMPLETE,
    ID_DASHBOARD_DELETE,
    ID_DASHBOARD_SCHEDULE,
    ID_DASHBOARD_CONFLICTS,
    ID_DASHBOARD_SUMMARY,
    ID_DASHBOARD_UNDO,
    ID_DASHBOARD_REDO,
    ID_DASHBOARD_EXIT,
    ID_DASHBOARD_ANALYTICS,
    ID_DASHBOARD_THEME,
    ID_ADD_TASK_SUBMIT,
    ID_ADD_TASK_BACK,
    ID_GETID_SUBMIT,
    ID_GETID_BACK,
    ID_VIEW_TASKS_BACK,
    ID_DASHBOARD_EDIT,
    ID_EDIT_TASK_SUBMIT,
    ID_EDIT_TASK_BACK
};

/* ---------------- App State ---------------- */
enum AppState { STATE_LOGIN, STATE_DASHBOARD, STATE_ADD_TASK, STATE_GET_ID, STATE_ANALYTICS, STATE_VIEW_TASKS, STATE_EDIT_TASK };

/* ---------------- GLOBALS (Declarations) ---------------- */
extern const char *LOGIN_QUOTES[];
extern const int LOGIN_QUOTES_N;
extern Task *head;
extern Action undo_stack[MAX_STACK]; extern int undo_top;
extern Action redo_stack[MAX_STACK]; extern int redo_top;
extern int next_id;
extern int total_points;
extern char current_user_id[64];
extern Task **schedule_arr;
extern int schedule_count;

extern HWND hUser, hPass, hName, hUserSignup, hPassSignup;
extern HWND hTitle, hDesc, hCat, hDeadline, hPriority, hDeps;
extern HWND hGetIdInput;
extern HWND hViewList;
extern HWND hMain;
extern Task *editingTask;

extern enum AppState currentState;
extern int get_id_next_action;

/* ---------------- Include Sub-Headers ---------------- */
#include "auth.h"
#include "task.h"
#include "analytics.h"
#include "gui.h"

/* ---------------- Main.c Prototypes ---------------- */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif // MAIN_H_