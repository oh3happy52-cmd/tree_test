/*
 * 과제-11: 삽입 정렬과 쉘 정렬의 성능 비교
 *  - 삽입 정렬
 *  - 쉘 정렬 (기본 간격: n/2, n/4, ..., 1)
 *  - 쉘 정렬 (Knuth 간격: 1, 4, 13, 40, ... h(k+1) = 3h(k) + 1)
 *  - 비교 횟수: 데이터 값끼리 비교할 때마다 1회 (반복문 조건 검사는 제외)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N   20000
#define MAXKEY  1000000

static long long g_cmp = 0;   /* 데이터 값 비교 횟수 */

/* gap 간격 삽입 정렬 (gap == 1 이면 일반 삽입 정렬) */
static void gap_insertion(int a[], int n, int gap)
{
    for (int i = gap; i < n; i++) {
        int key = a[i];
        int j = i - gap;
        while (j >= 0) {
            g_cmp++;                      /* a[j] 와 key 비교 */
            if (a[j] > key) {
                a[j + gap] = a[j];        /* gap 만큼 뒤로 이동 */
                j -= gap;
            } else {
                break;
            }
        }
        a[j + gap] = key;
    }
}

/* [1] 삽입 정렬 */
static void insertion_sort(int a[], int n)
{
    gap_insertion(a, n, 1);
}

/* [2] 쉘 정렬 - 기본 간격: n/2, n/4, ..., 1 */
static void shell_sort_original(int a[], int n)
{
    for (int gap = n / 2; gap >= 1; gap /= 2)
        gap_insertion(a, n, gap);
}

/* [3] 쉘 정렬 - Knuth 간격: n보다 작은 가장 큰 간격부터 1까지 */
static void shell_sort_knuth(int a[], int n)
{
    int h = 1;
    while (3 * h + 1 < n)                 /* 1, 4, 13, 40, ... 중 n보다 작은 가장 큰 값 */
        h = 3 * h + 1;
    for (; h >= 1; h /= 3)                /* h(k) = (h(k+1) - 1) / 3 이므로 정수 나눗셈 3으로 역순 */
        gap_insertion(a, n, h);
}

static int is_sorted(const int a[], int n)
{
    for (int i = 0; i < n - 1; i++)
        if (a[i] > a[i + 1])
            return 0;
    return 1;
}

int main(void)
{
    static int orig[MAX_N], work[MAX_N];
    const int sizes[] = {1000, 2000, 4000, 8000, 16000, 20000};
    const int count = sizeof(sizes) / sizeof(sizes[0]);
    int all_ok = 1;

    srand(20261031);   /* 같은 결과를 재현할 수 있도록 시드 고정 */

    for (int s = 0; s < count; s++) {
        int n = sizes[s];
        long long c_ins, c_org, c_knu;
        int ok1, ok2, ok3;

        /* 원본 데이터 생성 (측정 제외) */
        for (int i = 0; i < n; i++)
            orig[i] = rand() % (MAXKEY + 1);

        /* 삽입 정렬 */
        memcpy(work, orig, sizeof(int) * n);
        g_cmp = 0;
        insertion_sort(work, n);
        c_ins = g_cmp;
        ok1 = is_sorted(work, n);

        /* 쉘 정렬 - 기본 간격 */
        memcpy(work, orig, sizeof(int) * n);
        g_cmp = 0;
        shell_sort_original(work, n);
        c_org = g_cmp;
        ok2 = is_sorted(work, n);

        /* 쉘 정렬 - Knuth 간격 */
        memcpy(work, orig, sizeof(int) * n);
        g_cmp = 0;
        shell_sort_knuth(work, n);
        c_knu = g_cmp;
        ok3 = is_sorted(work, n);

        printf("Data Size : %d\n\n", n);
        printf("Insertion Sort       : %lld comparisons\n", c_ins);
        printf("Shell Sort (Original): %lld comparisons\n", c_org);
        printf("Shell Sort (Knuth)   : %lld comparisons\n\n", c_knu);

        if (!(ok1 && ok2 && ok3)) {
            all_ok = 0;
            printf("!! 정렬 결과 오류 (n=%d): insertion=%d original=%d knuth=%d\n\n",
                   n, ok1, ok2, ok3);
        }
    }

    printf("All results sorted correctly : %s\n", all_ok ? "Yes" : "NO!");
    return 0;
}