#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_COMMAND 200
#define MAX_PATH 100
#define MAX_NODES 1000

typedef struct Node {
    char data;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    Node *root;
    int size;
    int count;
} BTree;

/* 함수 선언 */
BTree *create_btree(int size);
void destroy_nodes(Node *node);
void destroy_btree(BTree *tree);

Node *create_node(char value);
Node *find_node(Node *root, const char *path);

int valid_path(const char *path);
int valid_data(const char *data);
int is_leaf(Node *node);

void insert_root(BTree *tree, char value);
void insert_child(BTree *tree, const char *parent_path,
                  char child, char value);
void delete_node(BTree *tree, const char *leaf_path);
void update_value(BTree *tree, const char *node_path, char value);
void read_child(BTree *tree, const char *parent_path);

void print_nodes(Node *node, int depth);
void print_btree(BTree *tree);

Node *find_parent(Node *root, Node *target);
void print_error(const char *message);


/* 오류 출력 */
void print_error(const char *message)
{
    printf("에러: %s\n", message);
}


/* 노드 생성 */
Node *create_node(char value)
{
    Node *new_node;

    new_node = (Node *)malloc(sizeof(Node));

    if (new_node == NULL) {
        return NULL;
    }

    new_node->data = value;
    new_node->left = NULL;
    new_node->right = NULL;

    return new_node;
}


/* 빈 이진트리 생성 */
BTree *create_btree(int size)
{
    BTree *tree;

    tree = (BTree *)malloc(sizeof(BTree));

    if (tree == NULL) {
        return NULL;
    }

    tree->root = NULL;
    tree->size = size;
    tree->count = 0;

    return tree;
}


/* 노드 전체 삭제 */
void destroy_nodes(Node *node)
{
    if (node == NULL) {
        return;
    }

    destroy_nodes(node->left);
    destroy_nodes(node->right);

    free(node);
}


/* 트리 삭제 */
void destroy_btree(BTree *tree)
{
    if (tree == NULL) {
        return;
    }

    destroy_nodes(tree->root);
    free(tree);
}


/* 단말 노드 확인 */
int is_leaf(Node *node)
{
    if (node == NULL) {
        return 0;
    }

    if (node->left == NULL && node->right == NULL) {
        return 1;
    }

    return 0;
}


/* 데이터가 영문 대문자인지 확인 */
int valid_data(const char *data)
{
    if (data == NULL || strlen(data) != 1) {
        return 0;
    }

    if (data[0] < 'A' || data[0] > 'Z') {
        return 0;
    }

    return 1;
}


/* 경로 형식 확인 */
int valid_path(const char *path)
{
    int i;

    if (path == NULL) {
        return 0;
    }

    if (path[0] != '/') {
        return 0;
    }

    if (path[1] < 'A' || path[1] > 'Z') {
        return 0;
    }

    i = 2;

    while (path[i] != '\0') {
        if (path[i] != '/') {
            return 0;
        }

        if (path[i + 1] < 'A' || path[i + 1] > 'Z') {
            return 0;
        }

        i += 2;
    }

    return 1;
}


/*
 * 경로를 이용하여 노드 검색
 *
 * 예: /A/B/C
 * 루트 A부터 시작하여 B, C 순서로 이동한다.
 */
Node *find_node(Node *root, const char *path)
{
    Node *current;
    int i;
    char value;

    if (root == NULL || !valid_path(path)) {
        return NULL;
    }

    current = root;

    if (current->data != path[1]) {
        return NULL;
    }

    i = 2;

    while (path[i] != '\0') {
        value = path[i + 1];

        if (current->left != NULL &&
            current->left->data == value) {
            current = current->left;
        }
        else if (current->right != NULL &&
                 current->right->data == value) {
            current = current->right;
        }
        else {
            return NULL;
        }

        i += 2;
    }

    return current;
}


/* 부모 노드 검색 */
Node *find_parent(Node *root, Node *target)
{
    Node *parent;

    if (root == NULL || target == NULL) {
        return NULL;
    }

    if (root->left == target || root->right == target) {
        return root;
    }

    parent = find_parent(root->left, target);

    if (parent != NULL) {
        return parent;
    }

    return find_parent(root->right, target);
}


/* 루트 노드 삽입 */
void insert_root(BTree *tree, char value)
{
    Node *new_node;

    if (tree == NULL) {
        print_error("트리가 존재하지 않습니다.");
        return;
    }

    if (tree->root != NULL) {
        print_error("루트 노드가 이미 존재합니다.");
        return;
    }

    if (tree->count >= tree->size) {
        print_error("트리의 최대 노드 수를 초과했습니다.");
        return;
    }

    new_node = create_node(value);

    if (new_node == NULL) {
        print_error("메모리 할당에 실패했습니다.");
        return;
    }

    tree->root = new_node;
    tree->count++;

    printf("Insert root: %c\n", value);
}


/* 자식 노드 삽입 */
void insert_child(BTree *tree, const char *parent_path,
                  char child, char value)
{
    Node *parent;
    Node *new_node;

    if (tree == NULL || tree->root == NULL) {
        print_error("트리가 비어 있습니다.");
        return;
    }

    parent = find_node(tree->root, parent_path);

    if (parent == NULL) {
        print_error("부모 노드를 찾을 수 없습니다.");
        return;
    }

    if (child != 'L' && child != 'R') {
        print_error("자식 위치는 L 또는 R이어야 합니다.");
        return;
    }

    /* 지정한 슬롯(L 또는 R)이 이미 차 있으면 삽입할 수 없다.
       (부모 전체가 단말일 필요는 없고, 넣으려는 자리만 비어 있으면 된다) */
    if (child == 'L' && parent->left != NULL) {
        print_error("왼쪽 자식이 이미 존재합니다.");
        return;
    }
    if (child == 'R' && parent->right != NULL) {
        print_error("오른쪽 자식이 이미 존재합니다.");
        return;
    }

    if (tree->count >= tree->size) {
        print_error("트리의 최대 노드 수를 초과했습니다.");
        return;
    }

    if (parent->left != NULL &&
        parent->left->data == value) {
        print_error("왼쪽 자식과 같은 데이터입니다.");
        return;
    }

    if (parent->right != NULL &&
        parent->right->data == value) {
        print_error("오른쪽 자식과 같은 데이터입니다.");
        return;
    }

    new_node = create_node(value);

    if (new_node == NULL) {
        print_error("메모리 할당에 실패했습니다.");
        return;
    }

    if (child == 'L') {
        parent->left = new_node;
    }
    else {
        parent->right = new_node;
    }

    tree->count++;

    printf("Insert: %c(%c)\n", value, child);
}


/* 단말 노드 삭제 */
void delete_node(BTree *tree, const char *leaf_path)
{
    Node *target;
    Node *parent;

    if (tree == NULL || tree->root == NULL) {
        print_error("트리가 비어 있습니다.");
        return;
    }

    target = find_node(tree->root, leaf_path);

    if (target == NULL) {
        print_error("삭제할 노드를 찾을 수 없습니다.");
        return;
    }

    if (!is_leaf(target)) {
        print_error("단말 노드만 삭제할 수 있습니다.");
        return;
    }

    /* 루트 노드를 삭제하는 경우 */
    if (target == tree->root) {
        free(target);
        tree->root = NULL;
        tree->count--;

        printf("Root deleted.\n");
        return;
    }

    parent = find_parent(tree->root, target);

    if (parent == NULL) {
        print_error("부모 노드를 찾을 수 없습니다.");
        return;
    }

    if (parent->left == target) {
        parent->left = NULL;
    }
    else if (parent->right == target) {
        parent->right = NULL;
    }

    free(target);
    tree->count--;

    printf("삭제 완료.\n");
}


/* 노드 데이터 수정 */
void update_value(BTree *tree, const char *node_path, char value)
{
    Node *target;
    Node *parent;

    if (tree == NULL || tree->root == NULL) {
        print_error("트리가 비어 있습니다.");
        return;
    }

    target = find_node(tree->root, node_path);

    if (target == NULL) {
        print_error("수정할 노드를 찾을 수 없습니다.");
        return;
    }

    /* 루트 노드는 부모가 없으므로 바로 수정 */
    if (target == tree->root) {
        target->data = value;
        printf("수정 완료.\n");
        return;
    }

    parent = find_parent(tree->root, target);

    if (parent == NULL) {
        print_error("부모 노드를 찾을 수 없습니다.");
        return;
    }

    /*
     * 같은 부모의 다른 자식과 데이터가 같아지는지 확인
     */
    if (parent->left == target &&
        parent->right != NULL &&
        parent->right->data == value) {
        print_error("다른 자식과 같은 데이터가 됩니다.");
        return;
    }

    if (parent->right == target &&
        parent->left != NULL &&
        parent->left->data == value) {
        print_error("다른 자식과 같은 데이터가 됩니다.");
        return;
    }

    target->data = value;

    printf("수정 완료.\n");
}


/* 자식 정보 출력 */
void read_child(BTree *tree, const char *parent_path)
{
    Node *parent;
    int printed;

    if (tree == NULL || tree->root == NULL) {
        print_error("트리가 비어 있습니다.");
        return;
    }

    parent = find_node(tree->root, parent_path);

    if (parent == NULL) {
        print_error("노드를 찾을 수 없습니다.");
        return;
    }

    printed = 0;

    if (parent->left != NULL) {
        printf("%c(L)", parent->left->data);
        printed = 1;
    }

    if (parent->right != NULL) {
        if (printed) {
            printf(", ");
        }

        printf("%c(R)", parent->right->data);
        printed = 1;
    }

    if (!printed) {
        printf("자식 노드가 없습니다.");
    }

    printf("\n");
}

/*
 * 트리를 왼쪽으로 눕힌 형태로 출력
 *
 * 오른쪽 자식을 먼저 출력하여
 * 트리가 왼쪽으로 누운 모양이 되도록 한다.
 */

void print_nodes(Node *node, int depth)
{
    int i;

    if (node == NULL) {
        return;
    }

    if (depth == 0) {
        printf("%c\n", node->data);
    }
    else {
        for (i = 0; i < depth - 1; i++) {
            printf("    ");
        }
        printf("+---%c\n", node->data);
    }

    print_nodes(node->left, depth + 1);
    print_nodes(node->right, depth + 1);
}


/* 전체 트리 출력 */
void print_btree(BTree *tree)
{
    if (tree == NULL || tree->root == NULL) {
        printf("트리가 비어 있습니다.\n");
        return;
    }

    print_nodes(tree->root, 0);
}


/* 메인 함수 */
int main(void)
{
    BTree *tree;
    char command[MAX_COMMAND];
    char *token;
    char *args[5];
    int count;
    char value;
    char child;

    tree = create_btree(MAX_NODES);

    if (tree == NULL) {
        printf("트리 생성에 실패했습니다.\n");
        return 1;
    }

    printf("이진트리 조작 프로그램\n");
    printf("명령어를 입력하세요.\n");

    while (1) {
        printf("> ");

        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = '\0';

        count = 0;
        token = strtok(command, " \t");

        while (token != NULL && count < 5) {
            args[count] = token;
            count++;
            token = strtok(NULL, " \t");
        }

        if (count == 0) {
            continue;
        }

        /* 종료 명령 */
        if (strcmp(args[0], "Exit") == 0 ||
            strcmp(args[0], "E") == 0) {
            break;
        }

        /* Insert */
        else if (strcmp(args[0], "Insert") == 0 ||
                 strcmp(args[0], "I") == 0) {

            if (count == 3) {
                /*
                 * Insert / A
                 * 루트 노드 생성
                 */
                if (strcmp(args[1], "/") != 0 ||
                    !valid_data(args[2])) {
                    print_error("루트 삽입 형식이 잘못되었습니다.");
                    continue;
                }

                value = args[2][0];
                insert_root(tree, value);
            }
            else if (count == 4) {
                /*
                 * Insert /A/B L C
                 * child 인자는 "L", "R" 뿐 아니라
                 * "Left", "Right" 전체 단어도 첫 글자로 인식한다.
                 */
                if (!valid_path(args[1]) ||
                    strlen(args[2]) < 1 ||
                    !valid_data(args[3])) {
                    print_error("Insert 명령 형식이 잘못되었습니다.");
                    continue;
                }

                child = toupper((unsigned char)args[2][0]);

                if (child != 'L' && child != 'R') {
                    print_error("자식 위치는 L 또는 R이어야 합니다.");
                    continue;
                }

                value = args[3][0];

                insert_child(tree, args[1], child, value);
            }
            else {
                print_error("Insert 명령의 인자 개수가 잘못되었습니다.");
            }
        }

        /* Delete */
        else if (strcmp(args[0], "Delete") == 0 ||
                 strcmp(args[0], "D") == 0) {

            if (count != 2 || !valid_path(args[1])) {
                print_error("Delete 명령 형식이 잘못되었습니다.");
                continue;
            }

            delete_node(tree, args[1]);
        }

        /* Update */
        else if (strcmp(args[0], "Update") == 0 ||
                 strcmp(args[0], "U") == 0) {

            if (count != 3 ||
                !valid_path(args[1]) ||
                !valid_data(args[2])) {
                print_error("Update 명령 형식이 잘못되었습니다.");
                continue;
            }

            value = args[2][0];

            update_value(tree, args[1], value);
        }

        /* Read */
        else if (strcmp(args[0], "Read") == 0 ||
                 strcmp(args[0], "R") == 0) {

            if (count != 2 || !valid_path(args[1])) {
                print_error("Read 명령 형식이 잘못되었습니다.");
                continue;
            }

            read_child(tree, args[1]);
        }

        /* Print */
        else if (strcmp(args[0], "Print") == 0 ||
                 strcmp(args[0], "P") == 0) {

            if (count != 1) {
                print_error("Print 명령은 인자를 받지 않습니다.");
                continue;
            }

            print_btree(tree);
        }

        /* 잘못된 명령 */
        else {
            print_error("잘못된 명령어입니다.");
        }
    }

    destroy_btree(tree);

    printf("프로그램을 종료합니다.\n");

    return 0;
}