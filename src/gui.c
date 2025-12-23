#include "main.h"

static void InitListViewColumns(HWND hList) {
    LVCOLUMN lvc = {0};
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;

    struct { char *text; int width; } columns[] = {
        {"ID", 40},
        {"TASK", 80},
        {"CATEGORY", 80},
        {"PRIORITY", 70},
        {"DEADLINE", 90},
        {"STATUS", 80},
        {"DEPENDS", 90},
        {"DESC", 160}
    };

    for (int i = 0; i < 8; i++) {
        lvc.iSubItem = i; 
        lvc.cx = columns[i].width;
        lvc.pszText = columns[i].text;
        ListView_InsertColumn(hList, i, &lvc);
    }
}

static void PopulateListView(HWND hList, Task *head_ptr) {
    ListView_DeleteAllItems(hList); // Clear old data
    if (!head_ptr) return; // No tasks

    LVITEM lvi = {0};
    lvi.mask = LVIF_TEXT;
    
    char buffer[MAX_DESC]; // A re-usable buffer for formatting text
    int itemIndex = 0;

    for (Task *cur = head_ptr; cur; cur = cur->next) {
        // --- 1. Insert the ID (Column 0) ---
        lvi.iItem = itemIndex;
        lvi.iSubItem = 0;
        snprintf(buffer, 16, "%d", cur->id);
        lvi.pszText = buffer;
        ListView_InsertItem(hList, &lvi);

        // --- 2. Task Title (Column 1) ---
        ListView_SetItemText(hList, itemIndex, 1, cur->title);

        // --- 2. Set Category (Column 2) ---
        ListView_SetItemText(hList, itemIndex, 2, cur->category);

        // --- 3. Set Priority (Column 3) ---
        if (cur->priority == 1) strcpy(buffer, "High");
        else if (cur->priority == 2) strcpy(buffer, "Medium");
        else strcpy(buffer, "Low");
        ListView_SetItemText(hList, itemIndex, 3, buffer);

        // --- 4. Set Deadline (Column 4) ---
        ListView_SetItemText(hList, itemIndex, 4, cur->deadline);

        // --- 5. Set Status (Column 5) ---
        strcpy(buffer, cur->status ? "Completed" : "Pending");
        ListView_SetItemText(hList, itemIndex, 5, buffer);

        // --- 6. Set Dependencies (Column 6) ---
        if (cur->depCount == 0) {
            strcpy(buffer, "None");
        } else {
            buffer[0] = '\0'; // Clear the buffer
            for (int i = 0; i < cur->depCount; i++) {
                char t[16];
                snprintf(t, sizeof(t), "%d", cur->dependencyIDs[i]);
                safe_strncat(buffer, t, sizeof(buffer));
                if (i < cur->depCount - 1) 
                    safe_strncat(buffer, ",", sizeof(buffer));
            }
        }
        ListView_SetItemText(hList, itemIndex, 6, buffer);

        // --- 7. Set Description (Column 7) ---
        ListView_SetItemText(hList, itemIndex, 7, cur->description);

        itemIndex++; // Move to the next row
    }
}

/* ---------------- GUI: helpers & screens ---------------- */
void DrawGradientBackground(HDC hdc, RECT rc) {
    int height = rc.bottom - rc.top;
    for (int i = 0; i < height; i++) {
        int r = 220 - (i * 30) / (height>0?height:1);
        int g = 235 - (i * 40) / (height>0?height:1);
        int b = 255 - (i * 20) / (height>0?height:1);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(r, g, b));
        HPEN old = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, rc.left, rc.top + i, NULL);
        LineTo(hdc, rc.right, rc.top + i);
        SelectObject(hdc, old);
        DeleteObject(pen);
    }
}

//close child window
void ClearScreen(HWND hwnd) {
    HWND child = GetWindow(hwnd, GW_CHILD);
    while (child) {
        HWND next = GetWindow(child, GW_HWNDNEXT);
        DestroyWindow(child);
        child = next;
    }
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}
//login

void ShowLoginScreen(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_LOGIN;
    CreateWindow("STATIC","User ID:", WS_VISIBLE|WS_CHILD, 50, 50, 80, 20, hwnd, NULL, NULL, NULL);
    hUser = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 150, 50, 120, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC","Password:", WS_VISIBLE|WS_CHILD, 50, 80, 80, 20, hwnd, NULL, NULL, NULL);
    hPass = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER|ES_PASSWORD, 150, 80, 120, 20, hwnd, NULL, NULL, NULL);
    
    // ADDED | BS_OWNERDRAW HERE:
    CreateWindow("BUTTON","Login", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 150, 120, 80, 30, hwnd, (HMENU)ID_SUBMIT_LOGIN, NULL, NULL);
   

    CreateWindow("STATIC","Name:", WS_VISIBLE|WS_CHILD, 50, 200, 80, 20, hwnd, NULL, NULL, NULL);
    hName = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 150, 200, 120, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC","User ID:", WS_VISIBLE|WS_CHILD, 50, 230, 80, 20, hwnd, NULL, NULL, NULL);
    hUserSignup = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 150, 230, 120, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("STATIC","Password:", WS_VISIBLE|WS_CHILD, 50, 260, 80, 20, hwnd, NULL, NULL, NULL);
    hPassSignup = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER|ES_PASSWORD, 150, 260, 120, 20, hwnd, NULL, NULL, NULL);
    
    // AND ADDED | BS_OWNERDRAW HERE:
    CreateWindow("BUTTON","Sign Up", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 150, 300, 80, 30, hwnd, (HMENU)ID_SUBMIT_SIGNUP, NULL, NULL);
}

// ... rest of gui.c ...
//dashboard
void ShowDashboard(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_DASHBOARD;

    // --- LEFT COLUMN ---
    // 1. Add Task (Y=20)
    CreateWindow("BUTTON","Add New Task", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 50, 20, 180, 30, hwnd, (HMENU)ID_DASHBOARD_ADD, NULL, NULL);
    
    // 2. Edit Task (Y=60)
    CreateWindow("BUTTON","Edit Task", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 50, 60, 180, 30, hwnd, (HMENU)ID_DASHBOARD_EDIT, NULL, NULL);
    
    // 3. View Tasks (Y=100)
    CreateWindow("BUTTON","View All Tasks", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 50, 100, 180, 30, hwnd, (HMENU)ID_DASHBOARD_VIEW, NULL, NULL);
    
    // 4. Mark Complete (Y=140) -- FIXED COORDINATE
    CreateWindow("BUTTON","Mark Task Completed", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 50, 140, 180, 30, hwnd, (HMENU)ID_DASHBOARD_COMPLETE, NULL, NULL);
    
    // 5. Delete Task (Y=180) -- FIXED COORDINATE
    CreateWindow("BUTTON","Delete Task", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 50, 180, 180, 30, hwnd, (HMENU)ID_DASHBOARD_DELETE, NULL, NULL);

    // --- RIGHT COLUMN ---
    CreateWindow("BUTTON","Generate Schedule", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 20, 180, 30, hwnd, (HMENU)ID_DASHBOARD_SCHEDULE, NULL, NULL);
    CreateWindow("BUTTON","Detect Conflicts", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 60, 180, 30, hwnd, (HMENU)ID_DASHBOARD_CONFLICTS, NULL, NULL);
    CreateWindow("BUTTON","Save Weekly Summary", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 100, 180, 30, hwnd, (HMENU)ID_DASHBOARD_SUMMARY, NULL, NULL);
    CreateWindow("BUTTON","Analytics", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 140, 180, 30, hwnd, (HMENU)ID_DASHBOARD_ANALYTICS, NULL, NULL);
    
    CreateWindow("BUTTON","Undo", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 180, 85, 30, hwnd, (HMENU)ID_DASHBOARD_UNDO, NULL, NULL);
    CreateWindow("BUTTON","Redo", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 345, 180, 85, 30, hwnd, (HMENU)ID_DASHBOARD_REDO, NULL, NULL);
    
    // --- EXIT ---
    CreateWindow("BUTTON","Exit", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 180, 250, 100, 40, hwnd, (HMENU)ID_DASHBOARD_EXIT, NULL, NULL);
    
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}

//add task screen
void ShowAddTaskForm(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_ADD_TASK;
    
    // All labels (STATIC) are now 130 pixels wide
    // All controls (EDIT/COMBOBOX) now start at X-position 160

    CreateWindow("STATIC","Title:", WS_VISIBLE|WS_CHILD, 20, 20, 130, 20, hwnd, NULL, NULL, NULL);
    hTitle = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 160, 20, 350, 20, hwnd, NULL, NULL, NULL);
    
    CreateWindow("STATIC","Description:", WS_VISIBLE|WS_CHILD, 20, 50, 130, 20, hwnd, NULL, NULL, NULL);
    hDesc = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER|ES_AUTOVSCROLL, 160, 50, 350, 60, hwnd, NULL, NULL, NULL);
    
    CreateWindow("STATIC","Category:", WS_VISIBLE|WS_CHILD, 20, 120, 130, 20, hwnd, NULL, NULL, NULL);
    hCat = CreateWindow("COMBOBOX","", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 
                        160, 120, 200, 150, hwnd, NULL, NULL, NULL);
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"study");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"playing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"dancing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"singing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"exercise");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"other");
    SendMessage(hCat, CB_SETCURSEL, (WPARAM)0, 0); 

    
    // --- MODIFIED BLOCK FOR CALENDAR ---
    CreateWindow("STATIC","Deadline:", WS_VISIBLE|WS_CHILD, 20, 150, 130, 20, hwnd, NULL, NULL, NULL);
    hDeadline = CreateWindowEx(0, 
                               MONTHCAL_CLASS, // Use the calendar class
                               "", 
                               WS_VISIBLE | WS_CHILD | WS_BORDER, 
                               160, 150,       // X, Y position
                               200, 160,      // Width, Height (calendar is large)
                               hwnd, NULL, NULL, NULL);
    // --- END OF MODIFIED BLOCK ---

    
    // --- RE-POSITIONED CONTROLS (MOVED DOWN) ---
    // The calendar ends at Y = 150 + 160 = 310. Start next control at Y=320.
    
    CreateWindow("STATIC","Priority (1-3):", WS_VISIBLE|WS_CHILD, 20, 320, 130, 20, hwnd, NULL, NULL, NULL);
    hPriority = CreateWindow("COMBOBOX","", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_BORDER, 
                             160, 320, 50, 100, hwnd, NULL, NULL, NULL);
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"1");
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"2");
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"3");
    SendMessage(hPriority, CB_SETCURSEL, (WPARAM)1, 0);

    
    CreateWindow("STATIC","Dependencies (IDs):", WS_VISIBLE|WS_CHILD, 20, 350, 130, 20, hwnd, NULL, NULL, NULL);
    hDeps = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 160, 350, 230, 20, hwnd, NULL, NULL, NULL);
    
    // Shifted buttons down
    CreateWindow("BUTTON","Submit Task", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 160, 390, 120, 30, hwnd, (HMENU)ID_ADD_TASK_SUBMIT, NULL, NULL);
    CreateWindow("BUTTON","Back", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 290, 390, 120, 30, hwnd, (HMENU)ID_ADD_TASK_BACK, NULL, NULL);
}
//show user to enter an id
void ShowGetIdForm(HWND hwnd, const char* prompt, int next_action) {
    ClearScreen(hwnd);
    currentState = STATE_GET_ID;
    get_id_next_action = next_action;
    CreateWindowA("STATIC", prompt, WS_VISIBLE|WS_CHILD|SS_CENTER, 50, 50, 420, 20, hwnd, NULL, NULL, NULL);
    hGetIdInput = CreateWindow("EDIT","", WS_VISIBLE|WS_CHILD|WS_BORDER, 190, 80, 100, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("BUTTON","Submit", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 140, 120, 100, 30, hwnd, (HMENU)ID_GETID_SUBMIT, NULL, NULL);
    CreateWindow("BUTTON","Back", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 250, 120, 100, 30, hwnd, (HMENU)ID_GETID_BACK, NULL, NULL);
}
//analytic screen
void ShowAnalyticsScreen(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_ANALYTICS;

    CreateWindow("STATIC", "Tasks Analytics & Dependency Graph", WS_VISIBLE | WS_CHILD | SS_CENTER,
                 20, 10, 540, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("BUTTON", "Back", WS_VISIBLE | WS_CHILD |BS_OWNERDRAW,
                 220, 450, 100, 30, hwnd, (HMENU)ID_ADD_TASK_BACK, NULL, NULL);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}


void ShowViewTasksScreen(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_VIEW_TASKS;

    // Create the ListView control
    hViewList = CreateWindowEx(WS_EX_CLIENTEDGE, // Use an extended style for the border
                               WC_LISTVIEW,      // This is the class name for ListView
                               "",               // No title
                               WS_VISIBLE | WS_CHILD | WS_VSCROLL | LVS_REPORT | LVS_SINGLESEL, 
                               20, 20, 620, 400, // Position and size
                               hwnd, NULL, NULL, NULL);
    
    // Set cool styles: gridlines and full-row highlighting
    ListView_SetExtendedListViewStyle(hViewList, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

    // Create the Back button (same as before)
    CreateWindow("BUTTON", "Back", 
                 WS_VISIBLE | WS_CHILD | BS_OWNERDRAW , 
                 280, 430, 100, 30, 
                 hwnd, (HMENU)ID_VIEW_TASKS_BACK, NULL, NULL);

    // Call our new helper functions to build the table
    InitListViewColumns(hViewList);
    PopulateListView(hViewList, head); // 'head' is the global task list
}

void ShowScheduleScreen(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_VIEW_TASKS;   // reuse same state

    // Create ListView
    hViewList = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WC_LISTVIEW,
        "",
        WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS ,
        20, 20, 620, 400,
        hwnd, NULL, NULL, NULL
    );

    // full row select + grid lines (same as view tasks)
    SendMessage(hViewList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0,
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    // same columns as before
    InitListViewColumns(hViewList);

    // if there is no schedule data, just show empty table
    if (!schedule_arr || schedule_count == 0) {
        CreateWindow("BUTTON","Back",
                     WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                     260, 430, 120, 30,
                     hwnd, (HMENU)ID_VIEW_TASKS_BACK, NULL, NULL);
        return;
    }

    LVITEM lvi;
    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask = LVIF_TEXT;

    char buffer[MAX_DESC];
    int itemIndex = 0;

    for (int i = 0; i < schedule_count; i++) {
        Task *cur = schedule_arr[i];

        // --- Column 0: ID ---
        lvi.iItem = itemIndex;
        lvi.iSubItem = 0;
        snprintf(buffer, 16, "%d", cur->id);
        lvi.pszText = buffer;
        ListView_InsertItem(hViewList, &lvi);
        
        // ---- column 1:task ----
        ListView_SetItemText(hViewList, itemIndex, 1, cur->title);

        // --- Column 2: Category ---
        ListView_SetItemText(hViewList, itemIndex, 2, cur->category);

        // --- Column 3: Priority ---
        if (cur->priority == 1) strcpy(buffer, "High");
        else if (cur->priority == 2) strcpy(buffer, "Medium");
        else strcpy(buffer, "Low");
        ListView_SetItemText(hViewList, itemIndex, 3, buffer);

        // --- Column 4: Deadline ---
        ListView_SetItemText(hViewList, itemIndex, 4, cur->deadline);

        // --- Column 5: Status ---
        strcpy(buffer, cur->status ? "Completed" : "Pending");
        ListView_SetItemText(hViewList, itemIndex, 5, buffer);

        // --- Column 6: Dependencies ---
        if (cur->depCount == 0) {
            strcpy(buffer, "None");
        } else {
            buffer[0] = '\0';
            for (int d = 0; d < cur->depCount; d++) {
                char t[16];
                snprintf(t, sizeof(t), "%d", cur->dependencyIDs[d]);
                safe_strncat(buffer, t, sizeof(buffer));
                if (d < cur->depCount - 1)
                    safe_strncat(buffer, ",", sizeof(buffer));
            }
        }
        ListView_SetItemText(hViewList, itemIndex, 6, buffer);

        // --- Column 7: Description ---
        ListView_SetItemText(hViewList, itemIndex, 7, cur->description);

        itemIndex++;
    }

    // Back button
    CreateWindow("BUTTON","Back",
                 WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                 260, 430, 120, 30,
                 hwnd, (HMENU)ID_VIEW_TASKS_BACK, NULL, NULL);
}

void ShowEditTaskForm(HWND hwnd) {
    ClearScreen(hwnd);
    currentState = STATE_EDIT_TASK;
    if (!editingTask) { ShowDashboard(hwnd); return; }

    // --- ID Display (Read Only) ---
    CreateWindow("STATIC","Edit Task ID:", WS_VISIBLE|WS_CHILD, 20, 20, 130, 20, hwnd, NULL, NULL, NULL);
    char idStr[32]; snprintf(idStr, 32, "%d", editingTask->id);
    CreateWindow("STATIC", idStr, WS_VISIBLE|WS_CHILD, 160, 20, 350, 20, hwnd, NULL, NULL, NULL); 

    // --- Title ---
    CreateWindow("STATIC","Title:", WS_VISIBLE|WS_CHILD, 20, 50, 130, 20, hwnd, NULL, NULL, NULL);
    hTitle = CreateWindow("EDIT", editingTask->title, WS_VISIBLE|WS_CHILD|WS_BORDER, 160, 50, 350, 20, hwnd, NULL, NULL, NULL);
    
    // --- Description ---
    CreateWindow("STATIC","Description:", WS_VISIBLE|WS_CHILD, 20, 80, 130, 20, hwnd, NULL, NULL, NULL);
    hDesc = CreateWindow("EDIT", editingTask->description, WS_VISIBLE|WS_CHILD|WS_BORDER|ES_AUTOVSCROLL, 160, 80, 350, 60, hwnd, NULL, NULL, NULL);
    
    // --- Category ---
    CreateWindow("STATIC","Category:", WS_VISIBLE|WS_CHILD, 20, 150, 130, 20, hwnd, NULL, NULL, NULL);
    hCat = CreateWindow("COMBOBOX","", CBS_DROPDOWN | WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL, 160, 150, 200, 150, hwnd, NULL, NULL, NULL);
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"study");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"playing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"dancing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"singing");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"exercise");
    SendMessage(hCat, CB_ADDSTRING, 0, (LPARAM)"other");
    // Auto-select current category
    int catIdx = SendMessage(hCat, CB_FINDSTRINGEXACT, -1, (LPARAM)editingTask->category);
    SendMessage(hCat, CB_SETCURSEL, (WPARAM)(catIdx >= 0 ? catIdx : 5), 0); 

    // --- Deadline ---
    CreateWindow("STATIC","Deadline:", WS_VISIBLE|WS_CHILD, 20, 180, 130, 20, hwnd, NULL, NULL, NULL);
    hDeadline = CreateWindowEx(0, MONTHCAL_CLASS, "", WS_VISIBLE | WS_CHILD | WS_BORDER, 160, 180, 200, 160, hwnd, NULL, NULL, NULL);
    // Parse current deadline string back to Calendar date
    int y, m, d;
    if (sscanf(editingTask->deadline, "%d-%d-%d", &y, &m, &d) == 3) {
        SYSTEMTIME st = {0};
        st.wYear = y; st.wMonth = m; st.wDay = d;
        SendMessage(hDeadline, MCM_SETCURSEL, 0, (LPARAM)&st);
    }

    // --- Priority ---
    CreateWindow("STATIC","Priority (1-3):", WS_VISIBLE|WS_CHILD, 20, 350, 130, 20, hwnd, NULL, NULL, NULL);
    hPriority = CreateWindow("COMBOBOX","", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_BORDER, 160, 350, 50, 100, hwnd, NULL, NULL, NULL);
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"1");
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"2");
    SendMessage(hPriority, CB_ADDSTRING, 0, (LPARAM)"3");
    SendMessage(hPriority, CB_SETCURSEL, (WPARAM)(editingTask->priority - 1), 0);

    // --- Dependencies ---
    CreateWindow("STATIC","Dependencies (IDs):", WS_VISIBLE|WS_CHILD, 20, 380, 130, 20, hwnd, NULL, NULL, NULL);
    char depStr[256] = {0};
    for(int i=0; i<editingTask->depCount; i++) {
        char num[16]; snprintf(num, 16, "%d", editingTask->dependencyIDs[i]);
        strcat(depStr, num);
        if(i < editingTask->depCount - 1) strcat(depStr, ", ");
    }
    hDeps = CreateWindow("EDIT", depStr, WS_VISIBLE|WS_CHILD|WS_BORDER, 160, 380, 230, 20, hwnd, NULL, NULL, NULL);
    
    // --- Buttons ---
    CreateWindow("BUTTON","Save Changes", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 160, 420, 120, 30, hwnd, (HMENU)ID_EDIT_TASK_SUBMIT, NULL, NULL);
    CreateWindow("BUTTON","Cancel", WS_VISIBLE|WS_CHILD|BS_OWNERDRAW, 290, 420, 120, 30, hwnd, (HMENU)ID_EDIT_TASK_BACK, NULL, NULL);
}

/* Random init & quote */
void ensure_random() { static int seeded=0; if (!seeded) { srand((unsigned)time(NULL)); seeded=1; } }
void show_random_login_quote(HWND hwnd) { ensure_random(); int idx = rand()%LOGIN_QUOTES_N; MessageBoxA(hwnd, LOGIN_QUOTES[idx], "Welcome!", MB_OK | MB_ICONINFORMATION); }