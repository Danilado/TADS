#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "my_tree.h"
#include <sys/time.h>
#include <time.h>

#define time_now(x) clock_gettime(CLOCK_REALTIME, &x)

long delta_time(struct timespec mt1, struct timespec mt2)
{
    return 1000000000 * (mt2.tv_sec - mt1.tv_sec) + (mt2.tv_nsec - mt1.tv_nsec);
}

int intcmp(const void *l, const void *r)
{
    return *(int *)l - *(int *)r;
}

enum menu
{
    EXIT,
    SHOW_TREE,
    CHOOSE_FILE,
    PARSE_FILE,
    ADD_NUMBER,
    COUNT_NODES_ON_LEVELS,
    AVG_ADD_TIME,
    DELETE_NUMBER,
};

int main(void)
{
    tree_t tree = NULL;
    char *filename = NULL;

    int running = 1;

    while (running)
    {
        {
            printf("Текущий файл: %s\n", (filename ? filename : "Не выбран"));
            printf("Меню:\n");
            printf("0: Выход\n");
            printf("1: Вывести дерево на экран\n");
            printf("2: Выбрать файл для работы\n");
            printf("3: Считать дерево из файла\n");
            printf("4: Добавить число в дерево\n");
            printf("5: Посчитать количество узлов дерева на каждом уровне\n");
            printf("6: Вывести среднее время добавления числа в дерево\n");
            printf("7: Удалить число из дерева\n");
            printf(">");
        }

        int tmp;
        if (scanf("%d", &tmp) != 1)
            continue;

        switch (tmp)
        {
        case EXIT: {
            running = 0;
        }
        break;
        case SHOW_TREE: {
            if (!tree)
            {
                printf("Сначала инициализируйте дерево!\n");
                break;
            }
            if (open_tree_img("tree", tree))
            {
                printf(
                "Не получилось создать файл с деревом - проверьте права\n");
            }
        }
        break;
        case CHOOSE_FILE: {
            getc(stdin);

            if (filename)
            {
                free(filename);
                filename = NULL;
            }

            if (tree)
                tree_destroy(&tree);

            printf("Введите название файла\n>");
            size_t bufsize = 1;
            char *buf = calloc(bufsize, sizeof(char));
            getline(&buf, &bufsize, stdin);

            buf[strlen(buf) - 1] = '\0';

            FILE *tmp = fopen(buf, "r");
            if (!tmp)
            {
                printf("Файл не открывается или не существует");
                continue;
            }
            fclose(tmp);
            tmp = fopen(buf, "a");
            if (!tmp)
            {
                printf("Нет прав на запись в файл!");
                continue;
            }
            fclose(tmp);

            filename = buf;
        }
        break;
        case PARSE_FILE: {
            FILE *tmp = fopen(filename, "r");
            if (!tmp)
            {
                printf("Файл не открывается или не существует");
                continue;
            }

            if (tree)
                tree_destroy(&tree);

            tree = tree_create_from_file(tmp, intcmp, "d");
            if (!tree)
                printf("Возникли ошибки при создании дерева (скорее всего не "
                       "хватило памяти)");

            fclose(tmp);
        }
        break;
        case ADD_NUMBER: {
            if (!filename)
            {
                printf("Вы не выбрали файл для работы!\n");
                continue;
            }

            if (!tree)
            {
                tree = tree_create(intcmp, "d");
                if (!tree)
                {
                    printf("Не удалось инициализировать дерево\n");
                    continue;
                }
            }

            int tmp;
            printf("Введите число: ");
            while (scanf("%d", &tmp) != 1)
            {
                printf("Ошибка чтения\n");
                printf("Введите число: ");
            }

            struct timespec begin, end;
            long time1, time2;

            FILE *f = fopen(filename, "a");
            if (!f)
            {
                printf("Не удалось открыть файл для записи\n");
                continue;
            }

            int *buf = calloc(1, sizeof(int));
            if (!buf)
            {
                fclose(f);
                printf("Не хватает места для числа\n");
                continue;
            }

            *buf = tmp;

            time_now(begin);
            fprintf(f, " %d\n", tmp);
            time_now(end);
            time1 = delta_time(begin, end);

            time_now(begin);
            int rc = tree_push(tree, buf);
            time_now(end);
            time2 = delta_time(begin, end);

            if (rc)
            {
                free(buf);
                printf("Не удалось записать число в дерево");
                continue;
            }

            printf("Время добавления числа в файл:   %ldнс\n", time1);
            printf("Время добавления числа в дерево: %ldнс\n", time2);

            fclose(f);
        }
        break;
        case COUNT_NODES_ON_LEVELS: {
            tree_print_node_level_count(tree);
        }
        break;
        case AVG_ADD_TIME: {
            if (!tree)
            {
                printf("Сначала инициализируйте дерево!\n");
                continue;
            }

            struct timespec begin, end;
            double timesum = 0;

            int tmin = int_tree_min(tree);
            int tmax = int_tree_max(tree);

            for (int i = tmin; i <= tmax; ++i)
            {
                int *buf = calloc(1, sizeof(int));
                if (!buf)
                {
                    printf("Не хватает места для числа\n");
                    continue;
                }

                *buf = i;

                time_now(begin);
                tree_push(tree, buf);
                time_now(end);

                timesum += delta_time(begin, end);
            }

            printf("Среднее время добавления числа в дерево: %.2lfнс\n",
            timesum / (double)(tmax - tmin + 1));
            continue;
        }
        break;
        case DELETE_NUMBER: {
            if (!filename)
            {
                printf("Вы не выбрали файл для работы!\n");
                continue;
            }

            if (!tree)
            {
                printf("Сначала инициализируйте дерево\n");
                continue;
            }

            int tmp;
            printf("Введите число для удаления: ");
            while (scanf("%d", &tmp) != 1)
            {
                printf("Ошибка чтения\n");
                printf("Введите число: ");
            }

            int *buf = calloc(1, sizeof(int));
            *buf = tmp;

            tree_node_delete(tree, buf);
            continue;
        }
        break;

        default:
            break;
        }
    }

    if (filename)
        free(filename);

    if (tree)
        tree_destroy(&tree);

    return EXIT_SUCCESS;
}
