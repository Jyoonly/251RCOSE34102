#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_PROCESSES 100

// Process 구조체 정의
typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int io_request_time;
    int io_burst_time;
    int priority;
    int start_time;
    int end_time;
    int waiting_time;
    int turnaround_time;
} Process;

// 함수 선언
void create_processes(Process processes[], int num_processes);
void fcfs(Process processes[], int num_processes);
void calculate_metrics(Process processes[], int num_processes);
void print_gantt_chart(Process processes[], int num_processes);

// main 함수
int main() {
    Process processes[MAX_PROCESSES];
    int num_processes;

    printf("프로세스 개수를 입력하세요: ");
    scanf("%d", &num_processes);

    create_processes(processes, num_processes);
    fcfs(processes, num_processes);
    calculate_metrics(processes, num_processes);
    print_gantt_chart(processes, num_processes);
    
    return 0;
}
