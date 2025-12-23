#include "main.h"

// Points & Badges
//load points
void load_points(const char* userID) {
    if (!userID || userID[0] == '\0') { total_points = 0; return; }
    char filename[128];
    snprintf(filename, sizeof(filename), "data/points_%s.txt", userID);

    FILE *f = fopen(filename,"r");
    if (!f) { total_points = 0; return; }
    fscanf(f,"%d",&total_points);
    fclose(f);
}
//save points
void save_points(const char* userID) {
    if (!userID || userID[0] == '\0') return;
    char filename[128];
    snprintf(filename, sizeof(filename), "data/points_%s.txt", userID);

    FILE *f = fopen(filename,"w");
    if (!f) return;
    fprintf(f,"%d\n", total_points);
    fclose(f);
}
void award_points(int pts, const char* userID) {
    total_points += pts;
    save_points(userID);
}
//show badges
void show_badges_if_any(HWND hwnd) {
    if (total_points >= 100) MessageBox(hwnd, "Badge: Productivity Master (100+ points)!", "Badge Earned", MB_OK);
    else if (total_points >= 50) MessageBox(hwnd, "Badge: Consistent Achiever (50+ points)!", "Badge Earned", MB_OK);
}

/* Weekly summary write */
void save_weekly_summary(Task *head_ptr, const char* userID) {
    if (!head_ptr) { MessageBox(NULL, "No tasks to summarize.", "Summary", MB_OK|MB_ICONINFORMATION); return; }
    if (!userID || userID[0] == '\0') return;

    char filename[128];
    snprintf(filename, sizeof(filename), "data/weekly_summary_%s.txt", userID);

    FILE *f = fopen(filename, "w");
    if (!f) { MessageBox(NULL, "Failed to open weekly summary file", "Error", MB_OK|MB_ICONERROR); return; }

    int week_total[12][5] = {0}, week_completed[12][5] = {0};
    for (Task *t = head_ptr; t; t = t->next) {
        int y=0,m=0,d=0;
        if (sscanf(t->deadline,"%d-%d-%d",&y,&m,&d) != 3) continue;
        if (m<1 || m>12) m=1;
        if (d<1) d=1; 
        if (d>31) d=31;
        int w = (d-1)/7; if (w>4) w=4;
        week_total[m-1][w]++;
        if (t->status) week_completed[m-1][w]++;
    }

    const char *months[] = {"January","February","March","April","May","June","July","August","September","October","November","December"};

    fprintf(f, "--- Weekly Summary ---\n\n");
    int written = 0;
    for (int m=0; m<12; m++) {
        for (int w=0; w<5; w++) {
            if (week_total[m][w] > 0) {
                fprintf(f, "%s - Week %d: Total %d | Completed %d\n",
                        months[m], w+1, week_total[m][w], week_completed[m][w]);
                written = 1;
            }
        }
    }
    if (!written) fprintf(f, "No tasks available for summary.\n");
    fclose(f);

    char msg[256];
    snprintf(msg, sizeof(msg), "Weekly summary saved to %s", filename);
    MessageBox(NULL, msg, "Summary", MB_OK);
}

/* Compute analytics values */
void ComputeAnalytics(Task *head_ptr,
                      int *total, int *completed, int *pending, int *overdue,
                      int *prioHigh, int *prioMed, int *prioLow,
                      int *waitingOnDeps, int *cyclesDetected,
                      int weekly_total[12][5], int weekly_completed[12][5],
                      int monthly_total[12], int monthly_completed[12]) {
    time_t now = time(NULL);
    struct tm tm_now;
    struct tm* tm_safe = localtime(&now);
    if (tm_safe) { tm_now = *tm_safe; } else { tm_now.tm_year = 2024 - 1900; tm_now.tm_mon = 0; tm_now.tm_mday = 1; }

    int cur_year = tm_now.tm_year + 1900, cur_month = tm_now.tm_mon + 1, cur_day = tm_now.tm_mday;
    *total = *completed = *pending = *overdue = 0;
    *prioHigh = *prioMed = *prioLow = 0;
    *waitingOnDeps = *cyclesDetected = 0;
    for (int m=0;m<12;m++) for (int w=0;w<5;w++) { weekly_total[m][w]=0; weekly_completed[m][w]=0; }
    for (int m=0;m<12;m++) { monthly_total[m]=0; monthly_completed[m]=0; }
    for (Task *t = head_ptr; t; t = t->next) {
        (*total)++; if (t->status) (*completed)++; else (*pending)++;
        if (!t->status) {
            int y=0,mo=0,d=0;
            if (sscanf(t->deadline,"%d-%d-%d",&y,&mo,&d)==3) {
                if (y < cur_year || (y==cur_year && (mo < cur_month || (mo==cur_month && d < cur_day)))) (*overdue)++;
            }
        }
        if (t->priority==1) (*prioHigh)++; else if (t->priority==2) (*prioMed)++; else (*prioLow)++;
        int y=0,mo=0,d=0; if (sscanf(t->deadline,"%d-%d-%d",&y,&mo,&d)==3) {
            if (mo<1||mo>12) mo=1; 
            if (d<1) d=1; 
            if (d>31) d=31;
            int w = (d-1)/7; if (w>4) w=4;
            weekly_total[mo-1][w]++; if (t->status) weekly_completed[mo-1][w]++;
            monthly_total[mo-1]++; if (t->status) monthly_completed[mo-1]++;
        }
        if (!t->status) {
            for (int i=0;i<t->depCount;i++) {
                Task *dep = find_task_by_id(head_ptr, t->dependencyIDs[i]);
                if (dep && dep->status==0) { (*waitingOnDeps)++; break; }
            }
        }
    }
    *cyclesDetected = detect_cycles(head_ptr);
}

/* Draw dependency graph (circular layout) */
void draw_dependency_graph(HDC hdc, RECT area, Task *head_ptr) {
    int n=0; for (Task *t=head_ptr;t;t=t->next) n++;
    if (n==0) { TextOutA(hdc, area.left+10, area.top+10, "No tasks", 7); return; }
    if (n > 40) { TextOutA(hdc, area.left+10, area.top+10, "Too many tasks to graph.", 25); return; }

    Task **arr = (Task**)malloc(n*sizeof(Task*));
    if (!arr) return;
    int idx=0;
    for (Task *t=head_ptr; t; t=t->next) arr[idx++]=t;
    int cx = (area.left + area.right)/2;
    int cy = (area.top + area.bottom)/2;
    int radius = (area.right - area.left) < (area.bottom - area.top) ? (area.right-area.left)/3 : (area.bottom-area.top)/3;
    if (radius < 60) radius = 60;
    POINT *pts = (POINT*)malloc(n * sizeof(POINT));
    if (!pts) { free(arr); return; }

    for (int i=0;i<n;i++) {
        double ang = 2.0 * 3.141592653589793 * i / n;
        pts[i].x = cx + (int)(radius * cos(ang));
        pts[i].y = cy + (int)(radius * sin(ang));
    }
    // Draw edges (dependencies)
    HPEN penEdge = CreatePen(PS_SOLID, 2, RGB(0, 0, 180));
    HPEN penOld = (HPEN)SelectObject(hdc, penEdge);
    for (int i=0;i<n;i++) {
        Task *t = arr[i];
        for (int d=0; d < t->depCount; d++) {
            int depId = t->dependencyIDs[d];
            int j=-1;
            for (int k=0;k<n;k++) if (arr[k]->id == depId) { j=k; break; }
            if (j>=0) {
                int ax = pts[i].x, ay = pts[i].y;
                int bx = pts[j].x, by = pts[j].y;
                double vx = ax - bx, vy = ay - by;
                double len = sqrt(vx*vx + vy*vy);
                if (len < 1) len = 1;
                double ux = vx / len, uy = vy / len;
                int startX = (int)(bx + ux*50);
                int startY = (int)(by + uy*50);
                int endX = (int)(ax - ux*50);
                int endY = (int)(ay - uy*50);
                MoveToEx(hdc, startX, startY, NULL);
                LineTo(hdc, endX, endY);
                POINT p1 = { endX - (int)(ux*12 - uy*6), endY - (int)(uy*12 + ux*6) };
                POINT p2 = { endX - (int)(ux*12 + uy*6), endY - (int)(uy*12 - ux*6) };
                MoveToEx(hdc, endX, endY, NULL);
                LineTo(hdc, p1.x, p1.y);
                MoveToEx(hdc, endX, endY, NULL);
                LineTo(hdc, p2.x, p2.y);
            }
        }
    }
    SelectObject(hdc, penOld);
    DeleteObject(penEdge);

    // Draw nodes
    for (int i=0;i<n;i++) {
        RECT r = { pts[i].x - 80, pts[i].y - 20, pts[i].x + 80, pts[i].y + 20 };
        HBRUSH hb = CreateSolidBrush(arr[i]->status ? RGB(200,240,200) : RGB(240,200,200));
        FillRect(hdc, &r, hb); DeleteObject(hb);
        Rectangle(hdc, r.left, r.top, r.right, r.bottom);
        char label[128]; snprintf(label, sizeof(label), "%d: %s", arr[i]->id, arr[i]->title);
        SetBkMode(hdc, TRANSPARENT);
        DrawTextA(hdc, label, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }
    free(pts); free(arr);
}