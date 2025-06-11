#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PROCESSES 100
#define MAX_TIME 500
#define RR_QUANTUM 2
#define INF 1e9
// -----------------------------
// 구조체 정의
// -----------------------------
typedef struct { 
    int pid;

    int arrival_time;
    int cpu_burst_total;
    int cpu_remaining;
    int priority;

    int start_time;
    int finish_time;
    int waiting_time;
    int turnaround_time;
} PCB;


// -----------------------------
// 공용 출력 함수
// -----------------------------
// 생성된 프로세스 출력
void print_created_list(PCB proc[], int n)
{
    puts("\n[Created Processes]");
    puts("PID | Arrival | Burst | Priority");

    for (int i = 0; i < n; i++)
        printf("%3d | %7d | %5d | %8d\n",
               proc[i].pid, proc[i].arrival_time,
               proc[i].cpu_burst_total, proc[i].priority);
}
// 간트차트 출력
void print_gantt(const int timeline[], int tl_len, const char *tag)
{   
    // // === 디버깅용 ===//
    // printf("\n");
    // for (int i = 0; i<tl_len; i++) {
    //     printf("%d ", timeline[i]);
    // }
    // // ===============//
    printf("\n[Gantt : %s]\n|", tag);

    for (int i = 0; i < tl_len; ) {
        int pid  = timeline[i];
        int span = 1;

        while (i + span < tl_len && timeline[i + span] == pid){
            span++;
        }

        if (pid){ // process
            printf(" P%-2d:%-2d |", pid, span);
        }
        else { // idle
            printf(" ID:%-2d |", span);
        }
        i += span;
    }
    printf("|");
}
// 디버그용 프로세스 상태 테이블
void print_table(PCB proc[], int n)
{
    puts("\n[Process Table]");
    puts("PID | Arr | Burst | Start | End | Wait | TAT | Pri");

    for (int i = 0; i < n; i++)
        printf("%3d | %3d | %5d | %5d | %3d | %4d | %3d | %3d\n",
               proc[i].pid,
               proc[i].arrival_time,
               proc[i].cpu_burst_total,
               proc[i].start_time,
               proc[i].finish_time,
               proc[i].waiting_time,
               proc[i].turnaround_time,
               proc[i].priority);
}

// 평가용 함수
void evaluate(PCB proc[], int n)
{
    double wait_sum = 0.0;
    double tat_sum  = 0.0;

    for (int i = 0; i < n; i++) {
        wait_sum += proc[i].waiting_time;
        tat_sum  += proc[i].turnaround_time;
    }
    printf("AVG Waiting : %.2f   AVG TAT : %.2f\n\n",
           wait_sum / n, tat_sum / n);
}

// -----------------------------
// 프로세스 생성
// -----------------------------
void create_processes(PCB proc[], int num_processes) {
    srand(time(NULL)); // 난수 초기화

    for (int i = 0; i < num_processes; i++) {
        proc[i].pid = i + 1;
        proc[i].arrival_time = rand() % 10; // 0~19 사이
        proc[i].cpu_burst_total = rand() % 10 + 1; // 1~10 사이
        proc[i].cpu_remaining = proc[i].cpu_burst_total;
        proc[i].priority = rand() % num_processes + 1; // 1부터 num_processes 사이의 우선순위

        // 나머지는 초기화
        proc[i].start_time = -1;
        proc[i].finish_time = -1;
        proc[i].waiting_time = 0;
        proc[i].turnaround_time = 0;
    }
}

// -----------------------------
// Schedule 함수들
// -----------------------------

// -----------------------------
// 1. FCFS 
// -----------------------------
void schedule_fcfs(PCB proc[], int n) {
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    // 도착시간 오름차순 정렬(버블) 
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++)
            if (proc[i].arrival_time > proc[j].arrival_time) {
                PCB tmp = proc[i];
                proc[i] = proc[j];
                proc[j] = tmp;
            }

    int current_time = 0;

    for (int i = 0; i < n; i++) {
        // CPU가 놀면 idle 기록 
        while (current_time < proc[i].arrival_time) {
            timeline[tl_len++] = 0;
            current_time++;
        }

        proc[i].start_time = current_time;

        for (int t = 0; t < proc[i].cpu_burst_total; t++) {
            timeline[tl_len++] = proc[i].pid;
            current_time++;
        }

        proc[i].finish_time = current_time;
        proc[i].waiting_time = proc[i].start_time - proc[i].arrival_time;
        proc[i].turnaround_time = proc[i].finish_time - proc[i].arrival_time;
    }

    print_gantt(timeline, tl_len, "FCFS");
    print_table(proc, n);
    evaluate(proc, n);
}

// -----------------------------
// 2. SJF (non-preemptive)
// -----------------------------
void schedule_sjf_np(PCB proc[], int n)
{
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    bool finished[MAX_PROCESSES] = {false};

    int completed    = 0;
    int current_time = 0;

    while (completed < n) {
        int sel_index = -1;
        int best_burst= INF;

        for (int i = 0; i < n; i++) {
            if (!finished[i] &&
                proc[i].arrival_time <= current_time &&
                proc[i].cpu_burst_total < best_burst)
            {
                best_burst = proc[i].cpu_burst_total;
                sel_index  = i;
            }
        }

        if (sel_index == -1) {
            timeline[tl_len++] = 0;
            current_time++;
            continue;
        }

        if (proc[sel_index].start_time == -1)
            proc[sel_index].start_time = current_time;

        for (int t = 0; t < proc[sel_index].cpu_burst_total; t++) {
            timeline[tl_len++] = proc[sel_index].pid;
            current_time++;
        }

        proc[sel_index].finish_time = current_time;
        proc[sel_index].waiting_time =
            proc[sel_index].start_time - proc[sel_index].arrival_time;
        proc[sel_index].turnaround_time =
            proc[sel_index].finish_time - proc[sel_index].arrival_time;

        finished[sel_index] = true;
        completed++;
    }

    print_gantt(timeline, tl_len, "SJF (NP)");
    print_table(proc, n);
    evaluate(proc, n);
}
// -----------------------------
// 3. Priority (non-preemptive)
// -----------------------------
void schedule_priority_np(PCB proc[], int n)
{
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    bool finished[MAX_PROCESSES] = {false};

    int completed    = 0;
    int current_time = 0;

    while (completed < n) {
        int sel_index = -1;
        int best_pri  = INF;

        for (int i = 0; i < n; i++) {
            if (!finished[i] &&
                proc[i].arrival_time <= current_time &&
                proc[i].priority < best_pri)
            {
                best_pri  = proc[i].priority;
                sel_index = i;
            }
        }

        if (sel_index == -1) {
            timeline[tl_len++] = 0;
            current_time++;
            continue;
        }

        if (proc[sel_index].start_time == -1)
            proc[sel_index].start_time = current_time;

        for (int t = 0; t < proc[sel_index].cpu_burst_total; t++) {
            timeline[tl_len++] = proc[sel_index].pid;
            current_time++;
        }

        proc[sel_index].finish_time     = current_time;
        proc[sel_index].waiting_time    =
            proc[sel_index].start_time - proc[sel_index].arrival_time;
        proc[sel_index].turnaround_time =
            proc[sel_index].finish_time - proc[sel_index].arrival_time;

        finished[sel_index] = true;
        completed++;
    }

    print_gantt(timeline, tl_len, "Priority (NP)");
    print_table(proc, n);
    evaluate(proc, n);
}

// -----------------------------
// 4. RR (Q=2)
// -----------------------------
void schedule_rr(PCB proc[], int n, int quantum)
{
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    int ready_q[MAX_PROCESSES];
    int q_front = 0, q_rear = 0;

    bool visited[MAX_PROCESSES] = {false};

    int completed    = 0;
    int current_time = 0;

    // 0 tick 에 도착한 프로세스 enqueue 
    for (int i = 0; i < n; i++)
        if (proc[i].arrival_time == 0) {
            ready_q[q_rear++] = i;
            visited[i] = true;
        }

    while (completed < n) {
        if (q_front == q_rear) { // Ready 큐가 비어있으면 idle 기록
            timeline[tl_len++] = 0;
            current_time++;
            
            for (int i = 0; i < n; i++)
                if (!visited[i] && proc[i].arrival_time == current_time) {
                    ready_q[q_rear++] = i;
                    visited[i]        = true;
                }
            continue;
        }

        int sel_index = ready_q[q_front++];

        if (proc[sel_index].start_time == -1)
            proc[sel_index].start_time = current_time;

        int exec = (proc[sel_index].cpu_remaining > quantum)
                   ? quantum
                   : proc[sel_index].cpu_remaining;

        for (int t = 0; t < exec; t++) {
            timeline[tl_len++] = proc[sel_index].pid;
            proc[sel_index].cpu_remaining--;
            current_time++;

            // tick 중간 도착 즉시 Ready 큐 삽입 
            for (int i = 0; i < n; i++)
                if (!visited[i] && proc[i].arrival_time == current_time) {
                    ready_q[q_rear++] = i;
                    visited[i] = true;
                }
        }

        if (proc[sel_index].cpu_remaining > 0)
            ready_q[q_rear++] = sel_index; 
        else {
            proc[sel_index].finish_time = current_time;
            proc[sel_index].turnaround_time =
                proc[sel_index].finish_time - proc[sel_index].arrival_time;
            proc[sel_index].waiting_time =
                proc[sel_index].turnaround_time - proc[sel_index].cpu_burst_total;
            completed++;
        }
    }

    print_gantt(timeline, tl_len, "Round-Robin(Q=2)");
    print_table(proc, n);
    evaluate(proc, n);
}

// -----------------------------
// 5. SJF (preemptive)
// -----------------------------
void schedule_sjf_p(PCB proc[], int n)
{
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    int completed    = 0;
    int current_time = 0;

    while (completed < n) {
        int sel_index = -1;
        int best_rem  = INF;

        for (int i = 0; i < n; i++)
            if (proc[i].arrival_time <= current_time &&
                proc[i].cpu_remaining > 0 &&
                proc[i].cpu_remaining < best_rem)
            {
                best_rem  = proc[i].cpu_remaining;
                sel_index = i;
            }

        if (sel_index == -1) {
            timeline[tl_len++] = 0;
            current_time++;
            continue;
        }

        if (proc[sel_index].start_time == -1)
            proc[sel_index].start_time = current_time;

        timeline[tl_len++] = proc[sel_index].pid;  // 1 tick 실행
        proc[sel_index].cpu_remaining--;
        current_time++;

        if (proc[sel_index].cpu_remaining == 0) {
            proc[sel_index].finish_time = current_time;
            proc[sel_index].turnaround_time =
                proc[sel_index].finish_time - proc[sel_index].arrival_time;
            proc[sel_index].waiting_time =
                proc[sel_index].turnaround_time - proc[sel_index].cpu_burst_total;
            completed++;
        }
    }

    print_gantt(timeline, tl_len, "SJF (Preemptive)");
    print_table(proc, n);
    evaluate(proc, n);
}

// -----------------------------
// 6. Priority (preemptive)
// -----------------------------
void schedule_priority_p(PCB proc[], int n)
{
    int timeline[MAX_TIME] = {0};
    int tl_len = 0;

    int completed    = 0;
    int current_time = 0;

    while (completed < n) {
        int sel_index = -1;
        int best_pri  = INF;

        for (int i = 0; i < n; i++)
            if (proc[i].arrival_time <= current_time &&
                proc[i].cpu_remaining > 0 &&
                proc[i].priority < best_pri)
            {
                best_pri  = proc[i].priority;
                sel_index = i;
            }

        if (sel_index == -1) {
            timeline[tl_len++] = 0;
            current_time++;
            continue;
        }

        if (proc[sel_index].start_time == -1)
            proc[sel_index].start_time = current_time;

        timeline[tl_len++] = proc[sel_index].pid;  // 1 tick 실행
        proc[sel_index].cpu_remaining--;
        current_time++;

        if (proc[sel_index].cpu_remaining == 0) {
            proc[sel_index].finish_time = current_time;
            proc[sel_index].turnaround_time =
                proc[sel_index].finish_time - proc[sel_index].arrival_time;
            proc[sel_index].waiting_time =
                proc[sel_index].turnaround_time - proc[sel_index].cpu_burst_total;
            completed++;
        }
    }

    print_gantt(timeline, tl_len, "Priority (Preemptive)");
    print_table(proc, n);
    evaluate(proc, n);
}

// -----------------------------
// 메인 함수
// -----------------------------
int main() {
    PCB original[MAX_PROCESSES];
    PCB work[MAX_PROCESSES];

    int num_processes = 0;

    printf("Number of processes?: ");
    scanf("%d", &num_processes);

    // Process 생성
    create_processes(original, num_processes);
    print_created_list(original, num_processes);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_fcfs(work, num_processes);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_sjf_np(work, num_processes);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_priority_np(work, num_processes);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_rr(work, num_processes, RR_QUANTUM);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_sjf_p(work, num_processes);

    memcpy(work, original, sizeof(PCB) * num_processes);
    schedule_priority_p(work, num_processes);

    return 0;
}
